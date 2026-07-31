#include <Wire.h>
#include <NewPing.h>
#include <ESP32Servo.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

#define USTFRONT 13
#define USEFRONT 12
#define USTLEFT 14
#define USELEFT 27
#define USTRIGHT 26
#define USERIGHT 25

#define MAX_DISTANCE 357

#define IN2 19
#define IN1 18
#define PIN_SERVO 2
#define PIN_BOTON 15

NewPing USFRONT(USTFRONT, USEFRONT, MAX_DISTANCE);
NewPing USLEFT(USTLEFT, USELEFT, MAX_DISTANCE);
NewPing USRIGHT(USTRIGHT, USERIGHT, MAX_DISTANCE);

Servo myservo;

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire);

float anguloZ = 0;
float anguloFiltrado = 0;
float anguloMagnetometro = 0;
float anguloGiroscopio = 0;
unsigned long lastTimeBNO = 0;
bool bnoListo = false;

float anguloAcumulado = 0;
float anguloObjetivoRelativo = 0;
float anguloInicioGiroRelativo = 0;

const float ANGULO_GIRO_IZQUIERDA = 86.0;   // Grados que gira a la izquierda
const float ANGULO_GIRO_DERECHA = 71.3;     // Grados que gira a la derecha

const int DISTANCIA_OBSTACULO_FRONTAL = 40;
const int DISTANCIA_OBSTACULO_LATERAL = 120;
const unsigned long TIEMPO_ESPERA_GIRO = 700;
const int MAX_GIROS = 12;
const float AJUSTE_GIRO_RETROCESO = 0.2;
const int GRADOS_MAX_RETROCESO = 80;

const int COMPENSACION_CENTRO_IZQUIERDA = 0;
const int CENTRO_BASE = 95;
int centroServoActual = CENTRO_BASE;

const int ANGULO_SERVO_IZQ = 120;
const int ANGULO_SERVO_DER = 60;

bool programaIniciado = false;
bool finalizado = false;
bool girando = false;
bool enAvanceFinal = false;
bool giroIzquierda = false;
bool retrocediendo = false;
bool retrocesoRealizado = false;
bool recalculandoGiro = false;
bool obstaculoFrontalConfirmado = false;

unsigned long tiempoUltimoGiro = 0;
unsigned long tiempoInicioFinal = 0;
unsigned long tiempoInicioRetroceso = 0;
unsigned long tiempoInicioFrontal = 0;
int contadorGiros = 0;
int lecturasFrontales = 0;

float anguloObjetivo = 0;
float gyro = 0;
float anguloInicioGiro = 0;
float anguloOriginal = 0;
float anguloNecesario = 0;
float errorAcumulado = 0;

bool iniciarBNO055() {
  Serial.println("Inicializando BNO055...");
  
  Wire.begin(21, 22);
  Wire.setClock(400000);
  
  delay(2000);
  
  for (int intento = 0; intento < 10; intento++) {
    Serial.print("Intento ");
    Serial.print(intento + 1);
    Serial.print("/10... ");
    
    if (bno.begin()) {
      Serial.println("OK");
      bno.setExtCrystalUse(true);
      bno.setMode(OPERATION_MODE_COMPASS);
      
      uint8_t system, gyro, accel, mag;
      bno.getCalibration(&system, &gyro, &accel, &mag);
      Serial.print("Estado: S:");
      Serial.print(system);
      Serial.print(" G:");
      Serial.print(gyro);
      Serial.print(" A:");
      Serial.print(accel);
      Serial.print(" M:");
      Serial.println(mag);
      
      sensors_event_t event;
      bno.getEvent(&event);
      anguloMagnetometro = event.orientation.x;
      if (anguloMagnetometro > 180) anguloMagnetometro -= 360;
      
      anguloZ = -anguloMagnetometro;
      anguloFiltrado = anguloZ;
      anguloGiroscopio = anguloZ;
      lastTimeBNO = micros();
      
      anguloAcumulado = 0;
      
      return true;
    }
    
    Serial.println("FALLÓ");
    delay(500);
  }
  
  return false;
}

void actualizarBNO() {
  if (!bnoListo) return;
  
  imu::Vector<3> gyro = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);
  
  sensors_event_t event;
  bno.getEvent(&event);
  anguloMagnetometro = event.orientation.x;
  if (anguloMagnetometro > 180) anguloMagnetometro -= 360;
  
  unsigned long ahora = micros();
  float dt = (ahora - lastTimeBNO) / 1000000.0;
  lastTimeBNO = ahora;
  
  if (dt > 0 && dt < 0.1) {
    anguloGiroscopio += gyro.z() * dt;
    
    anguloFiltrado = 0.95 * (anguloFiltrado + gyro.z() * dt) + 0.05 * (-anguloMagnetometro);
    if (anguloFiltrado > 180) anguloFiltrado -= 360;
    if (anguloFiltrado < -180) anguloFiltrado += 360;
    
    anguloZ = anguloFiltrado;
  }
}

