#include <Wire.h>
#include <NewPing.h>
#include <ESP32Servo.h>
#include <MPU6050_light.h>

#define USTFRONT 13
#define USEFRONT 12
#define USTLEFT 14
#define USELEFT 27
#define USTRIGHT 26
#define USERIGHT 25

#define MAX_DISTANCE 357

#define IN2 16
#define IN1 17
#define PIN_SERVO 2
#define PIN_BOTON 15

NewPing USFRONT(USTFRONT, USEFRONT, MAX_DISTANCE);
NewPing USLEFT(USTLEFT, USELEFT, MAX_DISTANCE);
NewPing USRIGHT(USTRIGHT, USERIGHT, MAX_DISTANCE);

Servo myservo;
MPU6050 mpu(Wire);

const int DISTANCIA_OBSTACULO_FRONTAL = 20;
const int DISTANCIA_OBSTACULO_LATERAL = 120;
const unsigned long TIEMPO_ESPERA_GIRO = 1000;
const int MAX_GIROS = 12;

bool programaIniciado = false;
bool finalizado = false;
bool girando = false;
bool enAvanceFinal = false;

unsigned long tiempoUltimoGiro = 0;
unsigned long tiempoInicioFinal = 0;
int contadorGiros = 0;

float angulof = 0;
float gyro = 0;

void motorAdelante() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
}

void motorParar() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
}

void servoCentro() {
  myservo.write(97);
}

int leerUltrasonico(NewPing &sensor) {
  int d = sensor.ping_cm();
  if (d == 0) return -1;
  return d;
}

void actualizarMPU() {
  mpu.update();
  gyro = mpu.getAngleZ();
}

void Adelante() {
  motorAdelante();
  servoCentro();
}

void Parar() {
  motorParar(); 
}

void iniciarGiroIzquierda() {
  girando = true;
  angulof += 90;
  myservo.write(120);
  motorAdelante();
}

void iniciarGiroDerecha() {
  girando = true;
  angulof -= 90;
  myservo.write(60);
  motorAdelante();
}

void actualizarGiro() {
  actualizarMPU();

  float error = angulof - gyro;
  if (error > 180) error -= 360;
  if (error < -180) error += 360;

  Serial.print("Gyro: ");
  Serial.print(gyro);
  Serial.print(" | Objetivo: ");
  Serial.print(angulof);
  Serial.print(" | Error: ");
  Serial.println(error);

  if (abs(error) <= 5) {
    motorParar();
    servoCentro();
    girando = false;
    contadorGiros++;
    tiempoUltimoGiro = millis();

    Serial.print("Giro completado. Conteo de giros: ");
    Serial.println(contadorGiros);

    if (contadorGiros >= MAX_GIROS) {
      enAvanceFinal = true;
      tiempoInicioFinal = millis();
    }
  }
}

void actualizarFinal() {
  motorAdelante();
  if (millis() - tiempoInicioFinal >= 700) {
    Parar();
    servoCentro();
    enAvanceFinal = false;
    finalizado = true;
    Serial.println("Se completaron 12 giros, robot detenido.");
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_BOTON, INPUT_PULLUP);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  myservo.attach(PIN_SERVO);
  servoCentro();

  Wire.begin();
  byte status = mpu.begin();
  while (status != 0) {
    Serial.println("Error MPU6050");
    delay(1000);
  }

  Serial.println("Calculando offsets, no mover el MPU...");
  delay(1000);
  mpu.calcOffsets();

  actualizarMPU();
  angulof = gyro;

  Parar();
  delay(3000);
  Serial.println("Esperando pulsar botón para iniciar...");
}

void loop() {
  if (!programaIniciado) {
    if (digitalRead(PIN_BOTON) == LOW) {
      programaIniciado = true;
      Serial.println("Botón presionado, iniciando programa...");
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

  Serial.print("Frontal: ");
  Serial.print(frontal);
  Serial.print(" | Izquierda: ");
  Serial.print(izquierda);
  Serial.print(" | Derecha: ");
  Serial.println(derecha);

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

  Serial.println("Sigue recto");
}
