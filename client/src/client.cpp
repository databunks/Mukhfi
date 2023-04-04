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
};

std::string input{};
bool commandMode = true;


class CustomClient : public olc::net::client_interface<CustomMsgTypes>
{
    public:

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
        std::string currentToken;
        
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
            if (currentToken.size() < 1)
            {
                printw("You are not logged in!\n");
                return;
            }
            
            printw("Enter Username:\n");  
            std::string username; //= getstring();
            std::cin >> username;

            std::string fullMsg;

            fullMsg = currentToken + "!" + username;

            SendMessageToServer(username, CustomMsgTypes::InitiateConversation);
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

            case KEY_ENTER:
            {
                c.CaptureInput();
                if (commandMode)
                {
                    if (input == "login")
                    {
                        c.Login();
                    }
                    else if (input == "register")
                    {
                        c.Register();
                    }
                    else if (input == "initiate")
                    {
                        c.InitiateConversation(c.currentToken);
                    }
                    else if (input == "ping")
                    {
                        c.PingServer();
                    }
                    else if (input == "quit")
                    {
                        bQuit = true;
                    }
                    else
                    {
                        printw("Invalid command!\n");
                    }
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
                            currentToken = res;
                        }
        
                        printw("%s", res);
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

                        printw("%s", res);
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
