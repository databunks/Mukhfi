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
};

class CustomClient : public olc::net::client_interface<CustomMsgTypes>
{
    public:
        void PingServer()
        {
            olc::net::message<CustomMsgTypes> msg;
            msg.header.id = CustomMsgTypes::ServerPing;

            // we are sending the time to the server, and the server will send it back so we can compare the time taken
            std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();

            msg << timeNow;

            Send(msg);
        } 
};



int main()
{
    CustomClient c;
    c.Connect("127.0.0.1", 60000);

    bool bQuit{false};

    initscr();
    keypad(stdscr, TRUE);
    
    
    while (!bQuit)
    {
        int ch = getch();

        bool keys[3] = {false, false, false};


        if (ch == KEY_LEFT)
        {
            keys[0] = true;
        }

        switch (ch)
        {
            case KEY_LEFT:
                c.PingServer();
                break;
            
            case KEY_UP:
                break;

            case KEY_RIGHT:
                break;
        }

        if (keys[0]) c.PingServer();

        printw("%d", keys[0]);

        if (c.IsConnected())
        {
            printw("checking...");
            if (!c.Incoming().empty())
            {
                printw("yes,....");
                auto msg = c.Incoming().pop_front().msg;
                
                switch (msg.header.id)
                {
                    case CustomMsgTypes::ServerPing:
                        std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
                        std::chrono::system_clock::time_point timeThen;
                        msg >> timeThen;
                         

                        printw("'Ping' %d\n", std::chrono::duration<double>(timeNow - timeThen).count());
                        break;
                }
            }
        }
        else
        {
            std::cout << "Server is down" << std::endl;
            bQuit = true;
        }

        //getch();
        
    }
    return 0;   
}
