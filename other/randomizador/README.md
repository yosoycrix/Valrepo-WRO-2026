# WRO Futuros Ingenieros 2026 - Randomizador Oficial Del Team Valgrind

Este es el randomizador oficial programado y utilizado por el **Team Valgrind** para poder simular y entrenar de una manera muchisimo mas efectiva los desafíos de la categoría **Future Engineers** (Futuros Ingenieros) de la **World Robot Olympiad 2026 (WRO)**. 

Este Randomizador permite llegar a generar de una manera visual la pista de una manera completamente aleatoria, asi tal cual la posición de los muros perimetrales, la ubicación de los bloques de tráfico (rojos y verdes) y las zonas de estacionamiento conforme a los reglamentos de la competencia.

🌐 **Link hacia el Randomizador:** [https://yosoycrix.github.io/wro-futuros-ingenieros-randomizador-team-valgrind/](https://yosoycrix.github.io/wro-futuros-ingenieros-randomizador-team-valgrind/)

---

## 🛠️ Características Principales

* **Selector de Modo de Ronda:** Alterna fácilmente entre el Desafío Abierto y el Desafío Cerrado.
* **Visualización Dinámica:** Renderizado en tiempo real de la posición del robot, dirección de marcha, bloques y muros de la sección central.
* **Cronómetro Integrado:** Permite medir el tiempo exacto de cada lanzamiento en minutos, segundos y milisegundos para llevar un control de velocidad.
* **Historial de Lanzamientos:** Un registro interactivo que guarda cada pista generada con un identificador único, marca de tiempo y detalles específicos.
* **Sistema de Filtros avanzados:** Filtra el historial por tipo de ronda o por cuadrante específico para analizar escenarios particulares.

---

## Estructura del Proyecto

```text
├── index.html       # Estructura de la interfaz de usuario
├── style.css        # Estilos modernos de la aplicación (Modo Oscuro)
├── script.js       # Lógica matemática, randomizador y control del flujo
└── field.png        # Imagen base del tapete oficial de la WRO
