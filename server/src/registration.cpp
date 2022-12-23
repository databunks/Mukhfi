#include <cstdint>
#include <iostream>
#include <vector>
#include <string>

#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/instance.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;


int main(int argc, char *argv[])
{
    // MongoCXX setup

    mongocxx::instance instance{};

    int requiredNumberOfArguments = 4;

    if (argc < requiredNumberOfArguments)
    {
        std::cerr << "Invalid number of arguments (only " << argc - 1 << " passed in, need " << requiredNumberOfArguments << ")" << std::endl;
        return 1;
    }

    std::string username {argv[1]};
    std::string password {argv[2]}; 
    std::string cluster {argv[3]};

    std::string uriString = std::string("mongodb+srv://") + username + std::string(":") + password + std::string("@") + cluster + std::string(".xlmttbn.mongodb.net/?retryWrites=true&w=majority");
    
    mongocxx::client client{mongocxx::uri{uriString.c_str()}};

    mongocxx::database db = client[cluster];

    // TODO: Setup Asio to receive registration details

    // TODO: Save And Verify Information To Save To Database

    return 0;
}
