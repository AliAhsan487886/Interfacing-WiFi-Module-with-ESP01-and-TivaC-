# Interfacing ESP-01 WiFi Module with Tiva C LaunchPad (TM4C123)

## 📌 Overview
This project demonstrates the interfacing of an **ESP-01 (ESP8266) WiFi module**
with the **Tiva C LaunchPad (TM4C123GH6PM)** using **UART communication**
and **AT commands**.

The system establishes a **Telnet (TCP) server** on the ESP module and allows
bidirectional data communication between:
- **Laptop-1 (via UART0 / USB)**
- **ESP-01 WiFi module (via UART2)**
- **Laptop-2 (via Telnet using PuTTY)**

This project was developed as part of **Open Ended Lab (OEL-1)**.

---

## 🎯 Objectives
- Interface ESP-01 with TM4C using UART
- Configure ESP-01 using AT commands
- Establish a TCP/Telnet server
- Send and receive data wirelessly
- Display ESP responses on a PC terminal

---

## 🧰 Hardware Requirements
- Tiva C LaunchPad (TM4C123GH6PM)
- ESP-01 (ESP8266) WiFi Module
- USB cable
- External 3.3V power supply for ESP-01
- Connecting wires

---

## 💻 Software Requirements
- Keil µVision / CCS
- PuTTY (for UART & Telnet)
- ESP8266 AT Firmware (v2.2.0 or compatible)

---

## 🔌 UART Configuration

| Module | UART | Pins |
|------|------|------|
| Laptop (USB) | UART0 | PA0 (RX), PA1 (TX) |
| ESP-01 | UART2 | PD6 (RX), PD7 (TX) |

- Baud Rate: **115200**
- Data Bits: **8**
- Parity: **None**
- Stop Bits: **1**

---

## ⚙️ AT Commands Used
```text
AT
AT+CWMODE=1
AT+CWJAP="SSID","PASSWORD"
AT+CIPMUX=1
AT+CIPSERVER=1,23
AT+CIFSR
