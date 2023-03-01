#include <cstdint>
#include <iostream>
#include <vector>
#include <string>
#include <cwctype>

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

// For identifying the process of registering
enum RegistrationCodes
{
    ValidInput,
    UsernameLong,
    UsernameShort,
    UsernameInvalidCharacters,
    PasswordLong,
    PasswordShort,
    PasswordInvalidCharacters,
};

RegistrationCodes ValidateUsername(std::string username)
{
    int usernameLen = username.length();

    if (usernameLen > 20)
    {
        return UsernameLong;
    }

    if (usernameLen <= 4)
    {
        return UsernameShort;
    }

    if (!iswalnum(username))
    {
        return UsernameInvalidCharacters;
    }

    return ValidateInput;
}

RegistrationCodes ValidatePassword(std::string password)
{

    int passwordLen = password.length();

    if (passwordLen > 30)
    {
        return PasswordLong;
    }
    
    if (passwordLen <= 8)
    {
        return PasswordShort;
    }
    
    char validPasswordChars[] = 
    {
    '!', '"', '#', '$', '&', '(', ')', '+', ',', '-', '.', '/', 
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', 
    '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', 
    ']', '^', '_', '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 
    'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 
    '{', '|', '}'
    };

    int validPasswordCharsLen = sizeof(validPasswordChars) / sizeof(validPasswordChars[0]);

    RegistrationCodes regCode = ValidInput;

    for (char c : password)
    {
        bool check = false;
        
        for (int i = 0; i < validPasswordCharsLen; i++))
        {
            if (c == validPasswordChars[i])
            {
                check = true;
                break;
            }

            if (i == validPasswordCharsLen - 1 && !check)
            {
                return PasswordInvalidCharacters;
            }
        } 
    }

    return regCode;
    
}

RegistrationCodes ValidateInput(std::string username, std::string password)
{
    RegistrationCodes regCode = ValidateUsername(username);

    if (regCode != ValidInput)
    {
        return regCode;
    }

    regCode = ValidatePassword(password);

    if (regCode != ValidInput)
    {
        return regCode;
    }

}

// Registering the user
RegistrationCodes RegisterUser(std::string username, std::string password)
{

    

    // int requiredNumberOfArguments = 4;

    // // if (argc < requiredNumberOfArguments)
    // // {
    // //     std::cerr << "Invalid number of arguments (only " << argc - 1 << " passed in, need " << requiredNumberOfArguments << ")" << std::endl;
    // //     return 1;
    // // }

    // std::string username { argv[1] };
    // std::string password { argv[2] }; 
    // std::string cluster { argv[3] } ;

    mongocxx::instance instance{};

    std::string uriString = std::string( "mongodb+srv://" ) + username + std::string(":") + password + std::string( "@" ) + cluster + std::string( ".xlmttbn.mongodb.net/?retryWrites=true&w=majority" );
    
    mongocxx::client client{ mongocxx::uri{ uriString.c_str()} } ;

    mongocxx::database db = client[cluster];

    // TODO: Setup Asio to receive registration details

    // TODO: Save And Verify Information To Save To Database

    return 0;
}
