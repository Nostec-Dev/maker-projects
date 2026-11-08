/*
  Calibracion de la celda de carga de 20 kg con HX711 y ESP32

  Conexiones:
    HX711 DT -> GPIO 18    SCK -> GPIO 19    VCC -> 3V3    GND -> GND

  Uso (Monitor Serie a 115200 baudios):
    1) Escribe c y envia.
    2) Quita todo el peso y envia cualquier letra.
    3) Pon el peso conocido, escribe cuanto pesa en kg y envia.

  El factor queda guardado en el ESP32. El codigo principal
  (ejercitador_bluetooth.ino) lo lee solo al encender.
*/

#include <HX711.h>
#include <Preferences.h>

const int PIN_HX_DT  = 18;
const int PIN_HX_SCK = 19;

const float CAPACIDAD_KG = 20.0;

HX711 celda;
Preferences memoria;

bool calibrado = false;
unsigned long ultimoAviso = 0;

// Espera a que se escriba algo en el Monitor Serie
String leerLinea() {
  while (!Serial.available()) {
    delay(10);
  }
  String texto = Serial.readStringUntil('\n');
  texto.trim();
  return texto;
}

void calibrar() {
  calibrado = false;

  if (!celda.wait_ready_timeout(2000)) {
    Serial.println("No encuentro el HX711. Revisa DT, SCK y alimentacion.");
    return;
  }

  Serial.println();
  Serial.println("--- CALIBRACION ---");
  Serial.println("PASO 1: Quita todo el peso y envia cualquier letra.");
  leerLinea();

  Serial.println("Poniendo en cero, no toques la celda...");
  celda.set_scale(1.0);
  celda.tare(20);

  Serial.println("PASO 2: Pon el peso conocido.");
  Serial.println("Cuando este quieto, escribe cuanto pesa en kg y envia (ejemplo: 1.0).");
  String texto = leerLinea();
  texto.replace(',', '.');
  float pesoKg = texto.toFloat();

  if (pesoKg <= 0 || pesoKg > CAPACIDAD_KG) {
    Serial.println("Peso no valido: debe ser mayor que 0 y maximo 20 kg.");
    return;
  }

  Serial.println("Midiendo, no muevas el peso...");
  float crudo = celda.get_units(20);

  if (abs(crudo) < 100) {
    Serial.println("La lectura casi no cambio. Revisa los cables de la celda.");
    return;
  }

  float factor = crudo / pesoKg;
  celda.set_scale(factor);
  memoria.putFloat("factor", factor);
  calibrado = true;

  Serial.println();
  Serial.print("LISTO. Factor guardado: ");
  Serial.println(factor, 2);
  Serial.println("Comprueba abajo que marque tu peso. Para repetir, envia c.");
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(500);

  memoria.begin("fuerza", false);
  celda.begin(PIN_HX_DT, PIN_HX_SCK);
}

void loop() {
  if (Serial.available()) {
    String comando = leerLinea();
    if (comando == "c" || comando == "C") {
      calibrar();
    }
  }

  if (calibrado) {
    if (celda.wait_ready_timeout(500)) {
      Serial.print("Peso medido: ");
      Serial.print(celda.get_units(5), 2);
      Serial.println(" kg");
    }
    delay(300);
  } else if (millis() - ultimoAviso >= 3000) {
    ultimoAviso = millis();
    Serial.println("Escribe c y envia para calibrar.");
  }
}
