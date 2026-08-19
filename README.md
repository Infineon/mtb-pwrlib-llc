# ModusToolbox&trade; Power Suite LLC Middleware

## Overview

The LLC Middleware provides a universal LLC controller for building DC-DC power conversion applications based on the LLC resonant converter topology with ModusToolbox&trade;. It implements real-time firmware for controlling LLC resonant converters with closed-loop voltage regulation, switching frequency control, synchronous rectification, and automated system identification — enabling rapid development of efficient, reliable power supplies.

## Features

* Half-bridge and full-bridge LLC converter topologies
* Voltage, frequency, and duty-cycle control modes
* Finite state machine managing converter operation (Init, Soft-Start, Active Control, Fault, System ID)
* Synchronous rectification with 2D lookup-table-based timing
* Configurable dead-time control (percentage-based or LUT-based)
* Comprehensive fault protection (input/output OV/UV/OC, overtemperature, resonant current OC)
* Automated two-stage system identification (frequency and duty-cycle sweep)
* Dual-rate ISR architecture (fast + slow tasks)
* Biquad and notch filter signal conditioning
* Multi-core shared memory support
* Flash-based parameter storage with versioning

### When to Use

Use this middleware when developing LLC resonant converter power supplies targeting Infineon PSoC&trade; Control C3 microcontrollers. Typical applications include:

* Isolated DC-DC converters for server, telecom, or industrial power supplies
* Battery chargers requiring high-efficiency resonant conversion
* LED drivers and other constant-voltage/constant-current applications
* Prototyping and characterizing LLC converter designs using the built-in system identification feature

## Prerequisites

### Hardware Requirements

* An Infineon PSoC&trade; Control C3 microcontroller with hardware PWM, ADC, and comparator peripherals
* LLC resonant converter power stage (half-bridge or full-bridge topology)
* Gate driver circuitry with high-impedance (tri-state) capability
* Voltage and current sensing for input voltage, output voltage, output current, and resonant tank current
* Hardware comparators for fast overcurrent (resonant current) and overvoltage protection
* Temperature sensor (optional, for overtemperature protection)

### Software Requirements

