#include <cstdint>
#include <iostream>
#include <vector>
#include <string>
#include <cwctype>
#include <regex>
#include <sstream>
#include <fstream>

#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>


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
    ValidInput = 0,
    UsernameLong = 1,
    UsernameShort = 2,
    UsernameInvalidCharacters = 4,
    PasswordLong = 8,
    PasswordShort = 16,
    PasswordInvalidCharacters = 32,
    PasswordSpecialCharacterEmpty = 64,
    PasswordNumberEmpty = 128,
    PasswordCapitalLetterEmpty = 256,
    PasswordLowercaseLetterEmpty = 512,
    DatabaseError = 1024,
};


class Registration
{
    public:

    // Registering the user
        uint32_t RegisterUser(std::string username, std::string password)
        {
            RegistrationCodes usernameVerificationCode {ValidateUsername(username)};
            RegistrationCodes passwordVerificationCode {ValidatePassword(password)};

            if ((usernameVerificationCode != RegistrationCodes::ValidInput) | (passwordVerificationCode != RegistrationCodes::ValidInput))
            {
                return u_int32_t(usernameVerificationCode) | uint32_t(passwordVerificationCode);
            }

            try
            {
                // setting up mongodb instance
                mongocxx::instance instance{};

                // Database credentials
                std::string dbUsername{ ReadFromEnvFile("username") };
                std::string dbPassword{ ReadFromEnvFile("password") };
                std::string dbCluster{ ReadFromEnvFile("cluster") };

                std::string uriString{ std::string( "mongodb+srv://" ) + dbUsername + std::string(":") + dbPassword + std::string( "@" ) + dbCluster + std::string( ".2ev07.mongodb.net/?retryWrites=true&w=majority") };
                
                mongocxx::client client{ mongocxx::uri{ uriString.c_str()} } ;

                mongocxx::database db = client[dbCluster];

                return u_int32_t(RegistrationCodes::ValidInput);
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
            
            return RegistrationCodes::ValidInput;
        }

    private:

        // Checking if the password is valid
        RegistrationCodes ValidatePassword(std::string password)
        {
            int passwordLen = password.length();

            // Checking password size
            if (passwordLen > 30)
            {
                return RegistrationCodes::PasswordLong;
            }
            
            if (passwordLen <= 8)
            {
                return RegistrationCodes::PasswordShort;
            }

            // checking if it contains valid characters
            std::regex validPwdChars(R"(^[A-Za-z0-9~!@#$%^&*\(\)\_\-\+=\{\[\}\]\|\:;"'<,>\?]+$)"); 
            
            if (!std::regex_match(password, validPwdChars))
            {
                return RegistrationCodes::PasswordInvalidCharacters;
            }

            // Checking if it contains at least one special character
            std::regex specialCharacterCheck(R"(^[~!@#$%^&*\(\)\_\-\+=\{\[\}\]\|\:;"'<,>\?]+$)");

            if (!std::regex_match(password, specialCharacterCheck))
            {
                return RegistrationCodes::PasswordSpecialCharacterEmpty;
            }

            // Checking if it has at least one capital letter
            std::regex capitalLetterCheck(R"(^[A-Z]+$)");

            if (!std::regex_match(password, capitalLetterCheck))
            {
                return RegistrationCodes::PasswordCapitalLetterEmpty;
            }

            // Checking if it has at least one lower case
            std::regex lowercaseLetterCheck(R"(^[a-z]+$)");

            if (!std::regex_match(password, lowercaseLetterCheck))
            {
                return RegistrationCodes::PasswordLowercaseLetterEmpty;
            }

            return RegistrationCodes::ValidInput;
            
        }
};


int main()
{
    Registration r;
    printf("%d", r.RegisterUser("Mr Boombastic", "Fat"));
}