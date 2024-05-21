#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <chrono>
#include <fstream>
#include <memory>
#include <string>
#include <map>
#include "../headers/json.hpp"

using boost::asio::ip::tcp;

nlohmann::json problem_data;
std::mutex problem_mutex;
std::mutex cout_mutex;

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
    "expand [task_number] - show description of task [task number]\n"
    "send [task_number] [file_directory] - send code for task from the specified directory";
    send_to_client(socket, help_text);
}

void get_task_list(std::shared_ptr<tcp::socket> socket)
{
    nlohmann::json local_problem_data;
    problem_mutex.lock();
    local_problem_data = problem_data;
    problem_mutex.unlock();
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

void clear_directory(std::string ip)
{
    system(("rm testing/input" + ip + ".txt").c_str());   
    system(("rm testing/output" + ip + ".txt").c_str());  
    system(("rm testing/solution" + ip + ".out").c_str()); 
}

void test_code(std::shared_ptr<tcp::socket> socket, std::string source_code, std::string task)
{
    nlohmann::json local_problem_data;
    problem_mutex.lock();
    local_problem_data = problem_data;
    problem_mutex.unlock();
    if (source_code != "")
    {
        int tests_passed = 0;
        bool flag = false;
        cout_mutex.lock();
        cout_mutex.unlock();

        for (auto& x : local_problem_data.items())
        {
            if (task == static_cast<std::string>(x.key()))
            {
                flag = true;
                break;
            }
        }
        if (!flag)
        {
            send_to_client(socket, "No such task found");
            return;
        }
        std::ofstream f;
        std::string solution_file_name = "testing/solution" + socket->remote_endpoint().address().to_string();
        f.open(solution_file_name + ".cpp");
        f << source_code;
        f.close();
        try
        {
            system(("g++ " + solution_file_name + ".cpp -o " + solution_file_name + ".out").c_str());
            system(("rm " + solution_file_name + ".cpp").c_str());
        }
        catch (std::exception& error)
        {
            send_to_client(socket, "Compilation Error");
            system(("rm " + solution_file_name + ".cpp").c_str());
            return;
        }

        for (int i = 0; i < local_problem_data[task]["tests"].size(); i++)
        {
            std::ofstream inp_f;
            std::string input_file_name = "testing/input" + socket->remote_endpoint().address().to_string() + ".txt";
            std::string output_file_name = "testing/output" + socket->remote_endpoint().address().to_string() + ".txt";
            inp_f.open(input_file_name);
            inp_f << static_cast<std::string>(local_problem_data[task]["tests"][i]["query"]);
            inp_f.close();

            auto start = std::chrono::high_resolution_clock::now();
            std::string command = "firejail --rlimit-nproc=1 --rlimit-cpu=" + static_cast<std::string>(local_problem_data[task]["time limit"]) +  
            " ./" + solution_file_name + ".out < " + input_file_name + " > " + output_file_name;
            system(command.c_str());
            double execution_time = (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start)).count()/1000.0;
            if (execution_time > stod(static_cast<std::string>(local_problem_data[task]["time limit"])))
            {
                send_to_client(socket, "Time limit exceeded on test " + std::to_string(tests_passed + 1));
                clear_directory(socket->remote_endpoint().address().to_string());
                return;
            }
            std::string answer;
            std::ifstream ans_f;
            ans_f.open(output_file_name);
            while (!ans_f.eof())
            {
                answer += ans_f.get();
            }
            answer = answer.substr(0, answer.size()-1);
            if (answer != static_cast<std::string>(local_problem_data[task]["tests"][i]["answer"]) && answer != static_cast<std::string>(local_problem_data[task]["tests"][i]["answer"]) + "\n")
            {
                send_to_client(socket, "Wrong answer on test " + std::to_string(tests_passed + 1));
                clear_directory(socket->remote_endpoint().address().to_string());
                return;
            }
            tests_passed++;
        }
        send_to_client(socket, "Accepted");
        clear_directory(socket->remote_endpoint().address().to_string());
    }
}

std::string get_code_from_client(std::shared_ptr<tcp::socket> socket, std::string file_name)
{
    send_to_client(socket, "awaiting solution " + file_name);
    std::string responce = get_from_client(socket);
    if (responce == "no such file or directory")
    {
        return "";
    }
    else
    {
        return responce;
    }
}

void process_client(std::shared_ptr<tcp::socket> socket)
{
    std::string query = get_from_client(socket);

    if (query.substr(0, 4) == "help") {help(socket);}
    else if (query.substr(0, 8) == "tasklist") {get_task_list(socket);}
    else if (query.substr(0, 6) == "expand") {expand_task(socket, query.substr(7, query.size()-7));}
    else if (query.size() > 8 && query.substr(0, 4) == "send") {test_code(socket, get_code_from_client(socket, query.substr(5, query.size()-5)), query.substr(5, 3));}
    else {send_to_client(socket, "type 'help' for command list.");}
}

int main()
{
    load_problem_data();
    boost::asio::thread_pool pool(std::thread::hardware_concurrency());

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
