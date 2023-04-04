#include <iostream>
#include <olc_net.h>
#include <ncurses.h>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>

enum class CustomMsgTypes : uint32_t
{
    ServerAccept,
    ServerDeny,
    ServerPing,
    MessageAll,
    ServerMessage,
    Login,
    InitiateConversation,
    Register,
    SendMessageToUser,
    ReceiveMessageFromUser,
};

std::string input{};
std::string currentLoggedInUser;
std::string currentConverser;
bool commandMode = true;
bool conversationMode = false;
int conversationLineCount = 0;
int currentConversatorID = 0;




class CustomClient : public olc::net::client_interface<CustomMsgTypes>
{
    public:

        void SetupConversation(int clientID)
        {
            conversationMode = true;
            // send public key to other client

            // receive public key from other client

            // send a message to the other client

            //SendMessageToServer();

        }

        bool StartsWith(const std::string& str, const std::string& prefix)
        {
            return str.substr(0, prefix.size()) == prefix;
        }

        void CommitBackspace()
        {
            printw(" ");

            int y, x;
            getyx(stdscr, y, x);

            if (x > 0)
            move(y, x - 1);
        }
        void CaptureInput()
        {
            input = "";

                int y, x;
                getyx(stdscr, y, x);

                char currentChar;

                for (int i = 0; i < x; i++)
                {
                    move(y, i);
                    currentChar = inch();
                    input += currentChar;
                }

                move(LINES - 1, 0);
                clrtoeol();
        }

    private:
        void BlockUntilDoneReceivingInput()
        {
            int currentCharInt = getch();
            while (currentCharInt != KEY_ENTER)
            {
                currentCharInt = getch();

                switch (currentCharInt)
                {
                    case KEY_BACKSPACE:
                    {
                        CommitBackspace();
                        break;
                    }
                }
            }
            CaptureInput();
        }

        std::string sha256(const std::string str)
        {
            unsigned char hash[SHA256_DIGEST_LENGTH];

            SHA256_CTX sha256;
            SHA256_Init(&sha256);
            SHA256_Update(&sha256, str.c_str(), str.size());
            SHA256_Final(hash, &sha256);

            std::stringstream ss;

            for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
            {
                ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>( hash[i] );
            }
            return ss.str();
            }

    public:

        void SendMessageToUser(std::string currentToken, int clientID)
        {
            if (currentToken.size() < 1)
            {
                printw("You are not logged in!\n");
                conversationMode = false;
                return;
            }

            //move((LINES - 1) - conversationLineCount,0);

            move(LINES - 1, 0);

            std::string fullMsg = currentToken + "¬" + std::to_string(clientID) + "¬" + input;

            char messageToSend[5000];

            for (int i = 0; i < 5000; i++)
            {
                messageToSend[i] = NULL;
            }

            for (int i = 0; i < fullMsg.size(); i++)
            {
                messageToSend[i] = fullMsg[i];
            }

            messageToSend[fullMsg.size()] = '\0';

            olc::net::message<CustomMsgTypes> msg;
            msg.header.id = CustomMsgTypes::SendMessageToUser;

            msg << messageToSend;

            Send(msg);


        }
        
        void PingServer()
        {
            olc::net::message<CustomMsgTypes> msg;
            msg.header.id = CustomMsgTypes::ServerPing;

            // we are sending the time to the server, and the server will send it back so we can compare the time taken
            std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();

            msg << timeNow;

            Send(msg);
        } 

        void SendMessageToServer(std::string message, CustomMsgTypes type)
        {
            char messageToSend[200];

            for (int i = 0; i < 200; i++)
            {
                messageToSend[i] = NULL;
            }

            for (int i = 0; i < message.size(); i++)
            {
                messageToSend[i] = message[i];
            }

            olc::net::message<CustomMsgTypes> msg;
            msg.header.id = type;

            msg << messageToSend;

            Send(msg);
        }

        void Login()
        {
            commandMode = false;
            move(0,0);
            erase();
            printw("Enter Username:\n"); 
            move(LINES - 1, 0);
            BlockUntilDoneReceivingInput();
            std::string username = input;

            move(0,0);
            erase();
            printw("Enter Password:\n");
            move(LINES - 1, 0);
            BlockUntilDoneReceivingInput();
            std::string password = input;
            erase();
            move(LINES - 1, 0);
            
            password = sha256(password);

            std::string fullMessage;
            fullMessage = username + " " + password;

            currentLoggedInUser = username;
            
            SendMessageToServer(fullMessage, CustomMsgTypes::Login);
            commandMode = true;
        }

        void Register()
        {
            commandMode = false;
            move(0,0);
            erase();
            printw("Enter Username:\n"); 
            move(LINES - 1, 0);
            BlockUntilDoneReceivingInput();
            std::string username = input;

            move(0,0);
            erase();
            printw("Enter Password:\n");
            move(LINES - 1, 0);
            BlockUntilDoneReceivingInput();
            std::string password = input;
            erase();
            move(LINES - 1, 0);

            password = sha256(password);
            std::string fullMessage;
            fullMessage = username + " " + password;
            
            SendMessageToServer(fullMessage, CustomMsgTypes::Register);
            commandMode = true;
        }

