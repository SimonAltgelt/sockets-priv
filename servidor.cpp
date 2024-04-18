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

class Server {
 private:
  // metodos
  void socketCreate() { handle = socket(AF_INET, SOCK_STREAM, 0); }

  void socketConnect(int handle, const char *ip, int port) {
    clientAddress.sin_family = AF_INET;
    clientAddress.sin_port = htons(port);
    clientAddress.sin_addr.s_addr = inet_addr(ip);
    bind(handle, (struct sockaddr *)&clientAddress, sizeof(clientAddress));
  }

  void socketListen() { listen(handle, 0); }

  int socketRead(int handle, char *data, int size, int maxTimeout) {
    return recv(handle, data, size, 0);
  }

  int socketWrite(int handle, const char *data) {
    return send(handle, data, strlen(data), 0);
  }

  void socketClose(int handle) { close(handle); }

 public:
  int handle;
  int client;
  sockaddr_in clientAddress;
  char buffer[4024] = {0};
  string strBuffer;
  socklen_t clientAddrSize = sizeof(clientAddress);

  Server() {
    // crea y conecta el socket a una ip y puerto
    socketCreate();
    socketConnect(handle, "127.0.0.1", 8080);
    socketListen();
    cout << "Esperando a conexiones..." << endl;
  }

  ~Server() { socketClose(handle); }

  void Recibir() {
    if ((client = accept(handle, (struct sockaddr *)&clientAddress,
                         &clientAddrSize)) != SOCKET_ERROR) {
      cout << "Cliente conectado con exito." << endl;
    }
    int bytes_read = socketRead(client, buffer, sizeof(buffer), 5);
    if (bytes_read > 0) {
      cout << "El cliente dice: " << buffer << endl;
    }
    strBuffer = buffer;
    // resetea el arreglo buffer a 0.
    memset(buffer, 0, sizeof(buffer));
  }

  void Enviar(const char *mensaje) { socketWrite(client, mensaje); }

  void CerrarSocket() {
    socketClose(client);
    cout << "Cliente desconectado, socket cerrado." << endl;
  }
};

string armarMensajeResponse(Server *server) {
  string mensaje;
  string codigoRespuestaAprovado = "00";
  string aprovado = " \n\t\tAPROBADA";
  string codigoRespuestaDesaprovado = "01";
  string rechazada = "\n\t\tRECHAZADA";
  // LongitudTarjeta visualmente quedaria mejor pero por algun motivo tira error
  // al ejecutarse. int longitudTarjeta = stoi(server->strBuffer.substr(4, 2));

  if (server->strBuffer.substr(0, 4) == "0200" &&
      stoi(server->strBuffer.substr(4, 2)) >= 13) {
    if (stoi(server->strBuffer.substr(4, 2)) ==
        server->strBuffer.substr(6, stoi(server->strBuffer.substr(4, 2)))
            .length()) {
      if (server->strBuffer.substr(stoi(server->strBuffer.substr(4, 2)) + 6, 12)
              .length() == 12) {
        if (server->strBuffer
                .substr(stoi(server->strBuffer.substr(4, 2)) + 18, 3)
                .length() == 3) {
          mensaje = TIPO_MENSAJE + codigoRespuestaAprovado + aprovado;
        }
      }
    }
  } else {
    mensaje = TIPO_MENSAJE + codigoRespuestaDesaprovado + rechazada;
  }
  return mensaje;
}

int main() {
  Server *servidor = new Server();

    servidor->Recibir();
    servidor->Enviar(armarMensajeResponse(servidor).c_str());
  
  delete servidor;
  return 0;
}