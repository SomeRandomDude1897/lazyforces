#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <thread>

#include "../headers/json.hpp"

using boost::asio::ip::tcp;

class server {
 private:
  void load_problem_data();
  std::string get_from_client(std::shared_ptr<tcp::socket>);
  void expand_task(std::shared_ptr<tcp::socket>, std::string);
  void help(std::shared_ptr<tcp::socket>);
  void get_task_list(std::shared_ptr<tcp::socket>);
  void handle_send(std::shared_ptr<tcp::socket>, std::string);
  void test_code(std::shared_ptr<tcp::socket>, std::string, std::string);
  std::string get_code_from_client(std::shared_ptr<tcp::socket>, std::string);
  void process_client(std::shared_ptr<tcp::socket>);

  std::map<std::string,
           std::function<void(std::shared_ptr<tcp::socket>, std::string)>>
      commands = {
          {"tasklist",
           [this](std::shared_ptr<tcp::socket> socket, std::string query) {
             this->get_task_list(socket);
           }},
          {"expand",
           [this](std::shared_ptr<tcp::socket> socket, std::string query) {
             this->expand_task(socket, query.substr(7, query.size() - 7));
           }},
          {"help", [this](std::shared_ptr<tcp::socket> socket,
                          std::string query) { this->help(socket); }},
          {"send",
           [this](std::shared_ptr<tcp::socket> socket, std::string query) {
             this->handle_send(socket, query);
           }}};

 public:
  int run();
};