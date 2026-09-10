# Ficha Técnica / Datasheet – Servomotor Digital INJORA INJ2065 (11g / 7kg.cm)

## 1. Descripción General
El **INJORA INJ2065** es un servomotor digital de tamaño micro diseñado para aplicaciones de alta densidad de potencia donde se requieren tiempos de respuesta mínimos, alto par de retención y alta resistencia al estrés mecánico. Incorpora un motor de tecnología *coreless*, caja de engranajes de aleación metálica y disipador térmico de aluminio integrado en la carcasa central.

---

## 2. Principio de Funcionamiento y Dinámica Electromecánica

### 2.1 Motor Coreless (Sin Núcleo de Hierro)
A diferencia de los motores DC tradicionales donde las bobinas están devanadas sobre un núcleo de hierro laminado, el motor *coreless* posee un devanado autosoportado cilíndrico de cobre que gira alrededor de un imán permanente central de alta intensidad.
* **Efecto Físico:** Elimina el momento de inercia asignado por la masa del hierro (*rotor cogging*). Esto se traduce en aceleraciones ultrarrápidas, respuesta dinámica instantánea y menor resistencia inductiva al cambio de sentido de giro.

### 2.2 Microcontrolador de Control Digital Integrado
El circuito de control lee la señal de entrada PWM proveniente del dispositivo anfitrión y procesa la diferencia angular a través de un microcontrolador interno a alta frecuencia (frecuencias de actualización internas >300 Hz). 
* **Banda Muerta (Deadband):** Al reducir la banda muerta a un nivel de 2 µs, el controlador reacciona ante variaciones de posición imperceptibles, aplicando pequeños pulsos de corriente para corregir y mantener la posición fijada.

### 2.3 Bucle Cerrado de Posición (Closed-Loop)
Un potenciómetro de alta precisión acoplado al eje de salida convierte el ángulo actual en un nivel de voltaje analógico. El controlador compara este valor en tiempo real con la posición solicitada mediante la señal PWM, ajustando dinámicamente la corriente aplicada por el puente H interno hacia el motor *coreless*.

---

## 3. Especificaciones Eléctricas y Mecánicas

### 3.1 Rendimiento Operativo

| Parámetro | Condición a 6.0V | Condición a 7.4V | Unidad |
| :--- | :--- | :--- | :--- |
| **Par de Bloqueo (Stall Torque)** | 6.0 (0.58) | 7.0 (0.68) | kg/cm (N.m) |
| **Velocidad Operativa (sin carga)** | 0.09 | 0.07 | sec / 60° |
| **Corriente en Vacío (No load)** | 50 | 60 | mA |
| **Corriente de Bloqueo (Stall Current)**| 1200 | 1500 | mA |
| **Banda Muerta (Deadband Width)** | 2 | 2 | µs |

### 3.2 Especificaciones Mecánicas y Constructivas

| Parámetro | Valor / Características |
| :--- | :--- |
| **Dimensiones Exteriores** | 23.0 mm x 12.5 mm x 26.5 mm |
| **Peso Total** | 11.0 gramos |
| **Material de Piñonería** | Acero Inoxidable / Aleación de Cobre |
| **Material de la Carcasa** | Polímero de alta resistencia con sección media de Aluminio CNC |
| **Sistema de Rodamientos** | Doble rodamiento de bolas (*Dual Ball Bearings*) |
| **Eje de Salida (Spline)** | 25T (Micro estrías) |

---

## 4. Señal de Control PWM y Protocolo

El servomotor se gobierna mediante la modulación del ancho de pulso de una señal cuadrada estándar.

* **Frecuencia de Cuadro:** 50 Hz a 330 Hz (Soporta tasas elevadas por ser de arquitectura digital).
* **Rango de Ancho de Pulso:** 500 µs a 2500 µs.
  * **500 µs:** Posición límite angular $0^\circ$.
  * **1500 µs:** Posición neutra / central $90^\circ$.
  * **2500 µs:** Posición límite angular $180^\circ$.

---

## 5. Mapeo de Conexión Eléctrica

| Color del Cable | Asignación | Descripción de Función |
| :---: | :--- | :--- |
| **Marrón / Negro** | GND | Tierra lógica y de potencia de referencia. |
| **Rojo** | VCC | Entrada de potencia (6.0V a 7.4V DC). |
| **Naranja / Blanco**| Signal | Entrada de la señal de control PWM. |