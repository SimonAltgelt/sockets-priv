#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <regex>

#define SOCKET_ERROR -1
#define TIPO_MENSAJE "0200"
#define LONGITUD_CEROS 12

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

void leerDatRages() {
  FILE *fp;
  vector<range_t> rangeVector = vector<range_t>(10);

  fp = fopen("ranges.dat", "rb");
  if (fp == NULL) {
    cout << "Error al abrir el archivo" << endl;
    return;
  }

  range_t range;
  while (fread(&range, sizeof(range_t), 1, fp) == 1) {
    rangeVector.push_back(range);
  };

  fclose(fp);
}

void leerDatCards() {
  FILE *fp;
  vector<card_t> cardVector = vector<card_t>(10);

  fp = fopen("cards.dat", "rb");
  if (fp == NULL) {
    cout << "Error al abrir el archivo" << endl;
    return;
  }

  card_t card;
  while (fread(&card, sizeof(card_t), 1, fp) == 1) {
    cardVector.push_back(card);
  };

  fclose(fp);
}

bool validarTarjetas(string numeroTarjeta) {
  // void leerDatRanges()
  FILE *fp;
  vector<range_t> rangeVector = vector<range_t>(10);

  fp = fopen("ranges.dat", "rb");
  if (fp == NULL) {
    cout << "Error al abrir el archivo" << endl;
    return false;
  }

  range_t range;
  while (fread(&range, sizeof(range_t), 1, fp) == 1) {
    rangeVector.push_back(range);
  };

  fclose(fp);

  // void leerDatCards()
  FILE *pf;
  vector<card_t> cardVector = vector<card_t>(10);

  pf = fopen("cards.dat", "rb");
  if (pf == NULL) {
    cout << "Error al abrir el archivo" << endl;
    return false;
  }

  card_t card;
  while (fread(&card, sizeof(card_t), 1, pf) == 1) {
    cardVector.push_back(card);
  };

  fclose(pf);

  int idBuscadorEnCards;
  for (int i = 0; i < rangeVector.size(); i++) {
    if (numeroTarjeta.length() == rangeVector[i].len) {
      string ochoDigitos = numeroTarjeta.substr(0, 8);
      if (rangeVector[i].rangeLow <= ochoDigitos && ochoDigitos <= rangeVector[i].rangeHigh) {
        idBuscadorEnCards = rangeVector[i].id;
      }
    } else {
      cout << "TARJETA INVALIDA." << endl;
      return false;
    }
  }

  for (int i = 0; i < cardVector.size(); i++) {
    if (idBuscadorEnCards == cardVector[i].id) {
      cout << "Nombre de la tarjeta: " << cardVector[i].label << endl;
    } else {
      return false;
    }
  }
}

bool validarMonto(const string &monto) {
  regex formato("^\\d+\\.\\d{2}$");
  return regex_match(monto, formato);
}

bool validarNumeroTarjeta(const string &nroTarjeta) {
  regex formato("^[0-9]+$");
  return regex_match(nroTarjeta, formato);
}

string padstart(const string &texto, const int longitud, const char caracter) {
  string result = "";
  int cant = longitud - texto.length();
  if (cant < 1) return texto;
  for (int i = 0; i < cant; i++) {
    result.push_back(caracter);
  }
  result.append(texto);
  return result;
} 

datos_tarjeta_t *SolicitarDatosTarjeta() {
  string numeroTarjeta;
  string monto;
  string codigoSeguridad;

  bool montoValido = false;
  while (!montoValido) {
    cout << "Ingrese el monto de la compra: ";
    cin >> monto;
    if (!validarMonto(monto)) {
      cout << "Por favor ingrese el numero del monto con exactamente 2 "
              "decimales y punto decimal"
           << endl;
    } else {
      montoValido = true;
    }
  }

  bool numeroTarjetaValido = false;
  while (!numeroTarjetaValido) {
    cout << "Ingrese el numero de tarjeta: ";
    cin >> numeroTarjeta;
    if (!validarNumeroTarjeta(numeroTarjeta)) {
      cout << "Por favor ingrese solo numeros." << endl;
    } else if (numeroTarjeta.length() < 13) {
      cout << "El numero de tarjeta debe tener al menos 13 digitos. Intente de "
              "nuevo."
           << endl;
    } else {
      numeroTarjetaValido = true;
    }
  }
  if (!validarTarjetas(numeroTarjeta)) {
    cout << "TARJETA NO SOPORTADA" << endl;
    return NULL;
  };

  bool codigoSeguridadValido = false;
  while (!codigoSeguridadValido) {
    cout << "Ingrese codigo de seguridad: ";
    cin >> codigoSeguridad;
    if (codigoSeguridad.length() > 3) {
      cout << "El codigo de seguridad tiene que tener 3 digitos. Intente de "
              "nuevo."
           << endl;
    } else {
      codigoSeguridadValido = true;
    }
  }

  datos_tarjeta_t *datos_pointer = new datos_tarjeta_t;
  datos_pointer->numeroTarjeta = numeroTarjeta;
  datos_pointer->codigoSeguridad = codigoSeguridad;
  datos_pointer->monto = monto;

  return datos_pointer;
}

string ArmarMensajeRequest(datos_tarjeta_t *datos) {
  datos->monto.erase(datos->monto.length() - 3, 1);

  string mensaje = TIPO_MENSAJE + to_string(datos->numeroTarjeta.length()) + datos->numeroTarjeta +
                   padstart(datos->monto, LONGITUD_CEROS, '0') + datos->codigoSeguridad;

  return mensaje;
}

class Client {
 public:
  int clientSocket;
  char buffer[1024];
  sockaddr_in serverAddress;

  Client() {
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    int result = connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (result == SOCKET_ERROR) {
      cout << "Error al conectar!\n Error " << errno << endl;
    } else {
      cout << "Conectado al servidor! (c)" << endl;
    }
  }

  void Enviar() {
    datos_tarjeta_t *datos = SolicitarDatosTarjeta();
    if (datos == NULL) {
      return;
    }
    string mensaje = ArmarMensajeRequest(datos);
    char *buffer = new char[mensaje.length() + 1];
    strcpy(buffer, mensaje.c_str());
    send(clientSocket, buffer, strlen(buffer), 0);
    cout << "Mensaje enviado! (c)" << endl;
    delete datos;
  }

  void Recibir() {
    recv(clientSocket, buffer, sizeof(buffer), 0);
    cout << "El servidor dice "
         << "'" << buffer << "'" << endl;
    // resetea el arreglo buffer a 0.
    memset(buffer, 0, sizeof(buffer));
  }

  void CerrarSocket() {
    close(clientSocket);
    cout << "Cliente desconectado, socket cerrado." << endl;
  }
};

int main() {
  Client *cliente = new Client();

  cliente->Enviar();
  cliente->Recibir();

  delete cliente;

  return 0;
}