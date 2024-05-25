#include "../headers/client.h"

void client::send_to_server(std::shared_ptr<tcp::socket> socket, std::string message) {
  size_t message_size = message.size();

  boost::asio::write(*socket,
                     boost::asio::buffer(&message_size, sizeof(message_size)));

  boost::system::error_code error;
  boost::asio::write(*socket, boost::asio::buffer(&message[0], message_size));
}

bool client::send_solution(std::shared_ptr<tcp::socket> socket, std::string file_name) {
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
  return false;};
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
      // Вообще расклад с взятием подстроки вот таким образом выглядит как шаткая
      // конструкция. Если вдруг захочется изменить формат сообщения, то это 
      // надо будет везде сильно менять. Стоит подумать над каким-то еще методом
      // типа аргументов между разделителями (читать между разделителями, которые
      // ищешь в строке) или еще что-то поумнее
      if (responce.substr(0, 17) == "awaiting solution") {
        bool succesfully_sent =
            send_solution(socket, responce.substr(22, responce.size() - 22));
        if (succesfully_sent) {
          responce = get_from_server(socket);
        } else {
          responce = "";
        }
      }
      std::cout << responce << '\n';

      std::cout << "\n";
    } catch (std::exception& e) {
      std::cerr << e.what() << std::endl;
    }
  }
}
