#include <sys/stat.h>
#include <unistd.h>

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <fstream>
#include <iostream>
#include <string>

using boost::asio::ip::tcp;

class client {
private:
    void send_to_server(std::shared_ptr<tcp::socket>, std::string);
    bool send_solution(std::shared_ptr<tcp::socket>, std::string);
    std::string get_from_server(std::shared_ptr<tcp::socket>);
public:
    void run(std::string);
};