#include "../headers/client.h"

void client::send_to_server(std::shared_ptr<tcp::socket> socket,
                            std::string message) {
  size_t message_size = message.size();

  boost::asio::write(*socket,
                     boost::asio::buffer(&message_size, sizeof(message_size)));

  boost::system::error_code error;
  boost::asio::write(*socket, boost::asio::buffer(&message[0], message_size));
}

bool client::send_solution(std::shared_ptr<tcp::socket> socket,
                           std::string file_name) {
  std::ifstream f(file_name);
  std::string file_content;
  if (f.good()) {
    while (!f.eof()) {
      file_content += f.get();
    }
  }
  f.close();
  send_to_server(socket, file_content.substr(0, file_content.size() - 1));
  if (file_content.substr(0, file_content.size() - 1).size() == 0) {
    std::cout << "No such file or directory\n";
    return false;
  };
  return true;
}

std::string client::get_from_server(std::shared_ptr<tcp::socket> socket) {
  std::string message;
  size_t message_size;
  boost::asio::read(*socket,
                    boost::asio::buffer(&message_size, sizeof(message_size)));
  message.resize(message_size);
  boost::system::error_code ignored_error;
  size_t len = socket->read_some(boost::asio::buffer(message), ignored_error);
  return message;
}

std::string client::get_responce(std::shared_ptr<tcp::socket> socket,
                                 std::string responce) {
  for (std::map<std::string,
                std::function<std::string(std::shared_ptr<tcp::socket>,
                                          std::string)>>::iterator x =
           responce_triggers.begin();
       x != responce_triggers.end(); x++) {
    std::string substr = x->first;
    std::string cur_substr = "";
    for (int i = 0; i < responce.size(); i++) {
      cur_substr += responce[i];
      if (cur_substr.size() > substr.size()) {
        cur_substr = substr.substr(1, cur_substr.size() - 1);
      }
      if (cur_substr == substr) {
        return x->second(socket,
                         responce.substr(i + 2, responce.size() - (i + 2)));
      }
    }
  }
  return responce;
}

std::string client::send_solution_handler(std::shared_ptr<tcp::socket> socket,
                                          std::string file_name) {
  for (int i = 0; i < file_name.size(); i++) {
    if (file_name[i] == ' ') {
      file_name = file_name.substr(i + 1, file_name.size() - i - 1);
    }
  }

  std::string local_responce = "";
  bool succesfully_sent = send_solution(socket, file_name);
  if (succesfully_sent) {
    local_responce = get_from_server(socket);
  } else {
    local_responce = "";
  }
  return local_responce;
}

void client::run(std::string ip) {
  while (true) {
    std::string query;
    std::getline(std::cin, query);
    try {
      boost::asio::io_context context;
      tcp::resolver resolver(context);
      tcp::resolver::results_type endpoints = resolver.resolve(ip, "8080");
      auto socket = std::make_shared<tcp::socket>(context);

      boost::asio::connect(*socket, endpoints);

      send_to_server(socket, query);

      std::string responce = get_from_server(socket);

      responce = get_responce(socket, responce);

      std::cout << responce << '\n';

      std::cout << "\n";
    } catch (std::exception& e) {
      std::cerr << e.what() << std::endl;
    }
  }
}
