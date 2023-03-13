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

        std::string Login()
        {
            olc::net::message<CustomMsgTypes> msg;
            std::string username = " MrBeoomblastic";
            std::string password = " c8fea865a2ded626c6882616f7703e25deeafbf2f65ad25d29cea8a6a879f32f";
            std::string code = "1";

            msg.header.id = CustomMsgTypes::Login;

            std::string finalmsg = code + username + password;

            std::cout << finalmsg;
            msg << finalmsg;

            Send(msg);
        }
};





int main()
{
    CustomClient c;
    c.Connect("127.0.0.1", 60000);
    

    bool bQuit{false};

    WINDOW *w = initscr();
    cbreak();
    nodelay(w, TRUE);
    keypad(stdscr, TRUE);

    std::string msg1 = c.Login();

    printw(msg1.c_str());
    
    while (!bQuit)
    {
        int ch = getch();

        switch (ch)
        {
            case KEY_LEFT:
                c.PingServer();
                break;
            
            case KEY_UP:
                c.MessageEveryone();
                break;

            case KEY_RIGHT:
                break;
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

                    case CustomMsgTypes::ServerMessage:
                    {
                        uint32_t id;
                        msg >> id;
                        printw("Your boy [%u] Wants to say hi to everyone :)\n", id);
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