* [ModusToolbox&trade; Software Environment](https://www.infineon.com/cms/en/design-support/tools/sdk/modustoolbox-software/) v3.8.0 (tested)
* GCC ARM Compiler v14.2.1 (tested)

### Read Documentation First

* [Power Suite LLC Firmware Reference Manual](../FirmwareReferenceManual.pdf)
* [Power Suite LLC Middleware Reference Manual](https://infineon.github.io/mtb-pwrlib-llc/html/index.html) — API reference and detailed usage
* [Power Suite Full-Bridge LLC Reference Code Example Documentation](https://infineon.github.io/mtb-example-pwrlib-llc/html/index.html) - The reference code example documentation
* [KIT-PSC3M5-CC1 User Guide](https://www.infineon.com/evaluation-board/KIT-PSC3M5-CC1) — Board setup and connections
* [KIT_PSC3_LLC1 User Guide](https://www.infineon.com/evaluation-board/KIT_PSC3_LLC1) — LLC power board setup
* [PSoC&trade; Control C3 Datasheet](https://www.infineon.com/products/microcontroller/32-bit-psoc-arm-cortex/32-bit-psoc-control-arm-cortex-m33-mcu/psoc-control-c3-main-line) — Hardware specifications and constraints


## Quick Start

The recommended and fastest way to start using this middleware is through the companion code example — [ModusToolbox&trade; Power Suite Full-Bridge LLC Reference Code Example](https://github.com/Infineon/mtb-example-pwrlib-llc). It provides a complete, ready-to-use application with all the HW and FW integration implemented for the supported kit.

### Library Dependencies

This middleware does not depend on any other ModusToolbox&trade; asset.

### Using the Code Example (Recommended)

See the correspondent section of the [Power Suite Full-Bridge LLC Reference Code Example Manual](https://infineon.github.io/mtb-example-pwrlib-llc/html/index.html)


## Troubleshooting

* **Converter stays in Init state:** Verify that hardware callbacks (`HW_FCN_t`) are correctly implemented and peripherals initialize without error. Check that the boot timer (`params.ctrl.boot_time`) is reasonable.
* **Soft-start fault:** Ensure the soft-start frequency (`params.ctrl.soft_start.f`) is within the safe operating range for your power stage. Verify gate driver connections and high-Z behavior.
* **Output voltage not regulating:** Check that the control mode is set to `Voltage_Mode` and that the PI controller bandwidth (`params.ctrl.vout.bw`) is appropriate for the converter dynamics.
* **Resonant current overcurrent fault:** The hardware comparator threshold may be too aggressive. Adjust `params.sys.fault.hw/sw.ires.thresh.max` or verify the current sensing gain.
* **Parameter fault on System ID:** System identification requires `Voltage_Mode`. Ensure `params.ctrl.mode == Voltage_Mode` before enabling System ID.

## Related Code Examples

* [ModusToolbox&trade; Power Suite Full-Bridge LLC Reference Code Example](https://github.com/Infineon/mtb-example-pwrlib-llc)

## Configuration Reference

### Control Modes

| Mode | Enum Value | Description |
|------|------------|-------------|
| Voltage Mode | `Voltage_Mode` | Closed-loop output voltage regulation via PI controller. Automatically selects the control variable based on operating point: varies switching frequency at full duty cycle in the high-voltage range, and varies duty cycle at minimum switching period in the low-voltage range |
| Frequency Mode | `Frequency_Mode` | Direct switching frequency command (open-loop) |
| Duty Cycle Mode | `Duty_Cycle_Mode` | Direct duty cycle command (open-loop) |

### Key Parameters (`PARAMS_t`)

| Parameter | Default | Description |
|-----------|---------|-------------|
| `llc.nom.type` | `Full_Bridge` | Converter topology |
| `llc.nom.vin` | 40 V | Nominal input voltage |
| `llc.nom.vout` | 5 V | Nominal output voltage |
| `llc.nom.pout` | 50 W | Nominal output power |
| `llc.circuit.Lr` | 2.6 µH | Series resonant inductance |
| `llc.circuit.Cr` | 36 nF | Series resonant capacitance |
| `llc.circuit.Lm` | 12.5 µH | Magnetizing inductance |
| `llc.circuit.n` | 7 | Transformer turns ratio |
| `sys.sample.fs0` | 50 kHz | Fast ISR countrol loop frequency |
| `sys.sample.fs0_fs1_ratio` | 5 | ISR0:ISR1 ratio (→ 10 kHz slow loop) |
| `ctrl.mode` | `Voltage_Mode` | Active control mode |
| `ctrl.vout.bw` | 500 Hz | Voltage controller bandwidth |
| `ctrl.soft_start.t` | 500 µs | Soft-start ramp time |
| `ctrl.boot_time` | — | Boot delay before soft-start |

### Fault Thresholds

| Fault | Type | Default Threshold | Debounce |
|-------|------|-------------------|----------|
| Input overvoltage | SW | 125% Vin nominal | 10 ms |
| Input undervoltage | SW | 75% Vin nominal | 10 ms |
| Output overvoltage | SW | 125% Vout nominal | 10 ms |
| Output overcurrent | SW | 125% Iout nominal | 10 ms |
| Overtemperature | SW | 110 °C | 400 ms |
| Resonant current OC | HW | 150% Ires nominal | 1 ms |
| Output overvoltage (fast) | HW | 150% Vout nominal | 1 ms |

## Compatible Software

| Software and Tools | Version |
|:---|:---:|
| ModusToolbox&trade; Software Environment | 3.8.0 |
| Power Suite Full-Bridge LLC Reference Code Example | 1.0.0 |
| GCC ARM Compiler | 14.2.1 |

## Changelog

 Version | Description of change
 ------- | ---------------------
 1.0.0   | Pre-production version of reference code example


## License

This software is provided under the Infineon End User License Agreement (EULA). Usage is limited to Infineon hardware products. Source code modification is permitted only for development on Infineon products. See the accompanying LICENSE file for full terms.

* [EULA](../../EULA) - End User License Agreement
* [LICENSE](../../LICENSE)

## Copyright

&copy; 2026, Infineon Technologies AG, or an affiliate of Infineon Technologies AG.
