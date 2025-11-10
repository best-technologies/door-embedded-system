# Smart Door Access System

## Architecture Overview

### Three-Device Setup:
1. **On-door ESP32**: Handles authentication (fingerprint, RFID, password) + TFT interface
2. **Controller ESP32**: Controls magnetic lock via relay
3. **Raspberry Pi 4**: Hosts web application and database

### Communication Flow:
- On-door ESP32 ↔ Controller ESP32 (ESP-NOW)
- On-door ESP32 ↔ Raspberry Pi (WiFi/HTTP API)
- Admin Laptop ↔ Raspberry Pi (WiFi/Web Browser)

## Hardware Requirements

### On-door ESP32:
- ESP32 DevKit v1
- 4.0" TFT 480x320 display with touch (ILI9488)
- Fingerprint sensor (R307/AS608)
- RFID reader (MFRC522)
- Buzzer
- Power supply

### Controller ESP32:
- ESP32 DevKit v1
- Relay module (5V)
- Magnetic lock (12V)
- Power supply

### Raspberry Pi 4:
- Raspberry Pi 4 (2GB+)
- MicroSD card (32GB+)
- Power supply
- Network connection

## Software Components

### ESP32 Code:
- `door_system.cpp` - Main door authentication system
- `controller_esp32.cpp` - Lock controller
- `main.cpp` - Display test code

### Raspberry Pi:
- `app.py` - Flask web application
- `dashboard.html` - Admin web interface
- SQLite database for user management

## Build Instructions

### For Door ESP32:
```bash
pio run -e door_esp32 --target upload
```

### For Controller ESP32:
```bash
pio run -e controller_esp32 --target upload
```

### For Display Test:
```bash
pio run -e display_test --target upload
```

### For Raspberry Pi:
```bash
cd raspberry_pi
pip install flask
python app.py
```

## Configuration

### WiFi Settings (door_system.cpp):
```cpp
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* pi_server = "http://192.168.1.100:5000";
```

### Controller MAC Address:
Update `controllerMAC[]` in door_system.cpp with actual MAC address from controller ESP32.

## Pin Assignments

### Door ESP32:
- TFT CS: GPIO 5
- TFT DC: GPIO 2
- TFT RST: GPIO 4
- Touch CS: GPIO 21
- Touch IRQ: GPIO 22
- RFID SS: GPIO 15
- RFID RST: GPIO 0
- Fingerprint RX: GPIO 16
- Fingerprint TX: GPIO 17
- Buzzer: GPIO 26

### Controller ESP32:
- Relay: GPIO 2
- Status LED: GPIO 4

## API Endpoints

### Authentication:
- POST `/api/authenticate` - Verify user access
- POST `/api/attendance` - Log access events

### User Management:
- GET `/api/users` - List all users
- POST `/api/users` - Add new user
- POST `/api/temp_password` - Generate temporary password

## Features

### Authentication Methods:
- Fingerprint recognition
- RFID card access
- Temporary password entry
- Emergency unlock

### Admin Features:
- Web-based user management
- Real-time attendance logging
- Temporary password generation
- System status monitoring

### Security Features:
- ESP-NOW encrypted communication
- Database-backed user verification
- Attendance audit trail
- Emergency access logging