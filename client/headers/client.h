#include <sys/stat.h>
#include <unistd.h>

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

using boost::asio::ip::tcp;

class client {
 private:
  void send_to_server(std::shared_ptr<tcp::socket>, std::string);
  bool send_solution(std::shared_ptr<tcp::socket>, std::string);
  std::string send_solution_handler(std::shared_ptr<tcp::socket>, std::string);
  std::string get_responce(std::shared_ptr<tcp::socket>, std::string);
  std::string get_from_server(std::shared_ptr<tcp::socket>);
  std::map<std::string, std::function<std::string(std::shared_ptr<tcp::socket>,
                                                  std::string)>>
      responce_triggers = {{"awaiting solution",
                            [this](std::shared_ptr<tcp::socket> socket,
                                   std::string responce) -> std::string {
                              return send_solution_handler(socket, responce);
                            }}};

 public:
  void run(std::string);
};