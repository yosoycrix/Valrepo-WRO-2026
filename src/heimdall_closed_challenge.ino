#include <Wire.h>
#include <NewPing.h>
#include <ESP32Servo.h>
#include <DFRobot_HuskylensV2.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

// ----- VELOCIDADES -----
#define VELOCIDAD_NORMAL   210
#define VELOCIDAD_REVERSA   160
#define VELOCIDAD_RETROCESO_EVASIVO  50   // Velocidad para retroceso por obstáculo frontal

// ----- ÁNGULOS DEL SERVO -----
#define ANGULO_CENTRO       97
#define ANGULO_IZQUIERDA    54
#define ANGULO_DERECHA      140
#define ANGULO_ESQUIVA_IZQ  54
#define ANGULO_ESQUIVA_DER  140

// ----- TICKS DE ESQUIVA (VERDE) -----
#define TICKS_VERDE_PRIMER     510
#define TICKS_VERDE_SEGUNDO    610
#define TICKS_VERDE_GIRO_CONTRARIO   600
#define TICKS_VERDE_GIRO_VUELTA      600
#define TICKS_VERDE_RETROCESO_VERIF  290

// ----- TICKS DE ESQUIVA (ROJO) -----
#define TICKS_ROJO_PRIMER      440
#define TICKS_ROJO_SEGUNDO     530
#define TICKS_ROJO_GIRO_CONTRARIO   400
#define TICKS_ROJO_GIRO_VUELTA      490
#define TICKS_ROJO_RETROCESO_VERIF  230

// ----- TICKS POST-ESQUIVA COMUNES -----
#define TICKS_AVANCE_POST_ESQUIVA  0

// ----- TICKS PARA RETROCESO EVASIVO (obstáculo frontal) -----
#define TICKS_RETROCESO_EVASIVO 830

// ----- DISTANCIA FRONTAL PARA ACTIVAR RETROCESO EVASIVO (cm) -----
#define DISTANCIA_FRONTAL_UMBRAL  30

// ----- PID -----
const float KP = 1.8, KI = 0.1, KD = 0.25;
const int LIMITE_CORRECCION = 30, DEAD_BAND = 2;

// ----- HUSKYLENS -----
const int ID_ROJO = 1;
const int ID_VERDE = 2;
const int MITAD_PANTALLA = 320;
const int UMBRAL_CENTRO = 40;
const int UMBRAL_ALTURA = 230;

// ----- DISTANCIAS LATERALES -----
const int DISTANCIA_LATERAL_GIRO = 120;   // cm para considerar espacio libre

#define IN1 18
#define IN2 19
#define PIN_SERVO 2
#define MOTOR_PWM_CHANNEL 6
#define MOTOR_PWM_PIN IN1

#define ENCODER_PIN_A  34
#define ENCODER_PIN_B  35
#define RX_PIN 16
#define TX_PIN 17
#define PIN_BOTON 15

#define TRIG_FRONTAL   13
#define ECHO_FRONTAL   12
#define TRIG_IZQUIERDO  14
#define ECHO_IZQUIERDO  27
#define TRIG_DERECHO    26
#define ECHO_DERECHO    25
#define MAX_DISTANCIA   300

NewPing USFRONT(TRIG_FRONTAL, ECHO_FRONTAL, MAX_DISTANCIA);
NewPing USLEFT(TRIG_IZQUIERDO, ECHO_IZQUIERDO, MAX_DISTANCIA);
NewPing USRIGHT(TRIG_DERECHO, ECHO_DERECHO, MAX_DISTANCIA);
Servo myservo;
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire);
HuskylensV2 huskylens;

// BNO055
float anguloRelativo = 0, anguloReferencia = 0;
bool bnoListo = false;
unsigned long lastTimeBNO = 0;

// Encoder
volatile long encoderTicks = 0;
long ticksAlInicio = 0;

// HuskyLens
bool sensorConectado = false;
unsigned long lastHuskyRead = 0;
const unsigned long READ_INTERVAL = 50;

// Ultrasonidos
int distanciaIzquierda = 0, distanciaDerecha = 0;

// Estado del robot
bool bloqueDetectado = false;
int ID_detectado = 0;
int alturaBloque = 0;
int centroX = 0;
int posicionBloque = 0;   // -1: izquierda, 0: centro, 1: derecha

// PID
float errorAcumulado = 0, errorAnterior = 0;
unsigned long tiempoAnteriorPID = 0;

