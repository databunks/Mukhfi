#pragma once
#include "net_common.h"
#include "net_tsqueue.h"
#include "net_message.h"
#include "net_connection.h"

namespace olc
{
    namespace net
    {
        template<typename T>
        class server_interface
        {
            public:
                // I think the function after : initializes it???
                server_interface(uint16_t port) : m_asioAcceptor(m_asioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
                {

                }

                virtual ~server_interface()
                {
                    Stop();
                }

                bool Start()
                {
                    try
                    {
                        WaitForClientConnection();
                        m_threadContext = std::thread([this]() {m_asioContext.run(); });
                    }
                    catch (const std::exception& e)
                    {
                        // Something prohibited the server from listening
                        std::cerr << "[SERVER] Exception: " << e.what() << '\n';
                        return false;
                    }

                    std::cout << "[SERVER] Started! \n";
                    return true;
                }

                void Stop()
                {
                    // Request the context to close
                    m_asioContext.stop();

                    // Tidy up context thread 
                    if (m_threadContext.joinable())
                    {
                        m_threadContext.join();
                    }

                    // Inform that the server has closed
                    std::cout << "[SERVER] Stopped!\n";
                }

                // Async - wait for asio to wait for connection
                void WaitForClientConnection()
                {
                    m_asioAcceptor.async_accept(
                        [this](std::error_code ec, asio::ip::tcp::socket socket)
                        {
                            if (!ec)
                            {
                                std::cout << "[SERVER] New Connection " << socket.remote_endpoint() << "\n";

                                std::shared_ptr<connection<T>> newConn =  
                                    std::make_shared<connection<T>>(connection<T>::owner::server, 
                                    m_asioContext, std::move(socket), m_qMessagesIn);

                                // Give the user server a chance to deny the connection
                                if (OnClientConnect(newConn))
                                {
                                    // Connection allowed, so add to container of new connections
                                    m_deqConnections.push_back(std::move(newConn));

                                    m_deqConnections.back() -> ConnectToClient(this, nIDCounter++);

                                    std::cout << "[" << m_deqConnections.back() -> GetID() << "] Connection Approved\n";
                                }
                                else
                                {
                                    std::cout << "[-----] Connection Denied\n";
                                }
                            }
                            else
                            {
                                // Error has occured during acceptance 
                                std::cout << "[SERVER] New Connection Error: " << ec.message() << "\n";
                            }

                            // Prime the asio context with more work - (wait for connection)
                            WaitForClientConnection();
                        });
                }



                // Send message to a specific client
                void MessageClient(std::shared_ptr<connection<T>> client, const message<T>& msg)
                {
                    // Making sure shared pointer is valid (remember nullptrs?)
                    if (client && client -> IsConnected())
                    {
                        client->Send(msg);
                    }
                    else
                    {
                        // Assumes the client has disconnected
                        // So we entirely remove it
                        OnClientDisconnect(client);
                        client.reset();

                        // If we have many clients this erase can be pretty expensive
                        m_deqConnections.erase(
                            std::remove(m_deqConnections.begin(), m_deqConnections.end(), client), m_deqConnections.end());
                    }
                }

                // Send message to all clients
                void MessageAllClients(const message<T>& msg, std::shared_ptr<connection<T>> pIgnoreClient = nullptr)
                {
                    bool bInvalidClientExists = false;
                    
                    for (auto& client : m_deqConnections)
                    {
                        
                        // Check if client is connected
                        if (client && client -> IsConnected())
                        {
                    
                            if (client != pIgnoreClient)
                            {
                                client->Send(msg);
                            }
                        }
                        else
                        {
                            // The client cant be connected so assume its disconnected
                            OnClientDisconnect(client);
                            client.reset();
                            bInvalidClientExists = true;
                        }
                    }

                    if (bInvalidClientExists)
                    {
                        m_deqConnections.erase(std::remove(m_deqConnections.begin(), m_deqConnections.end(), nullptr), m_deqConnections.end());
                    }
                }

                // since size_t is an unsigned integer setting it to -1 sets it to the maximum integer
                void Update(size_t nMaxMessages = -1, bool bWait = false)
                {

                    // Reduces cpu usage
                    if (bWait) 
                    {
                        m_qMessagesIn.wait();
                    }

                    size_t nMessageCount = 0;
                    while (nMessageCount < nMaxMessages && !m_qMessagesIn.empty())
                    {
                        // Grab the front message
                        auto msg = m_qMessagesIn.pop_front();

                        // Pass to message handler
                        OnMessage(msg.remote, msg.msg);

                        nMessageCount++;
                    }
                }

        protected:
            // Called when the client connects , you can reject the connection by returning false
            virtual bool OnClientConnect(std::shared_ptr<connection<T>> client)
            {
                return false;
            }

            // Called when a client appears to have disconnected
            virtual void OnClientDisconnect(std::shared_ptr<connection<T>> client)
            {

            }

            // Called when a message arrives
            virtual void OnMessage(std::shared_ptr<connection<T>> client, message<T>& msg)
            {

            }

        public:
            // Called when a client is validated
            virtual void OnClientValidated(std::shared_ptr<connection<T>> client)
            {

            }

        protected:
            // Thread safe Queue for incoming message packets
            tsqueue<owned_message<T>> m_qMessagesIn;

            // Container of active validated connections 
            std::deque<std::shared_ptr<connection<T>>> m_deqConnections;

            // Order of declaration is important - it is also the order of initialization
            asio::io_context m_asioContext;
            std::thread m_threadContext;

            // These things need an asio context
            asio::ip::tcp::acceptor m_asioAcceptor;

            // Clients will be identified in the wider system via an ID
            uint32_t nIDCounter = 10000;

        

        public:

            // void RemoveToken(std::vector<std::string> &tokens, std::string token)
            // {
            //     std::vector<std::string>::iterator it = std::find(tokens.begin(), tokens.end(), token);

            //     if (*(tokens.end() - 1) == *it)
            //     {
            //         tokens.pop_back();
            //     }
            //     else
            //     {
            //         tokens.erase(it);
            //     }
                
            // }

        };
    }
}