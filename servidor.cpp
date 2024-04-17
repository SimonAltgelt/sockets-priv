#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#define SOCKET_ERROR -1
#define TIPO_MENSAJE "0210"

using namespace std;

typedef struct datos_tarjeta {
  string numeroTarjeta;
  string codigoSeguridad;
  string monto;
} datos_tarjeta_t;

typedef struct range {
  char rangeLow[8 + 1];
  char rangeHigh[8 + 1];
  unsigned char len;
  int id;
} range_t;

typedef struct card {
  char label[12 + 1];
  int id;
} card_t;

// string armarMensajeResponse(){
//   // si el codigo de respuesta es '00' indica que la transaccion fue aprobada y debera mostrar "APROBADA" en pantalla.
//   // si el codigo de respuesta es cualquier otro valor, debera mostrar "RECHAZADA".

// }

class Server {
 private:
  void socketCreate() { handle = socket(AF_INET, SOCK_STREAM, 0); }

  void socketConnect(int handle, const char *ip, int port) {
    clientAddress.sin_family = AF_INET;
    clientAddress.sin_port = htons(port);
    clientAddress.sin_addr.s_addr = inet_addr(ip);
    bind(handle, (struct sockaddr *)&clientAddress, sizeof(clientAddress));
  }

  void socketListen() { listen(handle, 0); }

  int socketRead(int handle, char *data, int size, int maxTimeout) { return recv(handle, data, size, 0); }

  int socketWrite(int handle, const char *data) { return send(handle, data, strlen(data), 0); }

  void socketClose(int handle) { close(handle); }

 public:
  int handle;
  int client;
  sockaddr_in clientAddress;
  char buffer[1024];

  Server() {
    socketCreate();
    socketConnect(handle, "127.0.0.1", 8080);
    socketListen();
    cout << "Esperando a conexiones..." << endl;
    socklen_t clientAddrSize = sizeof(clientAddress);
    if ((client = accept(handle, (struct sockaddr *)&clientAddress, &clientAddrSize)) != SOCKET_ERROR) {
      cout << "Cliente conectado con exito." << endl;
    }
  }

  ~Server() { socketClose(handle); }

  void Recibir() {
    char buffer[4024] = {0};
    int bytes_read = socketRead(client, buffer, sizeof(buffer), 0);
    if (bytes_read > 0) {
      cout << "El cliente dice: " << buffer << endl;
    } 
    memset(buffer, 0, sizeof(buffer));
  }
  void Enviar(const char *mensaje) { socketWrite(client, mensaje); }

  void CerrarSocket() {
    socketClose(client);
    cout << "Cliente desconectado, socket cerrado." << endl;
  }
};

int main() {
  Server *servidor = new Server();
  while (true) {
    servidor->Enviar("\x1b[32mTe respondo desde el server \x1b[0m");
    servidor->Recibir();
  }
  delete servidor;
  return 0;
}
