#include "../headers/server.h"

int main() {
  server* my_server = new server();
  int status = my_server->run();
  delete (my_server);
  return status;
}