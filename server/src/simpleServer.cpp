#include <iostream>
#include <olc_net.h>

enum class CustomMsgTypes : uint32_t
{
    ServerAccept,
    ServerDeny,
    ServerPing,
    MessageAll,
    ServerMessage,
};

class CustomServer : public olc::net::server_interface<CustomMsgTypes>
{
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

                case CustomMsgTypes::MessageAll:
                {
                    std::cout << "[" << client -> GetID() << "] wants to say hello to everyone\n";
                    olc::net::message<CustomMsgTypes> msg;
                    msg.header.id = CustomMsgTypes::ServerMessage;
                    msg << client -> GetID();
                    MessageAllClients(msg, client);
                    std::cout << client << std::endl;
                    break;
                }
                        

                case CustomMsgTypes::ServerMessage:
                {
                    uint32_t clientID;
                    msg >> clientID;
                    std::cout << "Pong!, Hi there mr [" << clientID << "] Welcome to my shitty server\n";
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
        server.Update();
    }
    return 0;
}