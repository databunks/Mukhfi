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
            // }

            // printw("You entered: \n");

            // for (int i = 0; i < 100; i++)
            // {
            //     //printw("%c", line[i]);
            // }

            printw("Input: %s\n", inputChar);

            return input;
        }


    public:

        void MessageEveryone()
        {
            olc::net::message<CustomMsgTypes> msg;
            msg.header.id = CustomMsgTypes::MessageAll;
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
            printw("Enter Username:\n");  
            std::string username = getstring();

            printw("Enter Password:\n");
            std::string password = getstring();

            std::string fullMessage;
            fullMessage = username + " " + password;
            

            SendMessageToServer(username, CustomMsgTypes::Login);
        }

       
};







int main()
{
    

    CustomClient c;
    c.Connect("127.0.0.1", 60000);
    

    bool bQuit{false};

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
            case KEY_LEFT:
            {
                c.PingServer();
                break;
            }
                
            
            case KEY_UP:
            {
                c.MessageEveryone();
                break;
            }
                

            case KEY_RIGHT:
            {
                c.Login();
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
                        // std::cout << res;

                        printw("Aye sir \n");

                        for (int i = 0; i < strlen(res); i++)
                        {
                            printw("%c",res[i]);
                        }
                        //printw("%s", res);
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
