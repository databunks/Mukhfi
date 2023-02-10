#include <iostream>

#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

// Large buffer for reading in data
std::vector<char> vBuffer(20 * 1024);

void GrabSomeData(asio::ip::tcp::socket& socket)
{
    socket.async_read_some(asio::buffer(vBuffer.data(), vBuffer.size()),
    [&](std::error_code ec, std::size_t length)
    {
        if (!ec)
        {
            std::cout << "\n\nRead " << length << " bytes\n\n";

            for (int i = 0; i < length; i++)
            {
                std::cout << vBuffer[i];
            }

            // Grabs more and more data until there is none left
            GrabSomeData(socket);
        }
    });
}

int main()
{
    asio::error_code ec; // gives out any error code

    // Create a context which is the platform specific interface (creates a space where asio can do its work    )
    asio::io_context context; // context needs to be primed with jobs for asio to do, if it doesent have anything to do it will exit immediatly

    // Give some fake tasks to asio so the context does not finish
    asio::io_context::work idleWork(context);

    // Run context in its own thread so it can execute its instructions without blocking the main program
    std::thread thrContext = std::thread([&]() { context.run(); }); // the run function returns as soon as the context has run out of things to do

    // Get address of somewhere you wanna connect to
    asio::ip::tcp::endpoint endpoint(asio::ip::make_address("51.38.81.49", ec), 80);

    // Create a socket, context delivers implementation
    asio::ip::tcp::socket socket(context);

    // try to get socket to connect 
    socket.connect(endpoint, ec);

    // The error code determines if its successful or not
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
        // prime asio context to read data in before you write something to it
        GrabSomeData(socket);

        std::string sRequest =
            "GET /index.html HTTP/1.1\r\n"
            "Host: example.com\r\n"
            "Connection: close\r\n\r\n";

        // Reading and writing data with asio is done through buffers
        socket.write_some(asio::buffer(sRequest.data(), sRequest.size()), ec);

        //GrabSomeData(socket); - Wrong, it may send data when there is no bytes to read

        //sleep(2000ms);
        
    }

    return 0;
}