// Máquina de estados
enum Estado {
  BUSCANDO,
  ESQUIVANDO,
  AJUSTE_POST_ESQUIVA,
  RETROCESO_EVASIVO,
  DETENIDO_PERMANENTE
};
Estado estado = BUSCANDO;
int faseEsquiva = 0;
int ticksPrimer, ticksSegundo;
int ticksGiroContrario, ticksGiroVuelta, ticksRetrocesoVerif;
int direccionPrimer;      // 1 = derecha, -1 = izquierda
int direccionContraria;   // Lado contrario al de la esquiva
unsigned long tiempoInicioFase = 0;

bool programaIniciado = false;

bool iniciarBNO055() {
  Wire.begin(21, 22);
  Wire.setClock(400000);
  delay(2000);
  for (int i = 0; i < 10; i++) {
    if (bno.begin()) {
      bno.setExtCrystalUse(true);
      bno.setMode(OPERATION_MODE_COMPASS);
      sensors_event_t event;
      bno.getEvent(&event);
      float ang = event.orientation.x;
      if (ang > 180) ang -= 360;
      anguloRelativo = -ang;
      lastTimeBNO = micros();
      return true;
    }
    delay(500);
  }
  return false;
}

float getAnguloRelativo() {
  if (!bnoListo) return 0;
  imu::Vector<3> gyro = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);
  sensors_event_t event;
  bno.getEvent(&event);
  float ang = event.orientation.x;
  if (ang > 180) ang -= 360;
  unsigned long ahora = micros();
  float dt = (ahora - lastTimeBNO) / 1000000.0;
  lastTimeBNO = ahora;
  if (dt > 0 && dt < 0.1) {
    anguloRelativo = 0.98 * (anguloRelativo + gyro.z() * dt) + 0.02 * (-ang);
    if (anguloRelativo > 180) anguloRelativo -= 360;
    if (anguloRelativo < -180) anguloRelativo += 360;
  }
  return anguloRelativo;
}

void IRAM_ATTR encoderISR() { encoderTicks++; }
void iniciarEncoder() {
  pinMode(ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_PIN_B, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), encoderISR, RISING);
  encoderTicks = 0;
}
void resetearTicks() { encoderTicks = 0; }
void guardarTicksInicio() { ticksAlInicio = encoderTicks; }
long obtenerTicksRecorridos() { return abs(encoderTicks - ticksAlInicio); }
bool alcanzoTicks(int objetivo) { return obtenerTicksRecorridos() >= objetivo; }

void avanzarMotor() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  ledcWrite(MOTOR_PWM_CHANNEL, VELOCIDAD_NORMAL);
}

// Retroceso normal para esquivas (usa VELOCIDAD_REVERSA)
void retroceder() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  ledcWrite(MOTOR_PWM_CHANNEL, VELOCIDAD_REVERSA);
}

// Retroceso específico para obstáculo frontal (usa VELOCIDAD_RETROCESO_EVASIVO)
void retrocederEvasivo() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  ledcWrite(MOTOR_PWM_CHANNEL, VELOCIDAD_RETROCESO_EVASIVO);
}

void detenerMotor() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  ledcWrite(MOTOR_PWM_CHANNEL, 0);
}

void aplicarPID() {
  if (!bnoListo) { myservo.write(ANGULO_CENTRO); avanzarMotor(); return; }
  float anguloActual = getAnguloRelativo();
  float error = anguloReferencia - anguloActual;
  if (error > 180) error -= 360;
  if (error < -180) error += 360;
  if (abs(error) < DEAD_BAND) {
    errorAcumulado = 0; errorAnterior = 0;
    myservo.write(ANGULO_CENTRO);
    avanzarMotor();
    return;
  }
  unsigned long ahora = millis();
  float dt = (ahora - tiempoAnteriorPID) / 1000.0;
  if (dt <= 0) dt = 0.01;
  float P = KP * error;
  errorAcumulado += error * dt;
  errorAcumulado = constrain(errorAcumulado, -50, 50);
  float I = KI * errorAcumulado;
  float D = KD * (error - errorAnterior) / dt;
  float correccion = constrain(P + I + D, -LIMITE_CORRECCION, LIMITE_CORRECCION);
  int anguloServo = constrain(ANGULO_CENTRO + (int)correccion, ANGULO_IZQUIERDA, ANGULO_DERECHA);
  myservo.write(anguloServo);
  avanzarMotor();
  errorAnterior = error;
  tiempoAnteriorPID = ahora;
}

void resetearPID() {
  errorAcumulado = 0;
  errorAnterior = 0;
  tiempoAnteriorPID = millis();
}

