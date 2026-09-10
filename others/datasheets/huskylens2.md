# Ficha Técnica / Datasheet – DFRobot HuskyLens 2 (Módulo de Visión por IA)

## 1. Descripción General
La **HuskyLens 2** es una cámara de visión artificial autónoma y acelerada por hardware orientada a procesamiento *Edge AI*. Integra un procesador de redes neuronales (KPU) que ejecuta algoritmos de visión por computador directamente en el dispositivo, transmitiendo únicamente resultados procesados (coordenadas, áreas, identificadores) al microcontrolador anfitrión mediante interfaces de comunicación seriales o de bus.

---

## 2. Principio de Funcionamiento y Procesamiento Optoelectrónico

### 2.1 Captura y Mapeo Óptico
El sensor de imagen CMOS convierte la luz enfocada por la lente en señales digitales de píxeles, entregando un flujo de video interno al procesador principal.

### 2.2 Aceleración por Hardware (KPU)
A diferencia de los sistemas basados en software convencional, la HuskyLens 2 utiliza una **Unidad de Procesamiento KPU** (acelerador matricial). La KPU ejecuta capas convolucionales de redes neuronales en paralelo sobre el búfer del frame de entrada, extrayendo características espaciales y vectores geométricos sin requerir procesamiento por parte de un microcontrolador externo.

### 2.3 Protocolo de Salida Asíncrono
Una vez que el algoritmo en ejecución procesa el fotograma, emite un paquete estructurado de datos (en formato binario o texto delimitado) por la interfaz UART o I2C conteniendo la información de los objetos detectados (*Bounding Boxes*, centroides o vectores).

---

## 3. Especificaciones Técnicas Principales

| Parámetro | Valor / Rango | Unidad |
| :--- | :--- | :--- |
| **Procesador Principal** | Kendryte K210 (Dual-Core RISC-V 64-bit @ 400 MHz) | - |
| **Acelerador de IA** | KPU integrador de convolución matricial | - |
| **Sensor de Imagen** | OV2640 (2.0 Megapíxeles) | - |
| **Pantalla Integrada** | IPS de 2.0 pulgadas | px |
| **Resolución de Pantalla** | 320 x 240 | - |
| **Voltaje de Alimentación (VCC)** | 3.3 – 5.0 | V DC |
| **Consumo de Corriente (Operativo)** | 320 – 420 | mA |
| **Temperatura de Operación** | -10 a +60 | °C |
| **Nivel Lógico I/O** | 3.3 (Tolerante a 5V en líneas de comunicación) | V |
| **Dimensiones Físicas** | 52.0 x 44.5 x 19.0 | mm |

---

## 4. Algoritmos Integrados y Teoría Operativa

### 4.1 Clasificación y Reconocimiento de Objetos (Object Recognition)
* **Principio:** Utiliza una red neuronal convolucional (CNN) entrenada para detectar la presencia de clases de objetos específicas dentro del marco óptico.
* **Datos de Salida:**
  * Coordenadas de marco delimitador: $X_{min}$, $Y_{min}$, $X_{max}$, $Y_{max}$ (en píxeles).
  * ID asignado a la clase detectada.
  * Porcentaje de probabilidad/confianza de la detección.

### 4.2 Tracking y Vectorización de Líneas (Line Tracking)
* **Principio:** Evalúa diferencias de contraste y gradientes cromáticos para aislar patrones continuos y estimar la trayectoria de un carril o línea.
* **Datos de Salida:**
  * Punto inicial del vector: $(X_1, Y_1)$.
  * Punto final del vector: $(X_2, Y_2)$.
  * Ángulo de inclinación del vector ($\theta$) respecto a la vertical del marco.

### 4.3 Discriminación de Color (Color Recognition)
* **Principio:** Transforma el espacio de color RGB de la imagen al espacio de color HSV/LAB, permitiendo filtrar variaciones de iluminancia y aislar rangos de matiz (*hue*) y saturación (*saturation*).
* **Datos de Salida:**
  * Coordenadas del centroide del objeto de color: $(X_{c}, Y_{c})$.
  * Área del bloque de color (píxeles totales acumulados).
  * ID numérico asignado a la paleta de color aprendida.

### 4.4 Detección de Marcadores Fiduciales y AprilTags
* **Principio:** Realiza un proceso de umbralización adaptativa (*adaptive thresholding*) y análisis de cuadrantes binarios para descodificar la matriz de información de la etiqueta.
* **Datos de Salida:**
  * Identificador numérico único encodificado en la matriz.
  * Centroide $2D$ del marcador en pantalla.
  * Estimación de la matriz de pose $3D$ (rotaciones en yaw, pitch y roll respecto al plano focal).

### 4.5 Reconocimiento Facial (Face Recognition)
* **Principio:** Extrae un mapa de rasgos faciales (*facial landmarks*) combinando localización de puntos clave (ojos, nariz, comisura labial) y extracción de incrustaciones (*embeddings*).
* **Datos de Salida:**
  * Bounding box del rostro detectado.
  * ID de coincidencia almacenado en la memoria de usuario.

---

## 5. Descripción de Interfaces y Pinout

El puerto de conexión físico de 4 pines soporta multiplexación interna para UART e I2C.

| Pin N° | Etiqueta | Tipo | Función en Modo UART | Función en Modo I2C |
| :---: | :--- | :--- | :--- | :--- |
| **1** | VCC | Alimentación | 3.3V - 5V DC entrada de potencia | 3.3V - 5V DC entrada de potencia |
| **2** | GND | Tierra | Tierra común de referencia | Tierra común de referencia |
| **3** | TX / SDA | Salida / I/O | Transmisión de datos Serie (3.3V) | Línea de Datos I2C (SDA) |
| **4** | RX / SCL | Entrada | Recepción de datos Serie (3.3V) | Línea de Reloj I2C (SCL) |

### Parámetros de Comunicación
* **UART por Defecto:** Baudrate 9600 bps, 8 bits de datos, sin paridad, 1 bit de parada (9600 8N1). Configurable hasta 115200 bps.
* **I2C por Defecto:** Dirección esclava de bus `0x32`.