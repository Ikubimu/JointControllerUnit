# JointControllerUnit

Firmware para el control de una **articulación (joint) accionada por motor paso a paso** en una red de dispositivos CAN. Está pensado para un **STM32F103C8T6** (Blue Pill) y utiliza **FreeRTOS** como sistema operativo en tiempo real.

Cada unidad representa un *joint* dentro de un sistema multi-articulación (p. ej. un brazo robótico) gobernado por un maestro en el bus CAN. El firmware implementa control de movimiento, calibración, protecciones de seguridad y publicación de estado, todo controlado remotamente vía comandos CAN.

> **Nota de idioma:** el código y sus comentarios están en español/inglés mezclado. Este documento está en español.

---

## Tabla de contenidos

- [Características](#características)
- [Arquitectura](#arquitectura)
- [Hardware](#hardware)
  - [Mapa de pines](#mapa-de-pines)
- [Protocolo CAN](#protocolo-can)
  - [Encabezado de mensaje](#encabezado-de-mensaje)
  - [Comandos](#comandos)
  - [Errores](#errores)
  - [Mensajes de benchmarking](#mensajes-de-benchmarking)
- [Configuración y compilación](#configuración-y-compilación)
  - [Requisitos](#requisitos)
  - [Construir](#construir)
  - [Flashear](#flashear)
- [Tareas de FreeRTOS](#tareas-de-freertos)
- [Estructura del proyecto](#estructura-del-proyecto)
- [Consideraciones de diseño](#consideraciones-de-diseño)

---

## Características

- **Control de motor paso a paso** con microstepping (`1/2/4/8/16`) y rampa sigmoide de velocidad.
- **Retroalimentación por encoder** resuelto vía ADC (12 bits, 4095 cuentas/rev) y filtrada con un **filtro de Kalman**.
- **Calibración remota** por CAN: mapea la posición cruda del encoder a una posición deseada (offset corregido).
- **Protecciones de seguridad** integradas: error de encoder, colisión, stall / fallo de movimiento, fuera de rango y pérdida de comunicación con el maestro.
- **Protocolo CAN** con identificador por dispositivo y subcomando (compatible con redes multi-joint).
- **Benchmarking de RTOS**: mide el *timing* real de las tareas críticas y publica estadísticas por CAN.
- **Máquina de estados** para gestionar el ciclo de vida del dispositivo.

---

## Arquitectura

```
                    ┌─────────────────────────────────────────────┐
                    │                  FreeRTOS                    │
                    │                                              │
                    │  jointMain   ──►  StateMachine + CAN TX      │
                    │  Encoder     ──►  lectura ADC + Kalman +     │
                    │                   benchmark (100 Hz)         │
                    │  Control     ──►  bucle de control           │
                    │                   benchmark (50 Hz)          │
                    │  CAN         ──►  RX services + TX queue     │
                    └─────────────────────────────────────────────┘
                                       │
        ┌──────────────────────────────┼──────────────────────────────┐
        │                              │                              │
   CAN (500 kbit/s)             ADC · TIM1 (PWM)              GPIO (dirección,
   PA11 / PA12                  PA1 · TIM_CH1                 microstep, etc.)
```

El flujo de datos principal:

1. El **maestro** envía por CAN `CMD_CALIBRATION` o `CMD_MOVE_TARGET`.
2. **CommunicationHandler** recibe el mensaje y despacha a los *services* registrados.
3. `calibration()` reposiciona el offset del encoder; `Control::Move()` inicia un movimiento.
4. La tarea **Control** ejecuta la rampa de velocidad a 50 Hz y comanda el motor.
5. La tarea **Encoder** lee la posición real a 100 Hz y actualiza el filtro de Kalman.
6. `jointMain` publica estado periódicamente por CAN.

---

## Hardware

| Periférico | Uso |
|---|---|
| STM32F103C8T6 (Blue Pill) | MCU |
| Motor paso a paso (200 pasos/rev) + driver | Actuador |
| Driver con pines `MS1/MS2/MS3`, `DIR` | Microstepping y dirección |
| Encoder (potenciómetro/rotativo) leído por ADC | Retroalimentación de posición |
| CAN transceiver (p. ej. SN65HVD230) | Comunicación 500 kbit/s |
| UART (9 600 baud) | Depuración (`printf`) |

### Mapa de pines

| Pin | Función | Nota |
|---|---|---|
| `PA1` | `ADC1_IN1` | Lectura del encoder (12 bits) |
| `PA5` | `ADC1_IN5` | Entrada ADC secundaria (definida en CubeMX) |
| `PA8` | `TIM1_CH1` | PWM del motor |
| `PA2` | `TIM2_CH3` | PWM secundario (TIM2) |
| `PA11` / `PA12` | `CAN_RX` / `CAN_TX` | Bus CAN |
| `PA9` / `PA10` | `USART1_TX` / `USART1_RX` | Depuración |
| `PA3` | GPIO output | Salida genérica |
| `PA4` | GPIO input | Entrada genérica |
| `PB4`–`PB7` | GPIO input | Selección del **ID del joint** (4 bits) |
| `PB10` | GPIO output | `faultPIN` |
| `PB11` | GPIO output | `idlePIN` |
| `PB12` | GPIO output | `DIR` (dirección del motor) |
| `PB13`/`PB14`/`PB15` | GPIO output | `MS1` / `MS2` / `MS3` (microstepping) |
| `PC13` | GPIO output | LED de usuario |
| `PB0` | GPIO output | LED auxiliar (`ledPB0`) |

El ID del dispositivo se lee de `PB4`–`PB7` (`(GPIOB->IDR >> 4) & 0x0F`), obteniéndose un valor de 1 a 15 (`+1`). El **ID 0 está reservado para el maestro**.

---

## Protocolo CAN

Bus a **500 kbit/s**, formato estándar (11 bits). Toda comunicación usa identificadores con esta estructura:

### Encabezado de mensaje

```
          MSB ────────────────────────── LSB
          15            8  7           0
          │   joint_id     │   command   │
          └── dispositivo ─┘  └ subcomando┘
```

- **`joint_id`** (MSB): identificador de la articulación (`deviceId`), de 1 a 15.
- **`command`** (LSB): subcomando específico del dispositivo o del maestro.

Un mensaje **dirigido a un joint** lleva el `joint_id`; un **mensaje del maestro** al bus lleva `command` sin filtro de joint (se registra con `registerMasterService`).

### Comandos

| Cmd | Nombre | Dirección | Datos | Descripción |
|---|---|---|---|---|
| `0x01` | `CMD_ERROR` | Maestro → Bus | 1 byte | Notificación de error |
| `0x02` | `CMD_START` | Maestro → Joint | – | Habilita el registro de control |
| `0x03` | `CMD_CALIBRATION` | Maestro → Joint | `pos` u16, `ratio` i16, `pos_min` u16, `pos_max` u16 | Calibración de posición (÷100 para grados) |
| `0x04` | `CMD_ANNOUNCE` | Joint → Bus | – | Anuncio de presencia del dispositivo |
| `0x05` | `CMD_MOVE_TARGET` | Maestro → Joint | `pos` f32, `speed` f32 | Movimiento a una posición |
| `0x06` | `CMD_PAUSE` | Maestro → Joint | – | Pausa |
| `0x07` | `CMD_RESUME` | Maestro → Joint | – | Reanudar |
| `0x08` | `CMD_STATUS` | Joint → Maestro | `pos` f32, `speed` f32 | Publicación de estado |
| `0x09` | `CMD_WATCHDOG` | Maestro → Bus | – | Heartbeat del maestro |

> Los comandos de calibración y movimiento se manejan únicamente tras recibir `CMD_START`.

### Errores

`CMD_ERROR` (o `propagateError`) propaga un código de fallo. Los códigos definidos:

| Código | Nombre | Condición |
|---|---|---|
| `0x01` | `ENCODER_ERROR` | Delta de encoder demasiado brusco |
| `0x02` | `COLISION_ERROR` | El error no decrece (posible colisión) |
| `0x03` | `OUT_OF_RANGE_ERROR` | Posición fuera de `[pos_min, pos_max]` |
| `0x04` | `MOVE_FAILURE_ERROR` | Stall / fallo de movimiento |
| `0x05` | `CALIBRATION_FAILURE_ERROR` | Fallo de calibración |
| `0x06` | `COMMUNICATION_LOST_ERROR` | Se excede el timeout del maestro |
| `0xAA` | `UNKNOWN_ERROR` | Error no categorizado |

### Mensajes de benchmarking

Para depuración de timing RTOS, el dispositivo emite estadísticas sobre sus tareas periódicas:

| Cmd | Tarea | Contenido (6 bytes) |
|---|---|---|
| `0x15` (21) | Encoder | `min` u16, `max` u16, `avg` u16 |
| `0x16` (22) | Control | `min` u16, `max` u16, `avg` u16 |

Cada campo es el **periodo en ticks × 100** (2 decimales). Con `configTICK_RATE_HZ = 1000`:

- Encoder ideal ≈ `1000` → 10.00 ms (100 Hz)
- Control ideal ≈ `2000` → 20.00 ms (50 Hz)

Se envían cada vez que el *ring buffer* interno (128 muestras) se llena.

---

## Configuración y compilación

### Requisitos

- **ARM GNU Toolchain** (arm-none-eabi-gcc/g++). En este repo se referencia:
  `/opt/st/stm32cubeclt_1.19.0/GNU-tools-for-STM32/bin`
- `make`
- Hardware: Blue Pill + adaptador ST-Link para flashear.

Puedes ajustar `TOOLCHAIN_PATH` en el `Makefile` si tu toolchain está en otra ruta.

### Construir

```bash
make
```

Genera en `build/`:
- `JointControllerUnit.elf` — binario con símbolos (para debugger).
- `JointControllerUnit.bin` / `.hex` — para flashear.
- `JointControllerUnit.map` / `.lst` — mapas de memoria y listado de montaje.

Para limpiar:

```bash
make clean
```

### Flashear

Con **ST-Link** (via `st-flash` o STM32CubeProgrammer) usando el archivo `.bin`:

```bash
st-flash write build/JointControllerUnit.bin 0x08000000
```

O desde **STM32CubeProgrammer**:

```bash
STM32_Programmer_CLI -c port=SWD mode=UR -w build/JointControllerUnit.bin 0x08000000 -v -rst
```

> **Nota:** el proyecto también incluye `JointControllerUnit.ioc` y la subcarpeta `EWARM/` para abrirlo desde **STM32CubeIDE** / **IAR**. El `Makefile` es la vía de build independiente.

---

## Tareas de FreeRTOS

| Tarea | Prioridad | Stack (words) | Período | Descripción |
|---|---|---|---|---|
| `Control` | 2 | 256 | 20 ms | Bucle de control de movimiento (rampa sigmoide) |
| `joint_main` | 1 | 512 | 1000 ms | Estado, watchdog, publicación de estado |
| `Encoder` | 1 | 128 | 10 ms | Lectura de ADC, Kalman, protecciones de encoder |
| `CAN` | 1 | 256 | — | RX de servicios y TX de la cola de mensajes |

> `configTICK_RATE_HZ = 1000` en `FreeRTOS/FreeRTOSConfig.h`.

---

## Estructura del proyecto

```
JointControllerUnit/
├── Core/
│   ├── Inc/                      # Cabeceras de la aplicación
│   │   ├── LOW/                  #    wrappers de periféricos (ADC, PWM, CAN, DigitalOutput)
│   │   ├── StateMachine/         #    máquina de estados
│   │   └── benchmarker.hpp
│   └── Src/
│       ├── LOW/                  # implementación de periféricos bajo nivel
│       ├── StateMachine/
│       ├── main.cpp              # punto de entrada
│       ├── JointController.cpp   # inicialización + servicios CAN
│       ├── CommunicationHandler.cpp  # pila CAN + cola TX
│       ├── Motor.cpp             # motor + tarea de encoder + Kalman
│       ├── Control.cpp           # bucle de control
│       ├── Protections.cpp       # protecciones de seguridad
│       └── KalmanFilter.cpp      # filtro de Kalman
├── Drivers/                      # HAL + CMSIS de ST
├── FreeRTOS/                     # núcleo FreeRTOS + config
├── JointControllerUnit.ioc       # archivo de configuración CubeMX
├── STM32F103C8T6_FLASH.ld        # linker script
└── Makefile
```

---

## Consideraciones de diseño

### Calibración por offset

En lugar de escribir directamente la posición, `Motor::calibration()` calcula un **offset** entre la lectura actual del encoder y la posición deseada. La tarea de encoder suma ese offset en cada iteración, por lo que el valor calibrado se mantiene de forma persistente y robusta frente a lecturas posteriores.

### Benchmarking de timing

Los *ring buffers* recopilan el **periodo real** (en ticks) de las tareas `Encoder` y `Control`. Esto permite detectar, desde el maestro, desviaciones de jitter o carga excesiva de CPU que afecten al comportamiento en tiempo real.

### Filtro de Kalman

La velocidad cruda calculada por diferencia de posiciones es ruidosa. El `KalmanFilter` fusiona esa medición con un modelo de velocidad para suavizar la salida usada por la protección y el control.
