/*
  Ejercitador de fuerza: ESP32 + celda de 20 kg (HX711) + OLED I2C + Bluetooth

  Conexiones:
    OLED  SDA -> GPIO 21    SCK -> GPIO 22    VDD -> 3V3    GND -> GND
    HX711 DT  -> GPIO 18    SCK -> GPIO 19    VCC -> 3V3    GND -> GND

  Librerias (Gestor de librerias de Arduino IDE):
    - HX711 Arduino Library (Bogdan Necula)
    - Adafruit SSD1306 (instala tambien Adafruit GFX)
    - El Bluetooth (BLE) ya viene con la placa ESP32, no hay que instalarlo

  Bluetooth:
    - El ESP32 aparece como "Ejercitador"
    - Envia "kg,N,s" unas 5 veces por segundo (ejemplo: 2.27,22.3,4.1)
    - Si recibe "t", pone la celda en cero

  Comandos en el Monitor Serie (115200 baudios):
    t -> poner en cero
    c -> calibrar con un peso conocido
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HX711.h>
#include <Preferences.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// ----- Pines -----
const int PIN_SDA    = 21;
const int PIN_SCL    = 22;
const int PIN_HX_DT  = 18;
const int PIN_HX_SCK = 19;

// ----- Bluetooth -----
// Mismos UUID que el "UART" de Nordic, asi tambien se puede probar con nRF Connect
const char* NOMBRE_BT       = "Ejercitador";
const char* UUID_SERVICIO   = "6e400001-b5a3-f393-e0a9-e50e24dcca9e";
const char* UUID_RECIBIR    = "6e400002-b5a3-f393-e0a9-e50e24dcca9e";  // la pagina escribe aqui
const char* UUID_ENVIAR     = "6e400003-b5a3-f393-e0a9-e50e24dcca9e";  // el ESP32 notifica aqui

// ----- Pantalla -----
const int  ANCHO_OLED     = 128;
const int  ALTO_OLED      = 64;
const byte DIRECCION_OLED = 0x3C;

// ----- Ajustes de medida -----
const float CAPACIDAD_KG = 20.0;  // limite de la celda y tope de la barra
const float ZONA_CERO_KG = 0.02;  // por debajo de esto se muestra 0.00
const int   LECTURAS     = 2;     // promedio en cada medida
const float GRAVEDAD     = 9.81;  // 1 kg = 9.81 N

// Calibracion de esta celda, medida con 2.27 kg (lecturas crudas por cada kg).
// Si se envia c por Serial, la nueva calibracion guardada reemplaza este valor.
const float FACTOR_CALIBRADO = 133393.39;

// ----- Tiempo sostenido -----
const float INICIO_TIEMPO_KG = 0.20;  // desde esta fuerza empieza a contar
const float FIN_TIEMPO_KG    = 0.10;  // por debajo de esta fuerza se detiene

// ----- Encendido -----
const int DURACION_ENCENDIDO_MS = 2000;

Adafruit_SSD1306 pantalla(ANCHO_OLED, ALTO_OLED, &Wire, -1);
HX711 celda;
Preferences memoria;

float factor = FACTOR_CALIBRADO;
bool hayPantalla = false;

bool sosteniendo = false;
unsigned long inicioMs = 0;
float segundos = 0;

BLEServer* servidorBT = nullptr;
BLECharacteristic* caracEnviar = nullptr;
volatile bool btConectado = false;
bool btConectadoAntes = false;
volatile bool pedirCero = false;

class EventosServidor : public BLEServerCallbacks {
  void onConnect(BLEServer* servidor) {
    btConectado = true;
  }
  void onDisconnect(BLEServer* servidor) {
    btConectado = false;
  }
};

class EventosRecibir : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* carac) {
    auto valor = carac->getValue();
    if (valor.length() > 0 && (valor[0] == 't' || valor[0] == 'T')) {
      pedirCero = true;   // se hace en loop(), no dentro del Bluetooth
    }
  }
};

void iniciarBluetooth() {
  BLEDevice::init(NOMBRE_BT);
  servidorBT = BLEDevice::createServer();
  servidorBT->setCallbacks(new EventosServidor());

  BLEService* servicio = servidorBT->createService(UUID_SERVICIO);

  caracEnviar = servicio->createCharacteristic(
    UUID_ENVIAR, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ);
  caracEnviar->addDescriptor(new BLE2902());

  BLECharacteristic* caracRecibir = servicio->createCharacteristic(
    UUID_RECIBIR, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
  caracRecibir->setCallbacks(new EventosRecibir());

  servicio->start();

  BLEAdvertising* anuncio = BLEDevice::getAdvertising();
  anuncio->addServiceUUID(UUID_SERVICIO);
  anuncio->setScanResponse(true);
  BLEDevice::startAdvertising();
  Serial.println("Bluetooth listo: busca \"Ejercitador\"");
}

// Funciones de pantalla que estan mas abajo
void mostrarMensaje(const char* linea1, const char* linea2);
void dibujarIconoBT();

// Envia la lectura a la pagina como texto "kg,N,s"
void enviarBluetooth(float kg, float newtons) {
  // Recien conectado: aviso grande en la pantalla
  if (btConectado && !btConectadoAntes) {
    Serial.println("Bluetooth conectado a la pagina");
    mostrarMensaje("Bluetooth", "Conectado a la pagina");
    dibujarIconoBT();
    pantalla.display();
    delay(1500);
  }
  // Si se desconecto, avisa y vuelve a anunciarse para que la pagina pueda reconectar
  if (!btConectado && btConectadoAntes) {
    Serial.println("Bluetooth desconectado, esperando conexion");
    mostrarMensaje("Bluetooth", "Desconectado");
    delay(1000);
    BLEDevice::startAdvertising();
  }
  btConectadoAntes = btConectado;
  if (!btConectado) return;

  char texto[32];
  int largo = snprintf(texto, sizeof(texto), "%.2f,%.1f,%.1f", kg, newtons, segundos);
  caracEnviar->setValue((uint8_t*)texto, largo);
  caracEnviar->notify();
}

// Muestra dos lineas de texto pequeno en la pantalla
void mostrarMensaje(const char* linea1, const char* linea2) {
  if (!hayPantalla) return;
  pantalla.clearDisplay();
  pantalla.setTextSize(1);
  pantalla.setCursor(0, 16);
  pantalla.println(linea1);
  pantalla.setCursor(0, 32);
  pantalla.println(linea2);
  pantalla.display();
}

// Pantalla de bienvenida con una barra que se va llenando
void mostrarEncendido() {
  if (!hayPantalla) return;
  const int PASOS = 20;
  for (int i = 1; i <= PASOS; i++) {
    pantalla.clearDisplay();
    pantalla.setTextSize(1);
    pantalla.setCursor(22, 18);
    pantalla.print("Encendiendo...");
    pantalla.drawRect(14, 36, 100, 10, SSD1306_WHITE);
    pantalla.fillRect(16, 38, 96 * i / PASOS, 6, SSD1306_WHITE);
    pantalla.display();
    delay(DURACION_ENCENDIDO_MS / PASOS);
  }
}

// Icono de Bluetooth (8 x 13 pixeles) en la esquina superior derecha
void dibujarIconoBT() {
  if (!hayPantalla) return;
  const int x = 122, y = 0;
  pantalla.drawLine(x, y, x, y + 12, SSD1306_WHITE);              // palo central
  pantalla.drawLine(x, y, x + 3, y + 3, SSD1306_WHITE);           // punta de arriba
  pantalla.drawLine(x + 3, y + 3, x - 3, y + 9, SSD1306_WHITE);   // diagonal hacia abajo
  pantalla.drawLine(x - 3, y + 3, x + 3, y + 9, SSD1306_WHITE);   // diagonal hacia arriba
  pantalla.drawLine(x + 3, y + 9, x, y + 12, SSD1306_WHITE);      // punta de abajo
}

// Muestra peso, newtons, tiempo sostenido y barra de fuerza
void mostrarFuerza(float kg, float newtons) {
  if (!hayPantalla) return;
  pantalla.clearDisplay();

  // Peso en grande
  pantalla.setTextSize(2);
  pantalla.setCursor(0, 0);
  pantalla.print(kg, 2);
  pantalla.print(" kg");

  // Indicador de Bluetooth conectado
  if (btConectado) {
    dibujarIconoBT();
  }

  // Newtons y tiempo
  pantalla.setTextSize(1);
  pantalla.setCursor(0, 20);
  pantalla.print("Fuerza: ");
  pantalla.print(newtons, 1);
  pantalla.print(" N");

  pantalla.setCursor(0, 31);
  pantalla.print("Tiempo: ");
  pantalla.print(segundos, 1);
  pantalla.print(" s");

  // Barra de fuerza: vacia en 0 kg, llena en 20 kg
  float parte = constrain(kg / CAPACIDAD_KG, 0.0f, 1.0f);
  pantalla.drawRect(0, 42, ANCHO_OLED, 12, SSD1306_WHITE);
  pantalla.fillRect(2, 44, (int)((ANCHO_OLED - 4) * parte), 8, SSD1306_WHITE);

  // Escala de la barra, o aviso si se pasa del limite
  if (kg > CAPACIDAD_KG) {
    pantalla.setCursor(34, 56);
    pantalla.print("SOBRECARGA");
  } else {
    pantalla.setCursor(0, 56);
    pantalla.print("0");
    pantalla.setCursor(98, 56);
    pantalla.print("20 kg");
  }
  pantalla.display();
}

// Cuenta mientras se aprieta. Al soltar deja el ultimo tiempo en pantalla
// y vuelve a empezar desde 0 en el siguiente apreton.
void actualizarTiempo(float kg) {
  if (!sosteniendo && kg >= INICIO_TIEMPO_KG) {
    sosteniendo = true;
    inicioMs = millis();
    segundos = 0;
  } else if (sosteniendo && kg < FIN_TIEMPO_KG) {
    sosteniendo = false;
  }

  if (sosteniendo) {
    segundos = (millis() - inicioMs) / 1000.0;
  }
}

// Espera a que se escriba algo en el Monitor Serie
String leerLinea() {
  while (!Serial.available()) {
    delay(10);
  }
  String texto = Serial.readStringUntil('\n');
  texto.trim();
  return texto;
}

void ponerEnCero() {
  mostrarMensaje("Poniendo en cero", "No toques la celda");
  celda.tare(20);
  sosteniendo = false;
  segundos = 0;
  Serial.println("Cero listo.");
}

void calibrar() {
  Serial.println();
  Serial.println("--- CALIBRACION ---");
  Serial.println("1) Quita todo el peso y envia cualquier letra.");
  mostrarMensaje("Calibrando", "Quita todo el peso");
  leerLinea();

  float factorAnterior = factor;
  celda.set_scale(1.0);
  celda.tare(20);

  Serial.println("2) Pon un peso conocido y escribe cuanto pesa en kg (ejemplo: 1.0).");
  mostrarMensaje("Calibrando", "Pon el peso conocido");
  String texto = leerLinea();
  texto.replace(',', '.');
  float pesoKg = texto.toFloat();

  if (pesoKg <= 0) {
    Serial.println("Peso no valido. Calibracion cancelada.");
    celda.set_scale(factorAnterior);
    return;
  }

  mostrarMensaje("Calibrando", "Midiendo...");
  float crudo = celda.get_units(20);

  if (abs(crudo) < 100) {
    Serial.println("La lectura casi no cambio. Revisa los cables de la celda.");
    celda.set_scale(factorAnterior);
    return;
  }

  factor = crudo / pesoKg;   // si la celda da negativo, el factor corrige el signo
  celda.set_scale(factor);
  memoria.putFloat("factor", factor);

  Serial.print("Listo. Factor guardado: ");
  Serial.println(factor, 2);
}

void revisarComandos() {
  if (!Serial.available()) return;

  String comando = leerLinea();
  if (comando == "t" || comando == "T") {
    ponerEnCero();
  } else if (comando == "c" || comando == "C") {
    calibrar();
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);

  Wire.begin(PIN_SDA, PIN_SCL);
  hayPantalla = pantalla.begin(SSD1306_SWITCHCAPVCC, DIRECCION_OLED);
  if (hayPantalla) {
    pantalla.setTextColor(SSD1306_WHITE);
    mostrarEncendido();
  } else {
    Serial.println("No encuentro la pantalla. Revisa SDA, SCK y alimentacion.");
  }

  memoria.begin("fuerza", false);
  float guardado = memoria.getFloat("factor", 0);
  if (guardado != 0) {
    factor = guardado;
  }

  celda.begin(PIN_HX_DT, PIN_HX_SCK);
  celda.set_scale(factor);

  if (celda.wait_ready_timeout(2000)) {
    ponerEnCero();
  } else {
    Serial.println("No encuentro el HX711. Revisa DT, SCK y alimentacion.");
  }

  iniciarBluetooth();
  Serial.println("Comandos: t = cero, c = calibrar");
}

void loop() {
  revisarComandos();
  if (pedirCero) {
    pedirCero = false;
    ponerEnCero();
  }

  if (!celda.wait_ready_timeout(500)) {
    mostrarMensaje("HX711 sin senal", "Revisa las conexiones");
    return;
  }

  float kg = celda.get_units(LECTURAS);
  if (abs(kg) < ZONA_CERO_KG) {
    kg = 0;
  }

  float newtons = kg * GRAVEDAD;
  actualizarTiempo(kg);
  mostrarFuerza(kg, newtons);
  enviarBluetooth(kg, newtons);

  Serial.print("Peso: ");
  Serial.print(kg, 2);
  Serial.print(" kg   Fuerza: ");
  Serial.print(newtons, 1);
  Serial.print(" N   Tiempo: ");
  Serial.print(segundos, 1);
  Serial.println(" s");
}
