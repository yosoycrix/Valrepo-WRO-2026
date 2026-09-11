# Modelos del Robot – Carpeta `models` (Diseño 2025 - Descartado)

¡Bienvenido a la carpeta de modelos!
Aquí encontrarás todos los archivos y documentación relacionados con los componentes físicos y mecánicos del robot del año 2025, especialmente aquellos diseñados para impresión 3D.

---

> [!CAUTION]
> ### ⚠️ Estado del Proyecto: Diseño Descartado
> Los modelos contenidos en esta carpeta corresponden a la **versión 2025** y han sido **descartados para el desarrollo actual** debido a ineficiencias mecánicas y problemas de rendimiento detectados durante las pruebas en pista:
> 
> * **Juego Mecánico en Dirección:** El sistema de cruce y dirección delantera presentaba holguras/juegos excesivos, provocando imprecisión en los virajes.
> * **Desplazamiento del Servomotor:** El esfuerzo ejercido por el mecanismo de dirección hacía que el servomotor de dirección sufriera desplazamientos e inestabilidad en su soporte.
> * **Pérdida de Tracción:** El diseño del chasis y la distribución de masa ocasionaban que las ruedas patinaran constantemente, afectando la repetibilidad de la odometría por encoders.
> 
> *Este directorio se conserva exclusivamente como referencia histórica y registro de iteraciones de diseño.*

---

## Estructura de Carpetas y Descripción

A continuación se detallan las subcarpetas principales que conforman este directorio, junto a una breve descripción de su propósito:

---

### 1. **Bases y Acoples de Sensores y Cámara**
- **Descripción:**  
  Aquí se encuentran los modelos 3D de las bases y acoples para la fijación de sensores y la cámara del robot. 

---

### 2. **Cajas de Diferencial y Chasis Principal**
- **Descripción:**  
  Contiene los archivos de diseño para la estructura principal del robot, incluyendo el chasis central y las cajas del diferencial. Estos modelos brindan el soporte estructural necesario y alojan los mecanismos internos de transmisión y control.

---

### 3. **Impresión y Diseño 3D**
- **Descripción:**  
  Reúne los archivos fuente (STL, STEP, etc.) y documentación relacionada con el proceso de diseño e impresión 3D. Aquí se incluyen guías para la fabricación, recomendaciones de materiales, y configuraciones de impresión.

---

### 4. **Sistema de Movimiento Libre y Dirección Delantera**
- **Descripción:**  
  Incluye los modelos para los sistemas delanteros de dirección y movimiento libre. Aquí se encuentran los componentes que permiten al robot girar y maniobrar con precisión *(Afectado por el juego mecánico mencionado)*.

---

### 5. **Sistema de Transmisión Motriz Trasero**
- **Descripción:**  
  Contiene los diseños de la transmisión motriz trasera, tales como soportes de motores, engranajes, ejes y acoples. Estos elementos son fundamentales para el desplazamiento y la entrega de potencia a las ruedas traseras *(Afectado por patinado de ruedas)*.

---

> [!NOTE]
> ### ¿Cómo usar estos modelos?
> 1. **Revisa el README específico de cada subcarpeta** para detalles técnicos y notas de montaje de esta versión previa.
> 2. **Descarga los archivos de la pieza que necesites** si deseas analizar el histórico de diseño o reutilizar acoples secundarios.
> 3. **Consulta las guías y diagramas** para estudiar las iteraciones anteriores y evitar repetir estos fallos mecánicos en las nuevas versiones del robot.