float getGyro() {
  actualizarBNO();
  return anguloZ;
}

float getAnguloRelativo() {
  imu::Vector<3> gyro = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);
  
  unsigned long ahora = micros();
  float dt = (ahora - lastTimeBNO) / 1000000.0;
  lastTimeBNO = ahora;
  
  if (dt > 0 && dt < 0.1) {
    anguloAcumulado += gyro.z() * dt;
  }
  
  return anguloAcumulado;
}

void resetearAnguloAcumulado() {
  anguloAcumulado = 0;
  Serial.println("Ángulo acumulado resetado a 0");
}

float normalizarError(float error) {
  if (error > 180) error -= 360;
  if (error < -180) error += 360;
  return error;
}

void motorAdelante() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
}

void motorRetroceder() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
}

void motorParar() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
}

void servoCentro() {
  if (girando || retrocediendo || enAvanceFinal) {
    myservo.write(CENTRO_BASE);
  } else {
    myservo.write(centroServoActual);
  }
}

void actualizarCentroServo(bool giroFueIzquierda) {
  if (giroFueIzquierda) {
    centroServoActual = CENTRO_BASE + COMPENSACION_CENTRO_IZQUIERDA;
    Serial.printf("Centro para RECTA ajustado a %d\n", centroServoActual);
  } else {
    centroServoActual = CENTRO_BASE;
    Serial.printf("Centro para RECTA restaurado a %d\n", centroServoActual);
  }
}

int leerUltrasonico(NewPing &sensor) {
  int d = sensor.ping_cm();
  if (d == 0) return -1;
  return d;
}

int correccionServo(float error) {
  float ae = abs(error);
  
  if (ae > 35) return 25;
  if (ae > 20) return 15;
  if (ae > 10) return 8;
  return 4;
}

void Adelante() {
  motorAdelante();
  servoCentro();
}

void Parar() {
  motorParar();
}

void iniciarGiroIzquierda() {
  float anguloActual = getAnguloRelativo();
  anguloInicioGiroRelativo = anguloActual;
  anguloOriginal = ANGULO_GIRO_IZQUIERDA;
  anguloObjetivoRelativo = anguloActual + ANGULO_GIRO_IZQUIERDA;
  
  giroIzquierda = true;
  girando = true;
  retrocediendo = false;
  retrocesoRealizado = false;
  recalculandoGiro = false;
  obstaculoFrontalConfirmado = false;
  lecturasFrontales = 0;
  
  myservo.write(ANGULO_SERVO_IZQ);
  motorAdelante();
  
  Serial.printf("Inicio giro IZQUIERDA: Actual: %.1f° | Objetivo: %.1f°\n", 
                anguloActual, anguloObjetivoRelativo);
}

void iniciarGiroDerecha() {
  float anguloActual = getAnguloRelativo();
  anguloInicioGiroRelativo = anguloActual;
  anguloOriginal = ANGULO_GIRO_DERECHA;
  anguloObjetivoRelativo = anguloActual - ANGULO_GIRO_DERECHA;
  
  giroIzquierda = false;
  girando = true;
  retrocediendo = false;
  retrocesoRealizado = false;
  recalculandoGiro = false;
  obstaculoFrontalConfirmado = false;
  lecturasFrontales = 0;
  
  myservo.write(ANGULO_SERVO_DER);
  motorAdelante();
  
  Serial.printf("Inicio giro DERECHA: Actual: %.1f° | Objetivo: %.1f°\n", 
                anguloActual, anguloObjetivoRelativo);
}

