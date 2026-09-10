# Ficha Técnica / Datasheet – Motor DC Hytech GA37-520 con Encoder Magnético de Cuadratura

## 1. Descripción General
El **Hytech GA37-520** es un sistema electromecánico integrado compuesto por un motor de corriente continua, una caja de engranajes reductora de perfil cilíndrico (37 mm) y un encoder incremental magnético de efecto Hall montado sobre el eje posterior del rotor. Proporciona alto par rotacional y capacidad de monitoreo de velocidad/posición en bucle cerrado.

---

## 2. Principios Físicos de Funcionamiento

### 2.1 Conversión Electromecánica (Motor DC)
El motor opera bajo el principio de la fuerza de Lorentz ($F = I \cdot L \times B$). Cuando la corriente fluye a través de las escobillas hacia el colector y los devanados del inducido, la interacción con el campo magnético del estator permanente genera un par de torsión que provoca la rotación del rotor.
* **Control de Velocidad:** La velocidad es proporcional a la tensión media aplicada mediante modulación por ancho de pulso (PWM).
* **Control de Par:** El par electromecánico es directamente proporcional a la corriente eléctrica consumida por el motor ($\tau = K_t \cdot I$).

### 2.2 Reducción Mecánica de Velocidad (Spur Gearbox)
La caja de engranajes cilíndrica utiliza trenes de ruedas dentadas montadas en ejes paralelos para modificar la relación velocidad/par entre el eje del motor y el eje de salida final.
* **Ecuaciones de Conversión Mecánica:**
  $$\omega_{salida} = \frac{\omega_{rotor}}{i}$$
  $$\tau_{salida} = \tau_{rotor} \cdot i \cdot \eta$$
  *(donde $i$ es la relación de reducción de la caja y $\eta$ es la eficiencia de transferencia mecánica).*

### 2.3 Encoder Incremental de Cuadratura (Efecto Hall)
El módulo de lectura consta de un disco magnético multipolar fijo al eje del rotor y dos sensores de Efecto Hall integrados en la placa posterior colocados con un desfase espacial de $90^\circ$ eléctricos.

* **Generación de Onda:** Al girar el disco magnético, los cambios de flujo activan y desactivan los transistores del sensor de Hall, generando dos señales cuadradas digitales (Canal A y Canal B).
* **Determinación de Dirección:**
  * **Sentido Horario (CW):** El flanco de subida del **Canal A** precede al flanco de subida del **Canal B**.
  * **Sentido Antihorario (CCW):** El flanco de subida del **Canal B** precede al flanco de subida del **Canal A**.

---

## 3. Especificaciones Técnicas

### 3.1 Parámetros Eléctricos y Motrices (A 12V DC)

| Parámetro | Valor Típico | Unidad |
| :--- | :--- | :--- |
| **Voltaje Nominal Operativo** | 12.0 | V DC |
| **Rango de Tensión Permitido** | 6.0 – 15.0 | V DC |
| **Corriente en Vacío (No Load Current)** | $\le 0.15$ | A |
| **Corriente Típica Bajo Carga** | $0.8 – 1.4$ | A |
| **Corriente Máxima de Bloqueo (Stall Current)** | 3.2 | A |
| **Potencia Mecánica de Salida** | 10 – 15 | W |

### 3.2 Especificaciones del Encoder de Efecto Hall

| Parámetro | Valor | Unidad |
| :--- | :--- | :--- |
| **Tensión de Alimentación Lógica ($V_{cc}$)** | 3.3 – 5.0 | V DC |
| **Nivel Lógico de Salida** | TTL ($0V$ a $V_{cc}$) | - |
| **Pulsos por Revolución del Motor (PPR)** | 11 | PPR (Canal único) |
| **Cuentas por Revolución del Motor (CPR)** | 44 | CPR (En decodificación 4x) |
| **Frecuencia de Respuesta Máxima** | 100 | kHz |

*Nota: Para obtener el número total de pulsos por vuelta completa en el eje de salida final, se aplica la fórmula:*
$$\text{Pulsos Salida (PPR)} = 11 \times i$$
$$\text{Cuentas Salida (CPR)} = 44 \times i$$

---

## 4. Descripción del Conector y Pinout (6 Pines)

| Pin N° | Etiqueta | Función | Tipo de Señal / Conexión |
| :---: | :--- | :--- | :--- |
| **1** | M+ / Motor + | Alimentación (+) del Motor DC | Entrada de potencia (PWM / Tensión continua) |
| **2** | M- / Motor - | Alimentación (-) del Motor DC | Entrada de potencia (PWM / Tensión continua) |
| **3** | Hall GND | Tierra Lógica del Sensor | Tierra de referencia (0V) |
| **4** | Hall VCC | Alimentación Lógica Sensor | Entrada lógica regulada (3.3V – 5.0V DC) |
| **5** | Hall Out A | Salida Canal A del Encoder | Salida Digital TTL (Cuadratura A) |
| **6** | Hall Out B | Salida Canal B del Encoder | Salida Digital TTL (Cuadratura B) |