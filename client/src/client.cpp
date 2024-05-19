#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

void send_to_server(std::shared_ptr<tcp::socket> socket, std::string message)
{
    size_t message_size = message.size();

    boost::asio::write(*socket, boost::asio::buffer(&message_size, sizeof(message_size)));

    boost::system::error_code error;
    boost::asio::write(*socket, boost::asio::buffer(&message[0], message_size));
}

std::string get_from_server(std::shared_ptr<tcp::socket> socket)
{
    std::string message;
    size_t message_size;
    
    boost::asio::read(*socket, boost::asio::buffer(&message_size, sizeof(message_size)));
    message.resize(message_size);
    boost::system::error_code ignored_error;
    size_t len = socket->read_some(boost::asio::buffer(message), ignored_error);
    return message;
}

int main()
{
    while (true)
    {
        std::string query;
        std::getline(std::cin, query);
        try
        {
            boost::asio::io_context context;
            tcp::resolver resolver(context);
            tcp::resolver::results_type endpoints =
            resolver.resolve("127.0.0.1", "8080");
            auto socket = std::make_shared<tcp::socket>(context);
            
            boost::asio::connect(*socket, endpoints);

            send_to_server(socket, query);

            std::cout << get_from_server(socket) << '\n';

            std::cout << "\n";
        }
        catch (std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
    }
}