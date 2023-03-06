#include <cstdint>
#include <iostream>
#include <vector>
#include <string>
#include <cwctype>
#include <regex>
#include <sstream>
#include <fstream>


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

// Codes which display the status of the registration process
// The enums are formatted in a sense of each enum representing a bit
// i.e enumVar 1 = 2^0, enumVar 2 = 2^1
// These are the flags to represent what the error code is

enum class RegistrationCodes : uint32_t
{
    Success = 1,
    UsernameLong = 2,
    UsernameShort = 4,
    UsernameInvalidCharacters = 8,
    PasswordLong = 16,
    PasswordNotHashed = 32,
    // PasswordInvalidCharacters = 32,
    // PasswordSpecialCharacterEmpty = 64,
    // PasswordNumberEmpty = 128,
    // PasswordCapitalLetterEmpty = 256,
    // PasswordLowercaseLetterEmpty = 512,
    UserAlreadyExists = 64,
    UserDoesNotExist = 128,
    DatabaseError = 256,
};


class RegistrationLogin
{
    private:
        mongocxx::database InstantiateDatabaseConnection()
        {
            // setting up mongodb instance
            mongocxx::instance instance{};

            mongocxx::options::client client_options;

            auto api = mongocxx::options::server_api{mongocxx::options::server_api::version::k_version_1};
            client_options.server_api_opts (api);

            // Database credentials
            std::string dbUsername{ ReadFromEnvFile("username") };
            std::string dbPassword{ ReadFromEnvFile("password") };
            std::string dbCluster{ ReadFromEnvFile("cluster") };

            // establishing a connection to the database
            std::string uriString{ std::string( "mongodb+srv://" ) + dbUsername + std::string(":") + dbPassword + std::string( "@" ) + dbCluster + std::string( ".2ev07.mongodb.net/?retryWrites=true&w=majority") };
            mongocxx::uri mongoURI = mongocxx::uri{ uriString };

            std::cout << uriString << std::endl;

            mongocxx::client client{mongoURI, client_options };
            mongocxx::database db = client["Mukhfi"];
            return db;
        }

    public:
    // Logging in the user 
        std::string LoginUser(std::string username, std::string password)
        {
            RegistrationCodes usernameVerificationCode {ValidateUsername(username)};
            RegistrationCodes passwordVerificationCode {ValidatePassword(password)};

            if ((usernameVerificationCode != RegistrationCodes::Success) | (passwordVerificationCode != RegistrationCodes::Success))
            {
                //return std::string(u_int32_t(usernameVerificationCode) | uint32_t(passwordVerificationCode));
            }

            try
            {
                mongocxx::database db = InstantiateDatabaseConnection();
                mongocxx::collection users = db["users"];
                
                //check if there is a current matching username in the database
                bsoncxx::stdx::optional<bsoncxx::document::value> result =
                users.find_one(document {} << "username" << username << finalize);

                if(result) 
                {
                    // if user exists log them in
                    auto builder = bsoncxx::builder::stream::document{};
                    bsoncxx::document::value doc_value = builder
                    << "username" << username
                    << "password" << password
                    << bsoncxx::builder::stream::finalize;

                    bsoncxx::stdx::optional<mongocxx::result::insert_one> result = users.insert_one(doc_value.view());
                    if (result)
                    {
                        std::cout << "[INFO] User exists, Generating token...." << std::endl;
                    }
                    else 
                    {
                        std::cerr << "[ERROR] Failed to find user" << std::endl;
                    }

                }
                else
                {
                    std::cerr << "[ERROR] User does not exist" << std::endl;
                    //return std::string(u_int32_t(RegistrationCodes::UserAlreadyExists));
                }
        
                //return u_int32_t(RegistrationCodes::Success);
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
                //return u_int32_t(RegistrationCodes::DatabaseError);
            }

            
        }

    private:
        // Generate the token
        std::string GenerateToken()
        {
            std::mt19937_64 r;
        }

