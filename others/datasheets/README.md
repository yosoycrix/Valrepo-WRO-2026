# Datasheets – Referencias de Componentes de Hardware

Este directorio contiene las hojas de datos (*datasheets*) oficiales del fabricante y la documentación técnica de todos los componentes principales utilizados en el vehículo robótico **Heimdall**.

Estos documentos y fichas fueron consultados y utilizados durante:

* Diseño del esquema eléctrico y distribución de potencia.
* Desarrollo de firmware y controladores.
* Integración de sensores y calibración de visión.
* Control dinámico de motores y regulación de voltaje.
* Planificación de la arquitectura general del sistema.

---

## Componentes Incluidos

### Microcontrolador ESP32-WROOM-32
Unidad de procesamiento principal encargada del control en tiempo real, gestión de sensores, lógica de navegación y algoritmo de dirección.

**Archivo:**
* [<img src="https://img.shields.io/badge/Descargar_PDF-ESP32--WROOM--32D-red?style=flat-square&logo=adobeacrobatreader" />](./esp32-wroom-32_datasheet_en.pdf)

---

### Módulo Regulador Step-Down (Convertidor DC-DC Buck)
Módulo encargado de la regulación eficiente del voltaje proveniente de la batería, reduciendo y estabilizando la tensión a los niveles requeridos por la electrónica de control y sensores (5V / 3.3V).

**Archivo:**
* [<img src="https://img.shields.io/badge/Descargar_PDF-Step__Down__Converter-red?style=flat-square&logo=adobeacrobatreader" />](./LM2596%20SIMPLE%20SWITCHER®%20Power%20Converter%20150-kHz%203-A%20Step-Down%20Voltage%20Regulator%20datasheet%20(Rev.%20G)%20-%20lm2596.pdf)

---

### Driver de Motor / Puente H
Controlador de potencia encargado de gestionar el sentido de giro y la velocidad mediante señales PWM aplicadas al motor DC de tracción trasera.

**Archivo:**
* [<img src="https://img.shields.io/badge/Descargar_PDF-Puente__H__Driver-red?style=flat-square&logo=adobeacrobatreader" />](./L298N%20Motor%20Driver.pdf)

---

### Sensor IMU BNO055 (Bosch Sensortec)
Unidad de medición inercial de 9 ejes con fusión de datos integrada para la lectura precisa de orientación y aceleración.

**Archivo:**
* [<img src="https://img.shields.io/badge/Descargar_PDF-BNO055__Datasheet-red?style=flat-square&logo=adobeacrobatreader" />](./BNO055.pdf)

---

### Sensor de Distancia Ultrasónico HC-SR04
Arreglo de sensores para la detección periférica de obstáculos y cálculo de distancias por ultrasonido.

**Archivo:**
* [<img src="https://img.shields.io/badge/Descargar_PDF-HC--SR04__Datasheet-red?style=flat-square&logo=adobeacrobatreader" />](./HC-SR04%20-%20HCSR04.pdf)

---

### Cámara de Visión Artificial HuskyLens 2 (DFRobot)
Módulo de visión por IA dedicado al reconocimiento de colores, seguimiento de líneas, etiquetas y procesamiento de imágenes en tiempo real.

> [!NOTE]
> *Debido a la ausencia de un documento PDF oficial descargable por parte del fabricante, se redactó una ficha técnica detallada dentro del repositorio.*

**Documentación:**
* [<img src="https://img.shields.io/badge/Ver_Ficha_Técnica-HuskyLens_2-blue?style=flat-square&logo=markdown" />](./huskylens2.md)

---

### Servomotor INJORA INJ2065 (7 kg/cm)
Servomotor digital reforzado de perfil bajo y piñonería metálica, encargado del sistema de dirección articulada.

> [!NOTE]
> *Dado que el fabricante provee especificaciones directas de catálogo en lugar de un datasheet formal, se elaboró una guía técnica de referencia.*

**Documentación:**
* [<img src="https://img.shields.io/badge/Ver_Ficha_Técnica-Injora_INJ2065-blue?style=flat-square&logo=markdown" />](./injora2065.md)

---

### Motor con Reductora Hytech GA37-520 (12V con Encoder)
Motor DC de tracción trasera integrado con caja reductora y encoder de efecto Hall para el control de velocidad en bucle cerrado.

> [!NOTE]
> *Se sintetizaron los parámetros mecánicos, relación de reducción y diagrama del encoder en una ficha técnica Markdown dedicada.*

**Documentación:**
* [<img src="https://img.shields.io/badge/Ver_Ficha_Técnica-Hytech_GA37--520-blue?style=flat-square&logo=markdown" />](./hytech_ga37_520.md)

---

## Propósito de este Directorio

El análisis continuo de estas fichas técnicas asegura:

* **Niveles de voltaje correctos y aislamiento de potencia** para evitar daños en componentes de 3.3V / 5V / 12V.
* **Manejo térmico y de corriente adecuado** en el convertidor Step-Down y Puente H bajo cargas continuas.
* **Configuración precisa de tiempos e interrupciones** para buses de comunicación I2C, SPI y UART.
* **Ajuste fino de lectura de encoders** para el control PID del motor de tracción.

*Todas las decisiones de diseño e integración eléctrica de este proyecto se basan estrictamente en estos documentos técnicos.*