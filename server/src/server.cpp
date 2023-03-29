#include <iostream>
#include <olc_net.h>
#include <db_regLogin.h>



enum class CustomMsgTypes : uint32_t
{
    ServerAccept,
    ServerDeny,
    ServerPing,
    MessageAll,
    ServerMessage,
    Login,
};

class CustomServer : public olc::net::server_interface<CustomMsgTypes>
{
    RegistrationLogin r;

    public:
        // The : thing is just inherting from the server_interface class
        CustomServer(uint16_t nPort) : olc::net::server_interface<CustomMsgTypes>(nPort)
        {

        }

    protected:

        // Called when the client has appeared to be connected
        virtual bool OnClientConnect(std::shared_ptr<olc::net::connection<CustomMsgTypes>> client)
        {
            olc::net::message<CustomMsgTypes> msg;
            msg.header.id = CustomMsgTypes::ServerAccept;
            client ->  Send(msg);
            return true;
        }

        // Called when the client appears to have disconnected
        virtual void OnClientDisconnect(std::shared_ptr<olc::net::connection<CustomMsgTypes>> client)
        {
            std::cout << "Disconnecting this cunt: [" << client->GetID() << "]\n";
        }

        // Called when a message arrives
        virtual void OnMessage(std::shared_ptr<olc::net::connection<CustomMsgTypes>> client, olc::net::message<CustomMsgTypes>& msg)
        {
            switch (msg.header.id)
            {
                
                case CustomMsgTypes::ServerPing:
                {
                    std::cout << "[" << client->GetID() << "]: Server Ping!\n";
                    
                    // Send the message back to the client
                    client->Send(msg);
                    break;
                }
                        

                case CustomMsgTypes::ServerMessage:
                {
                    uint32_t clientID;
                    msg >> clientID;
                    std::cout << "Pong!, Hi there mr [" << clientID << "] Welcome to my shitty server\n";
                    break;
                }

                case CustomMsgTypes::Login:
                {
                   std::cout << "Received login request\n";
                   char loginDetails[200];

                   msg >> loginDetails;

                   std::string loginDetailsStr{loginDetails};

                   std::string username{loginDetailsStr.substr(0, loginDetailsStr.find(" "))};

                   std::string password{loginDetailsStr.substr(loginDetailsStr.find(" ") + 1, loginDetailsStr.length())};

                   std::string loginRes{r.LoginUser(username, password)}; 

                   olc::net::message<CustomMsgTypes> msg;
                   msg.header.id = CustomMsgTypes::Login;

                   char msgToSend[200];

                   for (int i = 0; i < loginRes.size(); i++)
                   {
                        msgToSend[i] = loginRes[i];
                   }

                   msg << msgToSend;

                   client->Send(msg);
                    break;

                }

                    
            }
        }
};

int main()
{
    CustomServer server(60000);
    server.Start();

    while (true)
    {
        server.Update(-1, true);
    }
    return 0;
}