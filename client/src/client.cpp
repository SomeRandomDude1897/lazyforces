#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <string>

using boost::asio::ip::tcp;

void send_to_server(std::shared_ptr<tcp::socket> socket, std::string message)
{
    size_t message_size = message.size();

    boost::asio::write(*socket, boost::asio::buffer(&message_size, sizeof(message_size)));

    boost::system::error_code error;
    boost::asio::write(*socket, boost::asio::buffer(&message[0], message_size));
}

bool send_solution(std::shared_ptr<tcp::socket> socket, std::string file_name)
{
    try
    {
        std::ifstream f(file_name);
        std::string file_content;
        if (f.good())
        {
            while (!f.eof())
            {
                file_content += f.get();
            }
        }
        else
        {
            throw std::invalid_argument("no such file or directory");
            return false;
        }
        f.close();
        send_to_server(socket, file_content.substr(0, file_content.size()-1));
    }
    catch (std::exception& error)
    {
        std::cerr << error.what() << '\n';
        send_to_server(socket, "no such file or directory");
        return false;
    }
    return true;
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

            std::string responce = get_from_server(socket);

            if (responce.substr(0, 17) == "awaiting solution")
            {
                bool succesfully_sent = send_solution(socket, responce.substr(22, responce.size()-22));
                if (succesfully_sent)
                {
                    responce = get_from_server(socket);
                }
                else
                {
                    responce = "";
                }
            }
            std::cout << responce << '\n';

            std::cout << "\n";
        }
        catch (std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
    }
}