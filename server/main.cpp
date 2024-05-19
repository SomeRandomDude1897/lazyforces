#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <memory>

using boost::asio::ip::tcp;



std::string get_from_client(std::shared_ptr<tcp::socket> socket)
{
    std::string message;
    size_t message_size;
    
    boost::asio::read(*socket, boost::asio::buffer(&message_size, sizeof(message_size)));
    message.resize(message_size);
    boost::system::error_code ignored_error;
    size_t len = socket->read_some(boost::asio::buffer(message), ignored_error);
    return message;
}

void send_to_client(std::shared_ptr<tcp::socket> socket, std::string message)
{
    size_t message_size = message.size();

    boost::asio::write(*socket, boost::asio::buffer(&message_size, sizeof(message_size)));

    boost::system::error_code error;
    boost::asio::write(*socket, boost::asio::buffer(&message[0], message_size));
}

int main()
{
    boost::asio::thread_pool pool(4);

    try 
    {
        boost::asio::io_context context;
        
        tcp::acceptor acceptor(context, tcp::endpoint(tcp::v4(), 8080));

        for(;;)
        {
            auto socket = std::make_shared<tcp::socket>(context);
            acceptor.accept(*socket);

            std::cout << get_from_client(socket) << '\n';

            boost::asio::post(pool, std::bind(send_to_client, socket, "amogusssss"));
        }
        pool.join();
        
    }
    catch (std::exception& error)
    {
        std::cerr << error.what() << '\n';
    }
    
    return 0;
}
