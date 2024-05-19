#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <fstream>
#include <memory>
#include <string>
#include "../headers/json.hpp"

using boost::asio::ip::tcp;

nlohmann::json problem_data;
std::mutex problem_mutex;

void load_problem_data()
{
    std::ifstream f("data/problems.json");
    problem_data = nlohmann::json::parse(f);
    f.close();
}

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

void expand_task(std::shared_ptr<tcp::socket> socket, std::string task_number)
{
    try
    {
        nlohmann::json local_problem_data;
        problem_mutex.lock();
        local_problem_data = problem_data;
        problem_mutex.unlock();
        std::string responce = local_problem_data[task_number]["description"];
        send_to_client(socket, responce);
    }
    catch (std::exception& error)
    {
        send_to_client(socket, "task name incorrect, enter the number of the task in the format of 'XXX'");
    }
}

void help(std::shared_ptr<tcp::socket> socket)
{
    std::string help_text = 
    "list of server commands:\n"
    "help - get this list of commands\n"
    "tasklist - get list of tasks avaiable\n"
    "expand [task_number]\n"
    "send [task_number] [file_name]";
    send_to_client(socket, help_text);
}

void get_task_list(std::shared_ptr<tcp::socket> socket)
{
    nlohmann::json local_problem_data;
    problem_mutex.lock();
    local_problem_data = problem_data;
    problem_mutex.unlock();
    std::cout << "bebra" << '\n';
    std::string responce = "";
    for (int i = 1; i < local_problem_data.size()+1; i ++)
    {
        std::string task_number = "";
        for (int j = 0; j < 3-(std::to_string(i)).size(); j++)
        {
            task_number += "0";
        }
        task_number += std::to_string(i);
        responce += task_number + " : ";
        responce += local_problem_data[task_number]["title"];
        if (i != local_problem_data.size())
        {
            responce += "\n";
        }
    }
    send_to_client(socket, responce);
}

void process_client(std::shared_ptr<tcp::socket> socket)
{
    std::string query = get_from_client(socket);

    std::cout << query << '\n';

    if (query.substr(0, 4) == "help") {help(socket);}
    else if (query.substr(0, 8) == "tasklist") {get_task_list(socket);}
    else if (query.substr(0, 6) == "expand") {expand_task(socket, query.substr(7, query.size()-7));}
    else {send_to_client(socket, "type 'help' for command list.");}
}

int main()
{
    load_problem_data();
    boost::asio::thread_pool pool(4);

    try 
    {
        boost::asio::io_context context;
        
        tcp::acceptor acceptor(context, tcp::endpoint(tcp::v4(), 8080));

        for(;;)
        {
            auto socket = std::make_shared<tcp::socket>(context);
            acceptor.accept(*socket);

            boost::asio::post(pool, std::bind(process_client, socket));
        }
        pool.join();
        
    }
    catch (std::exception& error)
    {
        std::cerr << error.what() << '\n';
    }
    
    return 0;
}
