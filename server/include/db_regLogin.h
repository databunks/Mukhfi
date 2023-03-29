#include <db_regLoginHelper.h>

class RegistrationLogin
{
    private:
        // setting up mongodb instance
        mongocxx::instance instance{};

        mongocxx::options::client client_options;

        // Database credentials
        std::string dbUsername{ ReadFromEnvFile("username") };
        std::string dbPassword{ ReadFromEnvFile("password") };
        std::string dbCluster{ ReadFromEnvFile("cluster") };

        // establishing a connection to the database
        std::string uriString{ std::string( "mongodb+srv://" ) + dbUsername + std::string(":") + dbPassword + std::string( "@" ) + dbCluster + std::string( ".2ev07.mongodb.net/?retryWrites=true&w=majority") };
        mongocxx::uri mongoURI = mongocxx::uri{ uriString };


        mongocxx::client client{mongoURI, client_options };
        mongocxx::database db = client["Mukhfi"];
        mongocxx::collection users = db["users"];

        // Helper functions used in registering and logging in
        HelperFunctions h;
        

    public:
    
    // Logging in the user 
        std::string LoginUser(std::string username, std::string password)
        {
            RegistrationLoginCodes usernameVerificationCode {h.ValidateUsername(username)};
            RegistrationLoginCodes passwordVerificationCode {h.ValidatePassword(password)};

            if ((usernameVerificationCode != RegistrationLoginCodes::Success) | (passwordVerificationCode != RegistrationLoginCodes::Success))
            {
                return std::to_string(u_int32_t(usernameVerificationCode) | uint32_t(passwordVerificationCode));
            }
            
            try
            {
                //check if there is a current matching username in the database
                bsoncxx::stdx::optional<bsoncxx::document::value> result =
                users.find_one(document {} << "username" << username << finalize);

                if(result) 
                {
                    std::cout << "[INFO] User exists, checking for matching password...." << std::endl;

                    // if user exists check for matching password
                    result = users.find_one(document {} << "username" << username << "password" << password << finalize);

                    if (result)
                    {
                        std::cout << "[Success] Matching password found!, Generating token...." << std::endl;
                        return h.GenerateToken();
                    }
                    else 
                    {
                        std::cerr << "[ERROR] Failed to find user with matching password" << std::endl;
                        return std::to_string(u_int32_t(RegistrationLoginCodes::PasswordNoMatchFound));
                    }

                }
                else
                {
                    std::cerr << "[ERROR] User does not exist" << std::endl;
                    return std::to_string(u_int32_t(RegistrationLoginCodes::UserAlreadyExists));
                }
        
                return std::to_string(u_int32_t(RegistrationLoginCodes::Success));
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
                return std::to_string(u_int32_t(RegistrationLoginCodes::DatabaseError));
            }

            
        }
        

    public:
    // Registering the user
        uint32_t RegisterUser(std::string username, std::string password)
        {
            RegistrationLoginCodes usernameVerificationCode {h.ValidateUsername(username)};
            RegistrationLoginCodes passwordVerificationCode {h.ValidatePassword(password)};

            if ((usernameVerificationCode != RegistrationLoginCodes::Success) | (passwordVerificationCode != RegistrationLoginCodes::Success))
            {
                return u_int32_t(usernameVerificationCode) | uint32_t(passwordVerificationCode);
            }
            
            try
            {
                
                //check if there is a current matching username in the database
                bsoncxx::stdx::optional<bsoncxx::document::value> result =
                users.find_one(document {} << "username" << username << finalize);

                if(!result) 
                {
                    // if not then register them in 
                    auto builder = bsoncxx::builder::stream::document{};
                    bsoncxx::document::value doc_value = builder
                    << "username" << username
                    << "password" << password
                    << bsoncxx::builder::stream::finalize;

                    bsoncxx::stdx::optional<mongocxx::result::insert_one> result = users.insert_one(doc_value.view());
                    if (result)
                    {
                        std::cout << "[SUCCESS] Successfully registered user" << std::endl;
                        return u_int32_t(RegistrationLoginCodes::Success);
                    }
                    else 
                    {
                        std::cerr << "[ERROR] Failed to register user" << std::endl;
                        return u_int32_t(RegistrationLoginCodes::DatabaseError);
                    }

                }
                else
                {
                    std::cerr << "[ERROR] User already exists" << std::endl;
                    return u_int32_t(RegistrationLoginCodes::UserAlreadyExists);
                }
        
                return u_int32_t(RegistrationLoginCodes::Success);
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
                return u_int32_t(RegistrationLoginCodes::DatabaseError);
            }
            
           
        }

    private:

        // Reads authentication parameters from env file
        std::string ReadFromEnvFile(std::string keyToFind)
        {
            std::ifstream envFile;
            envFile.open("../../auth/DatabaseCredentials.env");
            std::string envVar;

            if (envFile.is_open())
            {
                while (envFile)
                {
                    std::getline(envFile, envVar);

                    std::string key{envVar.substr(0, envVar.find("="))};

                   
                    if (keyToFind == key)
                    {
                        return envVar.substr(envVar.find("=") + 1, envVar.length());
                    }
                }
            }
            std::string error{"[ERROR] Could not find key"};
            std::cerr << error;

            return error;
        }        
};