void leerHusky() {
  if (millis() - lastHuskyRead < READ_INTERVAL) return;
  lastHuskyRead = millis();

  if (!sensorConectado) {
    sensorConectado = huskylens.begin(Serial2);
    if (sensorConectado) {
      huskylens.switchAlgorithm(ALGORITHM_COLOR_RECOGNITION);
      Serial.println("HuskyLens reconectado");
    }
    return;
  }

  int resultCount = huskylens.getResult(ALGORITHM_COLOR_RECOGNITION);
  bloqueDetectado = false;
  ID_detectado = 0;
  alturaBloque = 0;
  centroX = 0;
  posicionBloque = 0;

  if (resultCount > 0) {
    while (huskylens.available(ALGORITHM_COLOR_RECOGNITION)) {
      Result* result = huskylens.popCachedResult(ALGORITHM_COLOR_RECOGNITION);
      if (result == NULL) continue;

      int id = result->ID;
      int x = result->xCenter;
      int h = result->height;

      if (id == 0) continue;

      if ((id == ID_ROJO || id == ID_VERDE) && h > UMBRAL_ALTURA) {
        int pos = 0;
        if (x < MITAD_PANTALLA - UMBRAL_CENTRO) pos = -1;
        else if (x > MITAD_PANTALLA + UMBRAL_CENTRO) pos = 1;
        else pos = 0;

        bool posicionCorrecta = (id == ID_VERDE && pos == 1) || (id == ID_ROJO && pos == -1);
        if (posicionCorrecta) {
          Serial.printf("Bloque ID=%d | centroX=%3d | altura=%3d | POSICION: %s (CORRECTA, no se esquiva)\n",
                        id, x, h, (pos == -1) ? "IZQUIERDA" : (pos == 1) ? "DERECHA" : "CENTRO");
          continue;
        }

        if (!bloqueDetectado) {
          bloqueDetectado = true;
          ID_detectado = id;
          centroX = x;
          alturaBloque = h;
          posicionBloque = pos;

          String posStr = (posicionBloque == -1) ? "IZQUIERDA" :
                          (posicionBloque == 1) ? "DERECHA" : "CENTRO";
          Serial.printf("Bloque ID=%d | centroX=%3d | altura=%3d | POSICION: %s (INCORRECTA, esquiva)\n",
                        id, centroX, alturaBloque, posStr.c_str());
          Serial.println("   Bloque valido. Activando esquiva...");
        }
      }
    }
  } else if (resultCount < 0) {
    sensorConectado = false;
  }
}

void decidirDireccionEsquiva() {
  if (posicionBloque == -1) {
    direccionPrimer = 1;
    direccionContraria = -1;
    Serial.println("   Bloque a IZQUIERDA -> esquivando a DERECHA");
  } else if (posicionBloque == 1) {
    direccionPrimer = -1;
    direccionContraria = 1;
    Serial.println("   Bloque a DERECHA -> esquivando a IZQUIERDA");
  } else {
    if (ID_detectado == ID_VERDE) {
      direccionPrimer = 1;
      direccionContraria = -1;
      Serial.println("   Bloque en CENTRO (VERDE) -> esquivando a DERECHA (por defecto)");
    } else {
      direccionPrimer = -1;
      direccionContraria = 1;
      Serial.println("   Bloque en CENTRO (ROJO) -> esquivando a IZQUIERDA (por defecto)");
    }
  }

  if (ID_detectado == ID_VERDE) {
    ticksPrimer = TICKS_VERDE_PRIMER;
    ticksSegundo = TICKS_VERDE_SEGUNDO;
    ticksGiroContrario = TICKS_VERDE_GIRO_CONTRARIO;
    ticksGiroVuelta = TICKS_VERDE_GIRO_VUELTA;
    ticksRetrocesoVerif = TICKS_VERDE_RETROCESO_VERIF;
    Serial.println("   (VERDE) Ticks post-esquiva asignados");
  } else { // ROJO
    ticksPrimer = TICKS_ROJO_PRIMER;
    ticksSegundo = TICKS_ROJO_SEGUNDO;
    ticksGiroContrario = TICKS_ROJO_GIRO_CONTRARIO;
    ticksGiroVuelta = TICKS_ROJO_GIRO_VUELTA;
    ticksRetrocesoVerif = TICKS_ROJO_RETROCESO_VERIF;
    Serial.println("   (ROJO) Ticks post-esquiva asignados");
  }
}

void leerSensores() {
  distanciaIzquierda = USLEFT.ping_cm();
  distanciaDerecha = USRIGHT.ping_cm();
  if (distanciaIzquierda == 0) distanciaIzquierda = MAX_DISTANCIA;
  if (distanciaDerecha == 0) distanciaDerecha = MAX_DISTANCIA;
}

