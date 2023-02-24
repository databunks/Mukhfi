#pragma once

#include "net_common.h"
#include "net_tsqueue.h"
#include "net_message.h"

namespace olc
{
    namespace net
    {
        template<typename T>
        class connection : public std::enable_shared_from_this<connection<T>>
        {
            public:
                // Handles ownership requirement
                enum class owner
                {
                    server,
                    client
                };

                // constructor for connection
                connection(owner parent, asio::io_context& asioContext, asio::ip::tcp::socket socket, tsqueue<owned_message<T>>& qIn)
                   : m_asioContext(asioContext), m_socket(std::move(socket)), m_qMessagesIn(qIn)
                {
                    m_nOwnerType = parent;
                }

                // destructor for connection
                virtual ~connection()
                {}

                uint32_t GetID() const
                {
                    return id;
                }

            public:
                void ConnectToClient(uint32_t uid = 0)
                {
                    if (m_nOwnerType == owner::server)
                    {
                        if (m_socket.is_open())
                        {
                            id = uid;
                            ReadHeader();
                        }
                    }
                }
                bool ConnectToServer();
                bool Disconnect();
                bool IsConnected() const
                {
                    return m_socket.is_open();
                }

            public:
                bool Send(const message<T>& msg)
                {
                    asio::post(m_asioContext, 
                    [this, msg]()
                    {
                        bool bWritingMessage = !mqMessagesOut.empty();
                        m_qMessagesOut.push_back(msg);
                        WriteHeader();
                    });
                }
            
            private:
                // Asynchronously prime the context ready to read the message header
                void ReadHeader()
                {
                    asio::async_read(m_socket, asio::buffer(&m_msgTemporaryIn.header), sizeof(message_header<T>)),
                    [this](std::error_code ec, std::size_t length)
                    {

                        if (!ec)
                        {
                            if (m_msgTemporaryIn.header.size > 0)
                            {
                                m_msgTemporaryIn.body.resize(m_msgTemporaryIn.header.size);
                                ReadBody();
                            }
                            else
                            {
                                AddToIncomingMessageQueue();
                            }
                        }
                        else
                        {
                            std::cout << "[" << id << "] Read Header Fail.\n";
                            m_socket.close();
                        }
                    }
                }

                // Asynchronously prime the context ready to read the message body
                void ReadBody()
                {
                    asio::async_read(m_socket, asio::buffer(m_msgTemporaryIn.body.data(), m_msgTemporaryIn.body.size()),
                    [this](std::error_code ec, std::size_t length)
                    {
                        if (!ec)
                        {
                            AddToIncomingMessageQueue();
                        }
                        else
                        {
                            std::cout << "[" << id << "] Read Body Fail.\n";
                            m_socket.close();
                        }
                    }
                }

                // Asynchronously prime the context to write a message header
                void WriteHeader()
                {
                    asio::async_write(m_socket, asio::buffer(&m_qMessagesOut.front().header, sizeof(message_header<T>))),
                    [this](std::error_code ec, std::size_t length)
                    {
                        if (!ec)
                        {
                            if  (m_qMessagesOut.front().body.size() > 0)
                            {
                                WriteBody();
                            }
                            else
                            {
                                m_qMessagesOut.pop_front();

                                if (!m_qMessagesOut.empty())
                                {
                                    WriteHeader();
                                }
                            }
                        }
                        else
                        {
                            std::cout << "[" << id << "] Write Header Fail.\n";
                            m_socket.close();
                        }
                    }
                }

                // Asynchronously prime the context to write a message body
                void WriteBody()
                {
                    asio::async_write(m_socket, asio::buffer(m_qMessagesOut.front().body.data(), m_qMessagesOut.front().body.size()),
					[this](std::error_code ec, std::size_t length)
					{
						if (!ec)
						{
							m_qMessagesOut.pop_front();

							if (!m_qMessagesOut.empty())
							{
								WriteHeader();
							}
						}
						else
						{
							// Sending failed, see WriteHeader() equivalent for description :P
							std::cout << "[" << id << "] Write Body Fail.\n";
							m_socket.close();
						}
					});
                }

                void AddToIncomingMessageQueue()
                {
                    // transforms message into an owned message (tagged)
                    if (m_nOwnerType == owner::server)
                    {
                        m_qMessagesIn.push_back({ this -> shared_from_this(), m_msgTemporaryIn });
                    }
                    else
                    {
                        // tagging doesent make sense because clients only have 1 connection anyway
                        m_qMessagesIn.push_back({ nullptr, m_msgTemporaryIn });
                    }

                    ReadHeader();
                }

            protected:
                // Each connection has a unique socket to a remote
                asio::ip::tcp::socket m_socket;

                // This context is shared with the whole asio instance
                asio::io_context& m_asioContext;

                // This queue holds all messages to be sent to the remote side of this connection
                tsqueue<message<T>> m_qMessagesOut;

                // This queue holds all messages that have been received from the remote side of the connection
                // Note that it is a reference as the owner of this connection is expected to provide a queue
                tsqueue<owned_message<T>>& m_qMessagesIn; 
                message<T> m_msgTemporaryIn;

                // The owner decides how the connection behaves
                owner m_nOwnerType = owner::server;
                uint32_t id = 0; // allocate identifiers to clients
        };
    }
}   