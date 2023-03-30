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
    InitiateConversation,
    Register,
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
            std::cout << "Disconnecting this client: [" << client->GetID() << "]\n";
        }

        std::string ValidateTokenAndSplitMessage(std::string fullMsg)
        {
            std::string token = fullMsg.substr(0, fullMsg.find("!"));

            if (!ValidateToken(fullMsg))
            {
                return std::string{"Invalid Token / Token Expired"};
            }
            
            return fullMsg.substr(fullMsg.find("!") + 1, fullMsg.length());
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

                case CustomMsgTypes::InitiateConversation:
                {
                    char charMsg[200];
                    msg >> charMsg;

                    std::string validationMsg = ValidateTokenAndSplitMessage(std::string{charMsg});
                    char outwardMsg[200];


                    if (validationMsg == "Invalid Token / Token Expired")
                    {
                        std::string InvalidTokenMsg{"Invalid Token / Token Expired"};
                        strcpy(outwardMsg, InvalidTokenMsg.c_str());

                        olc::net::message<CustomMsgTypes> msg;
                        msg.header.id = CustomMsgTypes::InitiateConversation;
                        msg << outwardMsg;

                        break;
                    }

                    char username[200];

                    strcpy(username, validationMsg.c_str());

                    

                    bool usernameFound = false;
                    for (auto connection : m_deqConnections)
                    {
                        if (connection->GetUsername() == std::string(username))
                        {
                            std::string id = std::to_string(connection->GetID());

                            for (int i = 0; i < id.size(); i++)
                            {
                                outwardMsg[i] = id[i];
                            }

                            outwardMsg[id.size()] = '\0';

                            usernameFound = true;
                            olc::net::message<CustomMsgTypes> msg;
                            msg.header.id = CustomMsgTypes::InitiateConversation;
                            msg << outwardMsg;
                            break;
                        }
                    }

                    if (!usernameFound)
                    {
                       std::string failedToFindUsernameMsg{"Failed to find username"};
                       strcpy(outwardMsg, failedToFindUsernameMsg.c_str());
                    }
                }

                case CustomMsgTypes::Register:
                {
                    std::cout << "Received registration request\n";

                    char registerDetails[200];

                    msg >> registerDetails;

                    std::string registerDetailsStr{registerDetails};

                    std::string username{registerDetailsStr.substr(0, registerDetailsStr.find(" "))};

                    std::string password{registerDetailsStr.substr(registerDetailsStr.find(" ") + 1, registerDetailsStr.length())};

                    std::string registerRes{std::to_string(r.RegisterUser(username, password))}; 

                    olc::net::message<CustomMsgTypes> msg;
                    msg.header.id = CustomMsgTypes::Register;

                    char msgToSend[200];

                    for (int i = 0; i < registerRes.size(); i++)
                    {
                        msgToSend[i] = registerRes[i];
                    }

                    msgToSend[registerRes.size()] = '\0';

                    msg << msgToSend;

                    client->Send(msg);
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

                   if (loginRes == "1")
                   {
                        client->SetUsername(username);
                        client->SetCurrentToken(loginRes);
                        AddToken(loginRes);
                   }

                   olc::net::message<CustomMsgTypes> msg;
                   msg.header.id = CustomMsgTypes::Login;

                   char msgToSend[200];

                   for (int i = 0; i < loginRes.size(); i++)
                   {
                        msgToSend[i] = loginRes[i];
                   }

                   msgToSend[loginRes.size()] = '\0';

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