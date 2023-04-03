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
        int currentPressedChar = getch();

        while (currentPressedChar != '\n' )
        {
            currentPressedChar = getch();
        }

        move(LINES - 1, 0);
        int y, x;
        getyx(stdscr, y, x);

        std::string input{};

        for (int i = 0; i < x; i++)
        {
            move(y, i);
            char currentChar = inch();
            input += currentChar;
        }

        printw("This: %s", input.c_str());
        move(LINES - 1, 0);
        clrtoeol();

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
            std::string username = getstring();
            //std::cin >> username;

            printw("Enter Password:\n");
            std::string password; // = getstring();
            //std::cin >> password;


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
    move(LINES - 1, 0);


    while (!bQuit)
    {
        int ch = getch();
        

        switch (ch)
        {
            
            case KEY_BACKSPACE:
            {
                printw(" ");

                int y, x;
                getyx(stdscr, y, x);

                if (x > 0)
                move(y, x - 1);

                break;
            }

            case '\n':
            {
                move(LINES - 1, 0);

                int y, x;
                getyx(stdscr, y, x);

                std::string input{};

                for (int i = 0; i < x; i++)
                {
                    move(y, i);
                    char currentChar = inch();
                    input += currentChar;
                }

                move(LINES - 1, 0);
                clrtoeol();

                printw("This: %s", input.c_str());
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
