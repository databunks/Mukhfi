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

            // // let the terminal do the line editing
            // nocbreak();
            // echo();

            // this reads from buffer after <ENTER>, not "raw" 
            // so any backspacing etc. has already been taken care of
            int ch = getch();

            while ( ch != '\n' )
            {
                input.push_back( ch );
                ch = getch();
            }

            // restore your cbreak / echo settings here

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

        void Login()
        {
            printw("Enter Username:\n");  
            std::string username;
            std::cin >> username;

            printw("Enter Password: ");
            std::string password;
            std::cin >> password;

            std::string fullMessage;
            fullMessage = username + " " + password;

            olc::net::message<CustomMsgTypes> msg;
            msg.header.id = CustomMsgTypes::Login;

            std::cout << fullMessage;

            msg << fullMessage.c_str();

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
                        char* res;
                        msg >> res;
                        // std::cout << res;

                        printw("Aye sir\n");
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
