#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <regex>

#define SOCKET_ERROR -1

using namespace std;

typedef struct datos_tarjeta{
  int numeroTarjeta;
  int codigoSeguridad;
  float monto;
} datos_tarjeta_t;

typedef struct range{
  char rangeLow[8 + 1];
  char rangeHigh[8 + 1];
  unsigned char len;
  int id;
} range_t;

bool validarTarjeta(string numeroTarjeta) {
    // Lógica para validar el número de tarjeta, ver "Reconocimiento Tarjeta"
    return true;
}   

bool validarMonto(const string &monto) {
    // Expresión regular que busca un número con dos decimales
    regex formato("^\\d+(\\.\\d{1,2})?$");
    // ^\d+: Empieza con uno o más dígitos.
    // (\.\d{1,2})?: Puede o no tener un punto seguido de uno o dos dígitos.
    // $: Termina aquí.
    // Verifica si el monto coincide con la expresión regular
    return regex_match(monto, formato);
}


datos_tarjeta_t *SolicitarDatosTarjeta() {
    string numeroTarjeta;
    string monto;
    string codigoSeguridad;

    bool montoValido = false;
    while(!montoValido){
    cout << "Ingrese el monto de la compra: ";
    cin >> monto;  // validar que sea un numero y que tenga 2 decimales, un while igual que el de abajo // comprobar q funque
    if(!validarMonto(monto)){
        montoValido = false;
    } else{
        montoValido = true;
    }

    }
    bool numeroTarjetaValido = false;
    while (!numeroTarjetaValido) {
    cout << "Ingrese el numero de tarjeta: ";
    cin >> numeroTarjeta;
    if (numeroTarjeta.length() < 13) {
        cout << "El numero de tarjeta debe tener al menos 13 digitos. Intente de nuevo." << endl;
    } else {
        numeroTarjetaValido = true;
    }
    }
    if (!validarTarjeta(numeroTarjeta)) {
    cout << "TARJETA NO SOPORTADA" << endl;
    return NULL;
    };
    
    bool codigoSeguridadValido = false;
    while (!codigoSeguridadValido) {
    cout << "Ingrese codigo de seguridad: ";
    cin >> codigoSeguridad;;
    if (codigoSeguridad.length() >= 3) {
        cout << "El codigo de seguridad tiene que tener 3 digitos. Intente de nuevo." << endl;
    } else{
        codigoSeguridadValido = true;
    }
    }

    datos_tarjeta_t *datos_pointer = new datos_tarjeta_t;
    datos_pointer->numeroTarjeta = stoi(numeroTarjeta);
    datos_pointer->codigoSeguridad = stoi(codigoSeguridad);
    datos_pointer->monto = stof(monto);

    return datos_pointer;
    
}

string ArmarMensajeRequest(datos_tarjeta_t *datos) {
    // ver formato mensaje
    string mensaje =
        to_string(datos->numeroTarjeta) + "," + to_string(datos->codigoSeguridad) + "," + to_string(datos->monto);
    return mensaje;
}

class Client {
 public:
    int clientSocket;
    char buffer[1024];
    sockaddr_in serverAddress;

 Client(){
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    int result = connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (result == SOCKET_ERROR) {
        cout << "Error al conectar!\n Error " << errno << endl;
    } else{
        cout << "Conectado al servidor! (c)" << endl;
    }
  }

  void Enviar(){
    datos_tarjeta_t *datos = SolicitarDatosTarjeta();
    if (datos == NULL) {
      return;
    }
    string mensaje = ArmarMensajeRequest(datos);
    send(clientSocket, &mensaje, strlen(buffer), 0);
    cout << "Mensaje enviado! (c)" << endl;
    memset(buffer, 0, sizeof(buffer));
    delete datos;
  }

  void Recibir(){
    recv(clientSocket, buffer, sizeof(buffer), 0);
    cout << "El servidor dice " << "'" << buffer << "'" << endl;
    // resetea el arreglo buffer a 0.
    memset(buffer, 0, sizeof(buffer));
  }

  void CerrarSocket(){
    close(clientSocket);
    cout << "Cliente desconectado, socket cerrado." << endl;
  }

};

int main() {
    Client *cliente = new Client();
    while (true){
        cliente->Enviar();
        cliente->Recibir();
    }

    delete cliente;

return 0;
}