// ================================================================
// ===== SETUP ====================================================
// ================================================================
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== ROBOT - ESQUIVA DE BLOQUES + RETROCESO EVASIVO ===");
  Serial.printf("Velocidad avance: %d | Retroceso normal: %d | Retroceso evasivo: %d\n",
                VELOCIDAD_NORMAL, VELOCIDAD_REVERSA, VELOCIDAD_RETROCESO_EVASIVO);
  Serial.printf("Distancia frontal umbral: %d cm\n", DISTANCIA_FRONTAL_UMBRAL);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(PIN_BOTON, INPUT_PULLUP);
  ledcSetup(MOTOR_PWM_CHANNEL, 5000, 8);
  ledcAttachPin(MOTOR_PWM_PIN, MOTOR_PWM_CHANNEL);
  myservo.attach(PIN_SERVO);
  myservo.write(ANGULO_CENTRO);
  detenerMotor();
  iniciarEncoder();

  bnoListo = iniciarBNO055();
  if (!bnoListo) { Serial.println("BNO055 no detectado"); while(1); }
  anguloReferencia = getAnguloRelativo();
  Serial.printf("Referencia inicial: %.1f\n", anguloReferencia);

  Serial2.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
  sensorConectado = huskylens.begin(Serial2);
  if (sensorConectado) {
    huskylens.switchAlgorithm(ALGORITHM_COLOR_RECOGNITION);
    Serial.println("HuskyLens conectado - Modo Color Recognition");
  } else {
    Serial.println("HuskyLens no detectado");
  }

  tiempoAnteriorPID = millis();
  Serial.println("Presione el boton para iniciar...");
}

