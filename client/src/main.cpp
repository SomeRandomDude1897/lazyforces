#include "../headers/client.h"

int main(int argc, char* argv[])
{
    if (argc != 2) {
    throw std::logic_error("incorrect terminal input data");
    return -1;
  }
  client* my_client = new client();
  my_client->run(static_cast<std::string>(argv[1]));
  delete(my_client);
  return 1;
}