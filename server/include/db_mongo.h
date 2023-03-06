// #include <bsoncxx/json.hpp>
// #include <mongocxx/client.hpp>
// #include <mongocxx/stdx.hpp>
// #include <mongocxx/uri.hpp>
// #include <mongocxx/instance.hpp>
// #include <bsoncxx/builder/stream/helpers.hpp>
// #include <bsoncxx/builder/stream/document.hpp>
// #include <bsoncxx/builder/stream/array.hpp>


// class DatabaseFunctions
// {
//     void ConnectToDatabase()
//     {
//         mongocxx::instance instance{};
//         mongocxx::options::client client_options;

//         auto api = mongocxx::options::server_api{mongocxx::options::server_api::version::k_version_1};
//         client_options.server_api_opts (api);

//         // Database credentials
//         std::string dbUsername{ ReadFromEnvFile("username") };
//         std::string dbPassword{ ReadFromEnvFile("password") };
//         std::string dbCluster{ ReadFromEnvFile("cluster") };

//         // establishing a connection to the database
//         std::string uriString{ std::string( "mongodb+srv://" ) + dbUsername + std::string(":") + dbPassword + std::string( "@" ) + dbCluster + std::string( ".2ev07.mongodb.net/?retryWrites=true&w=majority") };
//         mongocxx::uri mongoURI = mongocxx::uri{ uriString };

//         mongocxx::client client{mongoURI, client_options } ;
//         mongocxx::database db = client[dbCluster];
//     }
// };

