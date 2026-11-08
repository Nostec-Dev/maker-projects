/*
  Diagnostico del sensor de fuerza (HX711) con ESP32

  No necesita calibracion ni la libreria HX711: lee el modulo directamente.
  Muestra el resultado en la pantalla y en el Monitor Serie (115200).

  Conexiones esperadas:
    HX711 DT -> GPIO 18    SCK -> GPIO 19
    OLED SDA -> GPIO 21    SCK -> GPIO 22
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const int PIN_SDA    = 21;
const int PIN_SCL    = 22;
const int PIN_HX_DT  = 18;
const int PIN_HX_SCK = 19;

const long LIMITE_SATURADO = 8380000;  // la lectura maxima es 8388607
const long CAMBIO_MINIMO   = 2000;     // cambio que cuenta como reaccion
const int  LECTURAS_BASE   = 5;

Adafruit_SSD1306 pantalla(128, 64, &Wire, -1);
portMUX_TYPE candado = portMUX_INITIALIZER_UNLOCKED;

long sumaBase = 0;
int  cuentaBase = 0;
long base = 0;

// Lee 24 bits del HX711. Devuelve false si DT nunca baja.
bool leerHX711(long &valor, bool &dtSubio) {
  unsigned long inicio = millis();
  while (digitalRead(PIN_HX_DT) == HIGH) {
    if (millis() - inicio > 500) return false;
    delay(1);
  }

  uint32_t dato = 0;
  portENTER_CRITICAL(&candado);
  for (int i = 0; i < 24; i++) {
    digitalWrite(PIN_HX_SCK, HIGH);
    delayMicroseconds(1);
    dato = (dato << 1) | digitalRead(PIN_HX_DT);
    digitalWrite(PIN_HX_SCK, LOW);
    delayMicroseconds(1);
  }
  // Pulso 25: ganancia 128. Un HX711 real sube DT aqui.
  digitalWrite(PIN_HX_SCK, HIGH);
  delayMicroseconds(1);
  digitalWrite(PIN_HX_SCK, LOW);
  delayMicroseconds(1);
  dtSubio = digitalRead(PIN_HX_DT) == HIGH;
  portEXIT_CRITICAL(&candado);

  if (dato & 0x800000) {
    dato |= 0xFF000000;   // numero negativo
  }
  valor = (int32_t)dato;
  return true;
}

void mostrar(const char* estado, long crudo, long cambio) {
  Serial.print(estado);
  Serial.print("   Crudo: ");
  Serial.print(crudo);
  Serial.print("   Cambio: ");
  Serial.println(cambio);

  pantalla.clearDisplay();
  pantalla.setTextSize(1);
  pantalla.setCursor(0, 0);
  pantalla.print("DIAGNOSTICO");
  pantalla.setCursor(0, 16);
  pantalla.print("Crudo: ");
  pantalla.print(crudo);
  pantalla.setCursor(0, 28);
  pantalla.print("Cambio: ");
  pantalla.print(cambio);
  pantalla.setCursor(0, 46);
  pantalla.print(estado);
  pantalla.display();
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_HX_DT, INPUT_PULLUP);
  pinMode(PIN_HX_SCK, OUTPUT);
  digitalWrite(PIN_HX_SCK, LOW);

  Wire.begin(PIN_SDA, PIN_SCL);
  pantalla.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  pantalla.setTextColor(SSD1306_WHITE);
  delay(500);
}

void loop() {
  long crudo = 0;
  bool dtSubio = false;

  if (!leerHX711(crudo, dtSubio)) {
    mostrar("1 DT SIEMPRE ALTO", 0, 0);
    return;
  }

  if (!dtSubio || crudo == 0 || crudo == -1) {
    mostrar("2 DT PEGADO ABAJO", crudo, 0);
    delay(300);
    return;
  }

  if (crudo >= LIMITE_SATURADO || crudo <= -LIMITE_SATURADO) {
    mostrar("3 SATURADO", crudo, 0);
    delay(300);
    return;
  }

  if (cuentaBase < LECTURAS_BASE) {
    sumaBase += crudo;
    cuentaBase++;
    base = sumaBase / cuentaBase;
    mostrar("No toques el agarre", crudo, 0);
    delay(300);
    return;
  }

  long cambio = crudo - base;
  if (abs(cambio) >= CAMBIO_MINIMO) {
    mostrar("4 EL SENSOR REACCIONA", crudo, cambio);
  } else {
    mostrar("5 APRIETA EL AGARRE", crudo, cambio);
  }
  delay(100);
}
