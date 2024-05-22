#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <thread>
#include <memory>
#include <string>
#include "../headers/json.hpp"

using boost::asio::ip::tcp;

class server {
 private:
  nlohmann::json problem_data;
  void load_problem_data();
  std::string get_from_client(std::shared_ptr<tcp::socket>);
  void expand_task(std::shared_ptr<tcp::socket>, std::string);
  void help(std::shared_ptr<tcp::socket>);
  void get_task_list(std::shared_ptr<tcp::socket>);
  void test_code(std::shared_ptr<tcp::socket>, std::string, std::string);
  std::string get_code_from_client(std::shared_ptr<tcp::socket>, std::string);
  void process_client(std::shared_ptr<tcp::socket>);

 public:
  int run();
};