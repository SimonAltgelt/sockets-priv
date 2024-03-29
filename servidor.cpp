#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#define SOCKET_ERROR -1

using namespace std;

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
    socketRead(client, buffer, sizeof(buffer), 0);
    cout << "El cliente dice: " << buffer << endl;
    // bool es_valida = validarTarjeta(buffer);
    // responderCliente(es_valida);
    memset(buffer, 0, sizeof(buffer));
  }

  void Enviar(const char *mensaje) { socketWrite(client, mensaje); }

  void CerrarSocket() {
    socketClose(client);
    cout << "Cliente desconectado, socket cerrado." << endl;
  }

  //   string SolicitarDatosTarjeta() {
  //     string numeroTarjeta;
  //     float monto;
  //     string codigoSeguridad;

  //     cout << "Ingrese el monto de la compra: ";
  //     cin >> monto;
  //     while (true) {
  //       cout << "Ingrese el numero de tarjeta: ";
  //       cin >> numeroTarjeta;
  //       if (numeroTarjeta.length() < 13) {
  //         cout << "El numero de tarjeta debe tener al menos 13 digitos. Intente de nuevo." << endl;
  //       } else {
  //         break;
  //       }
  //     }
  //     cout << "Ingrese el codigo de seguridad: ";
  //     cin >> codigoSeguridad;

  //     /// Lógica adicional para validar y procesar los datos de la tarjeta...

  //     stringstream ss;
  //     ss << fixed << setprecision(2) << monto;
  //     string strMonto = ss.str();
  //     string mensajeRespuesta =
  //         "NRO TARJETA:" + numeroTarjeta + ". MONTO: " + strMonto + ". COD. SEGURIDAD:" + codigoSeguridad;
  //     return mensajeRespuesta;
  //   }
};

int main() {
  Server *servidor = new Server();
  while (true) {
    // string mensajeParaEnviar = servidor->SolicitarDatosTarjeta();
    servidor->Enviar("\x1b[32mTe respondo desde el server \x1b[0m");
    servidor->Recibir();
  }
  delete servidor;
  return 0;
}