    public:
    // Registering the user
        uint32_t RegisterUser(std::string username, std::string password)
        {
            RegistrationCodes usernameVerificationCode {ValidateUsername(username)};
            RegistrationCodes passwordVerificationCode {ValidatePassword(password)};

            if ((usernameVerificationCode != RegistrationCodes::Success) | (passwordVerificationCode != RegistrationCodes::Success))
            {
                return u_int32_t(usernameVerificationCode) | uint32_t(passwordVerificationCode);
            }
            
            try
            {
                mongocxx::database db = InstantiateDatabaseConnection();
                mongocxx::collection users = db["users"];
                
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
                    }
                    else 
                    {
                        std::cerr << "[ERROR] Failed to register user" << std::endl;
                    }

                }
                else
                {
                    std::cerr << "[ERROR] User already exists" << std::endl;
                    return u_int32_t(RegistrationCodes::UserAlreadyExists);
                }
        
                return u_int32_t(RegistrationCodes::Success);
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
                return u_int32_t(RegistrationCodes::DatabaseError);
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

    private:

        // Checking if the username is valid
        RegistrationCodes ValidateUsername(std::string username)
        {
            int usernameLen = username.length();

            // Checking if username length is too long
            if (usernameLen > 20)
            {
                return RegistrationCodes::UsernameLong;
            }

            // Checking if username length is too short
            if (usernameLen <= 4)
            {
                return RegistrationCodes::UsernameShort;
            }

            // Checking if username is alphanumeric
            std::regex alnum("^[A-Za-z0-9]+$"); 
            if (!std::regex_match(username,alnum))
            {
                return RegistrationCodes::UsernameInvalidCharacters;
            }
            
            return RegistrationCodes::Success;
        }

    private:

        // Password Strength should be validated at client side pre hash
        // Checking if the password is valid
        RegistrationCodes ValidatePassword(std::string password)
        {
            int passwordLen = password.length();

            std::regex alnum("^[A-Za-z0-9]+$"); 

            // Checking password is sha256 hash
            if ( (passwordLen != 64) | (!std::regex_match(password, alnum)) )
            {
                std::cout << "Test";
                return RegistrationCodes::PasswordNotHashed;
            }
            
            // if (passwordLen < 64)
            // {
            //     return RegistrationCodes::PasswordShort;
            // }

            // // checking if it contains valid characters
            // std::regex validPwdChars(R"(^[A-Za-z0-9~!@#$%^&*\(\)\_\-\+=\{\[\}\]\|\:;"'<,>\?]+$)"); 
            
            // if (!std::regex_match(password, validPwdChars))
            // {
            //     return RegistrationCodes::PasswordInvalidCharacters;
            // }

            // // Checking if it contains at least one special character
            // std::regex specialCharacterCheck(R"(^[~!@#$%^&*\(\)\_\-\+=\{\[\}\]\|\:;"'<,>\?]+$)");

            // if (!std::regex_match(password, specialCharacterCheck))
            // {
            //     return RegistrationCodes::PasswordSpecialCharacterEmpty;
            // }

            // // Checking if it has at least one capital letter
            // std::regex capitalLetterCheck(R"(^[A-Z]+$)");

            // if (!std::regex_match(password, capitalLetterCheck))
            // {
            //     return RegistrationCodes::PasswordCapitalLetterEmpty;
            // }

            // // Checking if it has at least one lower case
            // std::regex lowercaseLetterCheck(R"(^[a-z]+$)");

            // if (!std::regex_match(password, lowercaseLetterCheck))
            // {
            //     return RegistrationCodes::PasswordLowercaseLetterEmpty;
            // }

            return RegistrationCodes::Success;
            
        }
};


int main()
{
    RegistrationLogin r;
    printf("%d", r.RegisterUser("MrBeoomblastic", "c8fea865a2ded626c6882616f7703e25deeafbf2f65ad25d29cea8a6a879f32f"));
}