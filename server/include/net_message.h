#pragma once // #pragma once is a preprocessor directive used to prevent header files from being included multiple times. The #pragma once directive, once present in a file, assures that the file will not be included multiple times in the current project.
#include "net_common.h"

namespace olc
{
    namespace net
    {
        // Message Header is sent at start of all messages. The template allows us
        // to use "enum class" to ensure that the messages are valid at compile time

        template <typename T> // T is the enum class
        struct message_header
        {
            T id{};
            uint32_t size = 0; // use uint32 because it is not gauranteed size_t is same on 32/64 bit systems (and uint32 is enough size)
        };

        template <typename T>
        struct message
        {
            message_header<T> header{};
            std::vector<uint8_t> body;

            // returns size of entire message packet in bytes
            size_t size() const
            {
                return sizeof(message_header<T>) + body.size();
            }

            // Override for std::cout compatibility - produces friendly description of message
            friend std::ostream& operator << (std::ostream& os, const message<T>& msg)
            {
                os << "ID:" << int(msg.header.id) << " Size:" << msg.header.size;
                return os;
            }

            // Pushes any POD (Plain old data)- like data into the message buffer
            template<typename DataType>
            friend message<T>& operator << (message<T>& msg, const DataType& data)
            {
                // Check that the type of the data being pushed is trivially copyable (that its part of the standard library)
                static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed into vector");

                // Cache current size of vector, as this will be the point where you insert the data 
                size_t i = msg.body.size();

                // Resize the vector by the size of the data being pushed
                msg.body.resize(msg.body.size() + sizeof(DataType));

                // Physically copy data into the newly allocated vector space (body vector)
                std::memcpy(msg.body.data() + i, &data, sizeof(DataType));

                // Since the body vector has changed in size we recalculate the size of it
                msg.header.size = msg.size();

                // return the target message so it can be "chained"
                return msg;
            }

            template<typename DataType>
            friend message<T>& operator >> (message<T> msg, DataType& data)
            {
                // Check that the type of the data being pushed is trivially copyable (that it is part of the standard library)
                static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed into vector");
                
                // Cache the location  towards the end of the vector when the pulled data starts
                size_t i = msg.body.size() - sizeof(DataType);

                // Physically copy the data from the vector into the user variable
                std::memcpy(&data, msg.body.data() + i, sizeof(DataType));

                // Shrink the vector to remove read bytes and rest the position
                msg.body.resize(i);

                // Recalculate the message size 
                msg.header.size = msg.size();   

                return msg;
            }
        };

        // Forward declare the connection
        template <typename T>
        class connection;

        template <typename T>
        struct owned_message
        {
            std::shared_ptr<connection<T>> remote = nullptr;
            message<T> msg;

            // A friendly string maker :3
            friend std::ostream& operator << (std::ostream& os, const owned_message<T>& msg)
            {
                os << msg.msg;
                return os;
            }
        };

        
    }
}