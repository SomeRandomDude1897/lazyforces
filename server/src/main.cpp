#include "../headers/server.h"

int main() {
  auto my_server = std::make_unique<server>(server());
  int status = my_server->run();
  delete (&my_server);
  return status;
}