        void InitiateConversation(std::string currentToken)
        {
            commandMode = false;
            if (currentToken.size() < 1)
            {
                printw("You are not logged in!\n");
                return;
            }

            move(0,0);
            erase();
            printw("Enter Username:\n");  
            move(LINES - 1, 0);
            BlockUntilDoneReceivingInput();
            std::string username = input;
            erase();
            move(LINES - 1, 0);

            std::string fullMsg;

            fullMsg = currentToken + "¬" + username;

            currentConverser = username;

            SendMessageToServer(fullMsg, CustomMsgTypes::InitiateConversation);
            commandMode = true;
        }

       
};







int main()
{
    

    CustomClient c;
    c.Connect("127.0.0.1", 60000);
    

    bool bQuit{false};

    std::string currentToken;

    WINDOW *w = initscr();
    cbreak();
    echo();
    nodelay(w, TRUE);
    keypad(stdscr, TRUE);
    move(LINES - 1, 0);

    start_color();

    // Define colors
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_WHITE, COLOR_BLACK);


    while (!bQuit)
    {
        int ch = getch();
        

        switch (ch)
        {
            
            case KEY_BACKSPACE:
            {
                c.CommitBackspace();

                break;
            }

            case 'q':
            {
                if (conversationMode)
                {
                    conversationMode = false;
                    commandMode = true;
                    echo();
                }
                else
                {
                    bQuit = true;
                }
            }

            case KEY_ENTER:
            {
                c.CaptureInput();

                if (!conversationMode)
                {
                    if (commandMode)
                    {
                        if (input == "/login")
                        {
                            c.Login();
                        }
                        else if (input == "/register")
                        {
                            c.Register();
                        }
                        else if (c.StartsWith(input, "/talkto"))
                        {
                            c.InitiateConversation(currentToken);
                        }
                        else if (input == "/ping")
                        {
                            c.PingServer();
                        }
                        else if (input == "/quit")
                        {
                            bQuit = true;
                        }
                        else
                        {
                            printw("Invalid command!\n");
                        }
                    }
                }
                else
                {
                    c.SendMessageToUser(currentToken, currentConversatorID);
                }
                
                break;
            }
        }

        if (c.IsConnected())
        {
    
            if (!c.Incoming().empty())
            {
                auto msg = c.Incoming().pop_front().msg;
                
                switch (msg.header.id)
                {

                    case CustomMsgTypes::ServerAccept:
                    {
                        move(0,0);
                        printw("Connection established with server! \n");
                        move(LINES - 1, 0);
                        break;
                    }
                        

                    case CustomMsgTypes::ServerPing:
                    {
                        std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
                        std::chrono::system_clock::time_point timeThen;
                        msg >> timeThen;
                        printw("'Ping' %f\n", std::chrono::duration<double>(timeNow - timeThen).count());
                        break;
                    }

                    case CustomMsgTypes::Login:
                    {
                        char res[200];
                        msg >> res;

                        if (strlen(res) >= 54)
                        {
                            currentToken = "";

                            for (int i = 0; i < strlen(res); i++)
                            {
                                currentToken += res[i];
                            }

                            printw("Logged in!");
                        }
                        else
                        {
                            printw("%s", res);
                        }
        
                        
                        break;
                    }

                    case CustomMsgTypes::Register:
                    {
                        char res[200];
                        msg >> res;

                        printw("%s", res);
                        break;
                    }

                    case CustomMsgTypes::InitiateConversation:
                    {
                        char res[200];
                        msg >> res;

                        std::string stringMsg(res);


                        try
                        {
                            currentConversatorID = std::stoi(stringMsg);
                            printw("Connected to user with client id: %s", res);
                            c.SetupConversation(currentConversatorID); 
                            
                        }
                        catch (const std::exception& e)
                        {
                            printw("Failed to connect to user, are they offline?");
                            break;
                        }

                        

                        break;
                    }

                    case CustomMsgTypes::SendMessageToUser:
                    {
                        char res[5000];
                        msg >> res;

                        conversationLineCount++;

                        if (conversationLineCount > (LINES - 1))
                        {
                            conversationLineCount = 0;
                            erase();
                        }

                        move(conversationLineCount, 0);
                        attron(COLOR_PAIR(1));
                        printw("%s: ", currentLoggedInUser.c_str());
                        attron(COLOR_PAIR(4));
                        printw("%s", res);
                        move(LINES - 1, 0);
                        break;
                    }

                    case CustomMsgTypes::ReceiveMessageFromUser:
                    {
                        char res[5000];
                        msg >> res;

                        if (conversationLineCount > (LINES - 1))
                        {
                            conversationLineCount = 0;
                            erase();
                        }
                        conversationLineCount++;
                        attron(COLOR_PAIR(2));
                        move(conversationLineCount, 0);
                        printw("%s: ", currentConverser.c_str());
                        attron(COLOR_PAIR(4));
                        printw("%s", res);
                        move(LINES - 1, 0);
                        break;
                    }
                                
                    
                }
            }
        }
        else
        {
            std::cout << "Server is down" << std::endl;
            bQuit = true;
        }
    }

    endwin();
    return 0;   
}
