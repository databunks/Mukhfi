namespace olc
{
    namespace net
    {
        template <typename T>
        class client_interface
        {
            public:

                // Connect to server with hostname/ip-address and port
                bool Connect(const std::string& host, const uint16_t port)
                {
                    try
                    {
                        // Create Connection
                        m_connection = std::make_unique<connection<T>>(); // TODO

                        // Resolve the hostname / ip-address into tangible physical address
                        asio::ip::tcp::resolver resolver(m_context);
                        m_endpoints = resolver.resolve(host, std::to_string(port));

                        // Tell the connection object to connect to the server
                        m_connection -> ConnectToServer(m_endpoints);

                        // Start context thread
                        thrContext = std::thread([this]() {m_context.run(); });
                    }
                    catch (const std::exception& e)
                    {
                        std::cerr << "Client Exception: " << e.what() << '\n';
                    }
                    
                }

                // Disconnect from server
                void Disconnect()
                {
                    // If connection exists, and it's connected then
                    if (IsConnected())
                    {
                        // Disconnect from server gracefully
                        m_connection->Disconnect();
                    }

                    // either way we are also done with asio context:
                    m_context.stop();

                    // and its thread:
                    if (thrContext.joinable())
                    {
                        thrContext.join();
                    }

                    // Destroy the connection object
                    m_connection.release();
                }

                // Check if client is actually connected to the server
                bool IsConnected()
                {
                    if (m_connection)
                    {
                        return m_connection -> IsConnected();
                    }
                    else
                    {
                        return false;
                    }
                }

                // Retrieve queue of messages from the server
                tsqueue<owned_message<T>>& Incoming()
                {
                    return m_qMessagesIn;
                }

            protected:
                // asio context handles the data transfer
                asio::io_context m_context;

                //.. but it also needs a thread of its own if it is to execute its work commands
                std::thread thrContext;

                // This is the hardware socket that is connected to the server
                asio::ip::tcp::socket m_socket;

                // The client has a single instance of a "connection" object, handles data transfer
                std::unique_ptr<connection<T>> m_connection;


            private:
                // This is the thread safe queue of incoming messages from the server
                tsqueue<owned_message<T>> m_qMessagesIn;
        };
    }
}