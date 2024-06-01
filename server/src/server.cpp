#include "../headers/server.h"

std::string server::get_from_client(std::shared_ptr<tcp::socket> socket) {
  std::string message;
  size_t message_size;

  boost::asio::read(*socket,
                    boost::asio::buffer(&message_size, sizeof(message_size)));
  message.resize(message_size);
  boost::system::error_code errorcode;
  size_t len = socket->read_some(boost::asio::buffer(message), errorcode);
  if (errorcode == boost::system::errc::success) {
    return message;
  } else {
    return "";
  }
}

void send_to_client(std::shared_ptr<tcp::socket> socket, std::string message) {
  size_t message_size = message.size();

  boost::asio::write(*socket,
                     boost::asio::buffer(&message_size, sizeof(message_size)));

  boost::system::error_code errorcode;
  boost::asio::write(*socket, boost::asio::buffer(&message[0], message_size),
                     errorcode);
  if (errorcode != boost::system::errc::success) {
    std::mutex cout_mutex;
    std::lock_guard<std::mutex> lockGuard(cout_mutex);
    std::cout << "error while sending data to client \nerrorcode: " << errorcode
              << '\n message : ' << message << '\n';
  }
}

void server::expand_task(std::shared_ptr<tcp::socket> socket,
                         std::string task_number) {
  try {
    std::mutex problem_mutex;
    std::lock_guard<std::mutex> lockGuard(problem_mutex);
    std::ifstream f("data/problems.json");
    nlohmann::json problem_data = nlohmann::json::parse(f);
    f.close();
    std::string responce = problem_data[task_number]["description"];
    send_to_client(socket, responce);
  } catch (std::exception& error) {
    send_to_client(socket,
                   "task name incorrect, enter the number of the task in the "
                   "format of 'XXX..X'");
  }
}

void server::help(std::shared_ptr<tcp::socket> socket) {
  std::string help_text =
      "list of server commands:\n"
      "help - get this list of commands\n"
      "tasklist - get list of tasks avaiable\n"
      "expand [task_number] - show description of task [task number]\n"
      "send [task_number] [file_directory] - send code for task from the "
      "specified directory";
  send_to_client(socket, help_text);
}

void server::get_task_list(std::shared_ptr<tcp::socket> socket) {
  std::mutex problem_mutex;
  std::lock_guard<std::mutex> lockGuard(problem_mutex);
  std::ifstream f("data/problems.json");
  nlohmann::json problem_data = nlohmann::json::parse(f);
  f.close();
  std::string responce = "";
  for (int i = 1; i < problem_data.size() + 1; i++) {
    std::string task_number = "";
    task_number = std::to_string(i);
    responce += task_number + " : ";
    responce += problem_data[task_number]["title"];
    if (i != problem_data.size()) {
      responce += "\n";
    }
  }
  send_to_client(socket, responce);
}
void clear_directory(std::string ip) {
  system(("rm testing/input" + ip + ".txt").c_str());
  system(("rm testing/output" + ip + ".txt").c_str());
  system(("rm testing/solution" + ip + ".out").c_str());
}

bool check_task_exists(std::string task, nlohmann::json* problem_data) {
  bool flag = false;

  for (auto& x : problem_data->items()) {
    if (task == x.key()) {
      flag = true;
      return true;
    }
  }
  return false;
}

bool check_answer(std::string output_file_name, nlohmann::json* problem_data,
                  std::string task, int test_number) {
  std::string answer;
  std::ifstream ans_f;
  ans_f.open(output_file_name);
  while (!ans_f.eof()) {
    answer += ans_f.get();
  }
  answer = answer.substr(0, answer.size() - 1);
  if (answer != static_cast<std::string>(problem_data->operator[](
                    task)["tests"][test_number]["answer"]) &&
      answer != static_cast<std::string>(problem_data->operator[](
                    task)["tests"][test_number]["answer"]) +
                    "\n") {
    return false;
  }
  return true;
}

bool compile(std::string source_code, std::string file_name) {
  if (source_code.size() == 0) {
    return false;
  }

  std::ofstream f;

  f.open(file_name + ".cpp");
  f << source_code;
  f.close();
  system(("g++ " + file_name + ".cpp -o " + file_name + ".out").c_str());
  system(("rm " + file_name + ".cpp").c_str());
  std::ifstream check_created_stream((file_name + ".out").c_str());
  if (!check_created_stream.good()) {
    return false;
  }
  return true;
}

