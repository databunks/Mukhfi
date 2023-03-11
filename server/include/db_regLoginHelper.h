#include <cstdint>
#include <iostream>
#include <vector>
#include <string>
#include <cwctype>
#include <regex>
#include <sstream>
#include <fstream>
#include <random>
#include <ctime>


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
enum class RegistrationLoginCodes : uint32_t
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
    UserDoesNotExist = 64,
    UserAlreadyExists = 128,
    PasswordNoMatchFound = 256,
    DatabaseError = 512,
};

class HelperFunctions
{
    public:
    // Generates a random 64 bit token
    std::string GenerateToken()
    {

        std::random_device r;
        std::mt19937_64 gen(r());
        int lenToken = 64;

        char token[lenToken];
        for (int i = 0; i < lenToken; i++)
        {
            token[i] = (char)(r() % (127 - 33) + 33);
        }
        return std::string(token);

    }

    // Checking if the username is valid
    RegistrationLoginCodes ValidateUsername(std::string username)
    {
        int usernameLen = username.length();

        // Checking if username length is too long
        if (usernameLen > 20)
        {
            return RegistrationLoginCodes::UsernameLong;
        }

        // Checking if username length is too short
        if (usernameLen <= 4)
        {
            return RegistrationLoginCodes::UsernameShort;
        }

        // Checking if username is alphanumeric
        std::regex alnum("^[A-Za-z0-9]+$"); 
        if (!std::regex_match(username,alnum))
        {
            return RegistrationLoginCodes::UsernameInvalidCharacters;
        }
        
        return RegistrationLoginCodes::Success;
    }


        // Password Strength should be validated at client side pre hash
        // Checking if the password is valid
        RegistrationLoginCodes ValidatePassword(std::string password)
        {
            int passwordLen = password.length();

            std::regex alnum("^[A-Za-z0-9]+$"); 

            // Checking password is sha256 hash
            if ( (passwordLen != 64) | (!std::regex_match(password, alnum)) )
            {
                std::cout << "Test";
                return RegistrationLoginCodes::PasswordNotHashed;
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

            return RegistrationLoginCodes::Success;
            
        }
};






