#include <iostream>
#include <olc_net.h>
#include <ncurses.h>


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

class CustomClient : public olc::net::client_interface<CustomMsgTypes>
{
    private:
        

        std::string getstring()
        {
            std::string input;

            char inputChar[200];
            refresh();
             // int currentChar = getch();

            // while (currentChar != '\n')
            // {
            //     currentChar = getch();
            // }
            getstr(inputChar);

            // int currentChar = getch();

            // while (currentChar != '\n')
            // {
            //     currentChar = getch();

            // printw("You entered: \n");

            // for (int i = 0; i < 100; i++)
            // {
            //     //printw("%c", line[i]);
            // }

            printw("Input: %s\n", inputChar);

            return input;
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
            printw("Enter Username:\n");  
            std::string username; //= getstring();
            std::cin >> username;

            printw("Enter Password:\n");
            std::string password; //= getstring();
            std::cin >> password;


            std::string fullMessage;
            fullMessage = username + " " + password;
            

            SendMessageToServer(fullMessage, CustomMsgTypes::Login);
        }

        void Register()
        {
            printw("Enter Username:\n");  
            std::string username; //= getstring();
            std::cin >> username;

            printw("Enter Password:\n");
            std::string password; //= getstring();
            std::cin >> password;


            std::string fullMessage;
            fullMessage = username + " " + password;

            std::cout << fullMessage;
            

            SendMessageToServer(fullMessage, CustomMsgTypes::Register);
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
    
    while (!bQuit)
    {
        int ch = getch();

        switch (ch)
        {
            case KEY_UP:
            {
                c.PingServer();
                break;
            }

            case KEY_DOWN:
            {
                c.InitiateConversation(currentToken);
                break;
            }
            
            case KEY_RIGHT:
            {
                c.Login();
                break;
            }

            case KEY_LEFT:
            {
                c.Register();
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
                        printw("Connection established with server, validating....\n");
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
    return 0;   
}