double execute(std::string input_file_name, std::string output_file_name,
               std::string task, nlohmann::json* problem_data, int test,
               std::string solution_file_name) {
  std::ofstream inp_f;

  inp_f.open(input_file_name);
  inp_f << static_cast<std::string>(
      problem_data->operator[](task)["tests"][test]["query"]);
  inp_f.close();

  auto start = std::chrono::high_resolution_clock::now();
  std::string command =
      "firejail --rlimit-nproc=1 --rlimit-cpu=" +
      static_cast<std::string>(problem_data->operator[](task)["time limit"]) +
      " ./" + solution_file_name + ".out < " + input_file_name + " > " +
      output_file_name;
  system(command.c_str());
  double execution_time =
      (std::chrono::duration_cast<std::chrono::milliseconds>(
           std::chrono::high_resolution_clock::now() - start))
          .count() /
      1000.0;
  return execution_time;
}

void server::test_code(std::shared_ptr<tcp::socket> socket,
                       std::string source_code, std::string task) {
  std::mutex problem_mutex;
  std::lock_guard<std::mutex> lockGuard(problem_mutex);
  std::ifstream f("data/problems.json");
  nlohmann::json problem_data = nlohmann::json::parse(f);
  f.close();

  std::cout << problem_data.size() << '\n';

  std::string client_ip = socket->remote_endpoint().address().to_string();

  if (!check_task_exists(task, &problem_data)) {
    send_to_client(socket, "No such task found");
    return;
  }

  std::string solution_file_name = "testing/solution" + client_ip;
  std::string input_file_name = "testing/input" + client_ip + ".txt";
  std::string output_file_name = "testing/output" + client_ip + ".txt";

  bool compilation_status = compile(source_code, solution_file_name);
  if (!compilation_status) {
    send_to_client(socket, "Compilation Error");
    return;
  }
  for (int i = 0; i < problem_data[task]["tests"].size(); i++) {
    double execution_time = execute(input_file_name, output_file_name, task,
                                    &problem_data, i, solution_file_name);

    if (execution_time >
        stod(static_cast<std::string>(problem_data[task]["time limit"]))) {
      send_to_client(socket,
                     "Time limit exceeded on test " + std::to_string(i));
      clear_directory(socket->remote_endpoint().address().to_string());
      return;
    }

    bool test_res = check_answer(output_file_name, &problem_data, task, i);
    if (!test_res) {
      send_to_client(socket, "Wrong answer on test " + std::to_string(i));
      clear_directory(socket->remote_endpoint().address().to_string());
      return;
    }
  }
  send_to_client(socket, "Accepted");
  clear_directory(socket->remote_endpoint().address().to_string());
}

std::string server::get_code_from_client(std::shared_ptr<tcp::socket> socket,
                                         std::string file_name) {
  send_to_client(socket, "awaiting solution " + file_name);

  std::string responce = get_from_client(socket);
  if (responce.size() == 0) {
    return "";
  } else {
    return responce;
  }
}

std::string get_task_number(std::string query) {
  std::string answer = "";
  bool flag = false;
  for (int i = 0; i < query.size(); i++) {
    if (flag == true) {
      answer += query[i];
    }
    if (query[i] == ' ') {
      if (flag) {
        return answer.substr(0, answer.size() - 1);
      }
      flag = true;
    }
  }
}

void server::handle_send(std::shared_ptr<tcp::socket> socket,
                         std::string query) {
  test_code(socket,
            get_code_from_client(socket, query.substr(5, query.size() - 5)),
            get_task_number(query));
}

void server::process_client(std::shared_ptr<tcp::socket> socket) {
  std::string query = get_from_client(socket);

  for (std::map<std::string, std::function<void(std::shared_ptr<tcp::socket>,
                                                std::string)>>::iterator x =
           commands.begin();
       x != commands.end(); x++) {
    std::string substr = x->first;
    std::string cur_substr = "";
    for (int i = 0; i < query.size(); i++) {
      cur_substr += query[i];
      if (cur_substr.size() > substr.size()) {
        cur_substr = substr.substr(1, cur_substr.size() - 1);
      }
      if (cur_substr == substr) {
        x->second(socket, query);
        return;
      }
    }
  }
  send_to_client(socket, "type 'help' for command list.");
  return;
}

int server::run() {
  boost::asio::thread_pool pool(std::thread::hardware_concurrency());

  try {
    boost::asio::io_context context;

    tcp::acceptor acceptor(context, tcp::endpoint(tcp::v4(), 8080));

    for (;;) {
      auto socket = std::make_shared<tcp::socket>(context);
      acceptor.accept(*socket);
      auto l = [this](std::shared_ptr<tcp::socket> socket) {
        process_client(socket);
      };
      boost::asio::post(pool, std::bind(l, socket));
    }
    pool.join();

  } catch (std::exception& error) {
    std::cerr << error.what() << '\n';
  }
  pool.join();
  return 0;
}