void actualizarGiro() {
  float anguloActual = getAnguloRelativo();

  float error = anguloObjetivoRelativo - anguloActual;
  int corr = correccionServo(error);
  
  float gradosGirados = abs(anguloActual - anguloInicioGiroRelativo);

  int frontal = leerUltrasonico(USFRONT);
  
  if (frontal != -1 && frontal <= DISTANCIA_OBSTACULO_FRONTAL) {
    lecturasFrontales++;
    
    if (lecturasFrontales >= 3 || (millis() - tiempoInicioFrontal > 200 && lecturasFrontales > 0)) {
      obstaculoFrontalConfirmado = true;
    }
    
    if (lecturasFrontales == 1) {
      tiempoInicioFrontal = millis();
    }
  } else {
    if (lecturasFrontales > 0) {
      lecturasFrontales--;
    }
    if (lecturasFrontales == 0) {
      obstaculoFrontalConfirmado = false;
      tiempoInicioFrontal = 0;
    }
  }

  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 200) {
    Serial.printf("Angulo: %.1f° | Objetivo: %.1f° | Error: %.1f° | Girados: %.1f° | Servo: %d°\n",
                  anguloActual, anguloObjetivoRelativo, error, gradosGirados, myservo.read());
    lastPrint = millis();
  }

  if (!retrocesoRealizado && !recalculandoGiro && 
      obstaculoFrontalConfirmado && 
      gradosGirados < GRADOS_MAX_RETROCESO && 
      !retrocediendo) {
    
    Serial.printf("OBSTACULO FRONTAL A %dcm - Retrocediendo\n", frontal);
    
    float anguloFaltante = anguloOriginal - gradosGirados;
    float anguloConAjuste = anguloFaltante - AJUSTE_GIRO_RETROCESO;
    
    if (giroIzquierda) {
      anguloNecesario = anguloActual + anguloConAjuste;
    } else {
      anguloNecesario = anguloActual - anguloConAjuste;
    }
    
    retrocediendo = true;
    retrocesoRealizado = true;
    recalculandoGiro = true;
    tiempoInicioRetroceso = millis();
    motorParar();
    myservo.write(CENTRO_BASE);
    delay(50);
    motorRetroceder();
    
    Serial.printf("Ángulo faltante: %.1f° | Nuevo objetivo: %.1f°\n", anguloFaltante, anguloNecesario);
    return;
  }
  
  if (retrocediendo) {
    myservo.write(CENTRO_BASE);
    
    if (millis() - tiempoInicioRetroceso < 800) {
      motorRetroceder();
      return;
    } else {
      retrocediendo = false;
      Serial.println("Retroceso completado");
      
      anguloObjetivoRelativo = anguloNecesario;
      
      Serial.printf("Reanudando giro con nuevo objetivo: %.1f°\n", anguloObjetivoRelativo);
      
      if (giroIzquierda) {
        myservo.write(ANGULO_SERVO_IZQ);
      } else {
        myservo.write(ANGULO_SERVO_DER);
      }
      motorAdelante();
      return;
    }
  }

  if (abs(error) <= 4) {
    motorParar();
    myservo.write(CENTRO_BASE);
    girando = false;
    retrocesoRealizado = false;
    recalculandoGiro = false;
    obstaculoFrontalConfirmado = false;
    lecturasFrontales = 0;
    contadorGiros++;
    tiempoUltimoGiro = millis();

    actualizarCentroServo(giroIzquierda);

    Serial.printf("Giro completado. Conteo: %d/%d\n", contadorGiros, MAX_GIROS);

    if (contadorGiros >= MAX_GIROS) {
      enAvanceFinal = true;
      tiempoInicioFinal = millis();
    }
    return;
  }

  if (error > 0) {
    myservo.write(CENTRO_BASE + corr);
  } else {
    myservo.write(CENTRO_BASE - corr);
  }

  motorAdelante();
}

void actualizarFinal() {
  motorAdelante();
  if (millis() - tiempoInicioFinal >= 100) {
    Parar();
    myservo.write(CENTRO_BASE);
    enAvanceFinal = false;
    finalizado = true;
    Serial.println("Giros completados - Robot detenido");
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n");
  Serial.printf("Giro IZQUIERDA: %.1f°\n", ANGULO_GIRO_IZQUIERDA);
  Serial.printf("Giro DERECHA: %.1f°\n", ANGULO_GIRO_DERECHA);
  Serial.println();

  pinMode(PIN_BOTON, INPUT_PULLUP);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  myservo.attach(PIN_SERVO);
  centroServoActual = CENTRO_BASE;
  myservo.write(CENTRO_BASE);

  bnoListo = iniciarBNO055();
  if (!bnoListo) {
    Serial.println("ERROR: BNO055 no detectado");
    while (1) delay(10);
  }
  Serial.println("BNO055 listo");

  resetearAnguloAcumulado();

  Parar();
  delay(3000);
  Serial.println("Esperando pulsar boton para iniciar...");
}

void loop() {
  if (!programaIniciado) {
    if (digitalRead(PIN_BOTON) == LOW) {
      programaIniciado = true;
      Serial.println("Boton presionado, iniciando programa...");
      delay(300);
    }
    return;
  }

  if (finalizado) {
    Parar();
    return;
  }

  if (enAvanceFinal) {
    actualizarFinal();
    return;
  }

  if (girando) {
    actualizarGiro();
    return;
  }

  int frontal = leerUltrasonico(USFRONT);
  int izquierda = leerUltrasonico(USLEFT);
  int derecha = leerUltrasonico(USRIGHT);

  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 500) {
    Serial.printf("Frontal: %d | Izquierda: %d | Derecha: %d\n", frontal, izquierda, derecha);
    lastPrint = millis();
  }

  if (frontal != -1 && frontal <= DISTANCIA_OBSTACULO_FRONTAL) {
    Parar();
    return;
  }

  Adelante();

  if (millis() - tiempoUltimoGiro < TIEMPO_ESPERA_GIRO) {
    return;
  }

  if (izquierda != -1 && izquierda > DISTANCIA_OBSTACULO_LATERAL) {
    Serial.println("Girando a la izquierda por espacio libre");
    iniciarGiroIzquierda();
    return;
  }

  if (derecha != -1 && derecha > DISTANCIA_OBSTACULO_LATERAL) {
    Serial.println("Girando a la derecha por espacio libre");
    iniciarGiroDerecha();
    return;
  }
}