void loop() {
  if (!programaIniciado) {
    if (digitalRead(PIN_BOTON) == LOW) {
      programaIniciado = true;
      estado = BUSCANDO;
      faseEsquiva = 0;
      Serial.println("Iniciando!");
      delay(300);
    }
    return;
  }

  leerHusky();
  leerSensores();

  switch (estado) {

    case BUSCANDO: {
      aplicarPID();

      // 1. Prioridad: bloque en posición incorrecta y cerca
      if (bloqueDetectado) {
        Serial.printf("Esquivando bloque ID=%d\n", ID_detectado);
        decidirDireccionEsquiva();
        estado = ESQUIVANDO;
        faseEsquiva = 0;
        guardarTicksInicio();
        tiempoInicioFase = millis();
        if (direccionPrimer == 1) myservo.write(ANGULO_ESQUIVA_DER);
        else myservo.write(ANGULO_ESQUIVA_IZQ);
        break;
      }

      // 2. Obstáculo frontal + espacio lateral → retroceso evasivo
      int frontal = USFRONT.ping_cm();
      if (frontal == 0) frontal = MAX_DISTANCIA;

      if (frontal <= DISTANCIA_FRONTAL_UMBRAL) {
        bool izquierdaLibre = (distanciaIzquierda > DISTANCIA_LATERAL_GIRO);
        bool derechaLibre = (distanciaDerecha > DISTANCIA_LATERAL_GIRO);

        if (izquierdaLibre || derechaLibre) {
          int anguloGiro;
          if (derechaLibre) {
            anguloGiro = ANGULO_ESQUIVA_IZQ;   // girar a izquierda
            Serial.printf("Obstáculo frontal (%d cm) y espacio a DERECHA -> girando servo a IZQUIERDA y retrocediendo %d ticks (vel %d)\n",
                          frontal, TICKS_RETROCESO_EVASIVO, VELOCIDAD_RETROCESO_EVASIVO);
          } else {
            anguloGiro = ANGULO_ESQUIVA_DER;   // girar a derecha
            Serial.printf("Obstáculo frontal (%d cm) y espacio a IZQUIERDA -> girando servo a DERECHA y retrocediendo %d ticks (vel %d)\n",
                          frontal, TICKS_RETROCESO_EVASIVO, VELOCIDAD_RETROCESO_EVASIVO);
          }

          estado = RETROCESO_EVASIVO;
          faseEsquiva = 0;
          guardarTicksInicio();
          tiempoInicioFase = millis();
          myservo.write(anguloGiro);
          delay(50);
          retrocederEvasivo();   // Usa velocidad específica
          break;
        }
      }

      // Si no hay bloque ni obstáculo frontal, sigue avanzando (PID ya activo)
      break;
    }

    case RETROCESO_EVASIVO:
      if (alcanzoTicks(TICKS_RETROCESO_EVASIVO) || (millis() - tiempoInicioFase > 1500)) {
        detenerMotor();
        delay(50);
        myservo.write(ANGULO_CENTRO);
        // Reiniciar PID y actualizar referencia al ángulo actual
        resetearPID();
        anguloReferencia = getAnguloRelativo();
        estado = BUSCANDO;
        faseEsquiva = 0;
        Serial.println("Retroceso evasivo completado. Volviendo a BUSCANDO.\n");
        delay(200);
      }
      break;

    case ESQUIVANDO:
      // Fase 0: primer giro
      if (faseEsquiva == 0) {
        if (alcanzoTicks(ticksPrimer) || (millis() - tiempoInicioFase > 2000)) {
          Serial.printf("  Primer giro (%d ticks)\n", ticksPrimer);
          faseEsquiva = 1;
          guardarTicksInicio();
          tiempoInicioFase = millis();
          if (direccionPrimer == 1) myservo.write(ANGULO_ESQUIVA_IZQ);
          else myservo.write(ANGULO_ESQUIVA_DER);
        }
      }
      // Fase 1: segundo giro
      else if (faseEsquiva == 1) {
        if (alcanzoTicks(ticksSegundo) || (millis() - tiempoInicioFase > 2000)) {
          Serial.printf("  Segundo giro (%d ticks)\n", ticksSegundo);
          faseEsquiva = 2;
          myservo.write(ANGULO_CENTRO);
          detenerMotor();
          delay(100);
          guardarTicksInicio();
          tiempoInicioFase = millis();
          retroceder();   // Usa VELOCIDAD_REVERSA
          Serial.printf("  Retroceso de verificación (%d ticks)\n", ticksRetrocesoVerif);
        }
      }
      // Fase 2: retroceso de verificación
      else if (faseEsquiva == 2) {
        if (alcanzoTicks(ticksRetrocesoVerif) || (millis() - tiempoInicioFase > 1500)) {
          detenerMotor();
          delay(100);
          estado = AJUSTE_POST_ESQUIVA;
          faseEsquiva = 0;
          guardarTicksInicio();
          tiempoInicioFase = millis();
          myservo.write(ANGULO_CENTRO);
          avanzarMotor();
          Serial.printf("Iniciando avance recto de %d ticks (post-esquiva)\n", TICKS_AVANCE_POST_ESQUIVA);
        }
      }
      break;

    case AJUSTE_POST_ESQUIVA:
      // FASE 0: Avance recto (post-esquiva)
      if (faseEsquiva == 0) {
        if (alcanzoTicks(TICKS_AVANCE_POST_ESQUIVA) || (millis() - tiempoInicioFase > 1500)) {
          Serial.printf("  Avance recto completado (%d ticks)\n", TICKS_AVANCE_POST_ESQUIVA);
          faseEsquiva = 1;
          guardarTicksInicio();
          tiempoInicioFase = millis();
          if (direccionContraria == 1) myservo.write(ANGULO_ESQUIVA_DER);
          else myservo.write(ANGULO_ESQUIVA_IZQ);
          avanzarMotor();
          Serial.println("  Iniciando giro contrario (post-esquiva)");
        }
      }
      // FASE 1: Giro contrario
      else if (faseEsquiva == 1) {
        if (alcanzoTicks(ticksGiroContrario) || (millis() - tiempoInicioFase > 1500)) {
          Serial.printf("  Giro contrario completado (%d ticks)\n", ticksGiroContrario);
          faseEsquiva = 2;
          guardarTicksInicio();
          tiempoInicioFase = millis();
          if (direccionPrimer == 1) myservo.write(ANGULO_ESQUIVA_DER);
          else myservo.write(ANGULO_ESQUIVA_IZQ);
          avanzarMotor();
          Serial.println("  Iniciando giro de vuelta al centro");
        }
      }
      // FASE 2: Giro de vuelta
      else if (faseEsquiva == 2) {
        if (alcanzoTicks(ticksGiroVuelta) || (millis() - tiempoInicioFase > 1500)) {
          Serial.printf("  Giro de vuelta completado (%d ticks)\n", ticksGiroVuelta);
          myservo.write(ANGULO_CENTRO);
          detenerMotor();

          // Reiniciar PID y actualizar referencia al ángulo actual
          resetearPID();
          anguloReferencia = getAnguloRelativo();
          estado = BUSCANDO;
          faseEsquiva = 0;
          Serial.println("Post-esquiva completada. Continuando navegación.\n");
          delay(200);
        }
      }
      break;

    case DETENIDO_PERMANENTE:
      detenerMotor();
      break;
  }

  delay(5);
}
