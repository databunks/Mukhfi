#include <iostream>

#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

int main()
{
    asio::error_code ec; // gives out any error code

    // Create a context which is the platform specific interface
    asio::io_context context; // instance of asio

    // Get address of somewhere you wanna connect to
    asio::ip::tcp::endpoint endpoint(asio::ip::make_address("2a00:1450:400b:c02::88", ec), 80);

    // Create a socket
    asio::ip::tcp::socket socket(context);

    // try to get socket to connect
    socket.connect(endpoint, ec);

    if (!ec)
    {
        std::cout << "Connected :-)" << std::endl;
    }
    else
    {
        std::cout << "Failed to connect to address:\n"
                  << ec.message() << std::endl;
    }

    if (socket.is_open())
    {
        std::string sRequest =
            "GET /index.html HTTP/1.1\r\n"
            "Host: example.com\r\n"
            "Connection: close\r\n\r\n";

        // Reading and writing data with asio is done through buffers
        socket.write_some(asio::buffer(sRequest.data(), sRequest.size()), ec);

        
        size_t bytes = socket.available();
        std::cout << "Bytes availiable: " << bytes << std::endl;

        int secondsToWait = 1;
        int retryAttempts = 5;
        int i = 0;

        while (bytes == 0 && i != retryAttempts)
        {
            std::cout << "No bytes received waiting for " << secondsToWait << "s" << std::endl;
            sleep(secondsToWait);
            bytes = socket.available();
            i++;
        }

        if (bytes <= 0)
        {
            std::cout << "No bytes received" << std::endl;
        }
        else
        {
            std::vector<char> vBuffer(bytes);
            socket.read_some(asio::buffer(vBuffer.data(), vBuffer.size()), ec);

            for (auto c : vBuffer)
            {
                std::cout << c;
            }
        }
        
    }

    return 0;
}

