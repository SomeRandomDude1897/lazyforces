#include "../headers/server.h"

int main() {
  // Лучше уж make_unique тогда. Вдруг тут еще добавят какой-то код в будущем
  // все это дело разрастется, где-то между станут бросаться исключения, в итоге
  // ничего не удалим
  server* my_server = new server();
  int status = my_server->run();
  delete (my_server);
  return status;
}