# Component Test Suite

## Individual Component Tests

### TFT Display Test
```bash
pio run -e test_tft --target upload
```
- Tests display colors and touch functionality
- Touch screen to draw yellow circles
- Verifies 480x320 resolution and touch mapping

### Fingerprint Sensor Test
```bash
pio run -e test_fingerprint --target upload
```
- Tests fingerprint sensor communication
- Shows sensor parameters and status
- Place finger to test detection and matching

### RFID Reader Test
```bash
pio run -e test_rfid --target upload
```
- Tests RFID module communication
- Shows firmware version
- Present RFID card to read UID and type

### ESP-NOW Communication Test
```bash
pio run -e test_esp_now --target upload
```
- Tests wireless communication between ESP32s
- Type 'send' in serial monitor to transmit test message
- Update receiverMAC with actual controller MAC address

### Relay/Output Test
```bash
pio run -e test_relay --target upload
```
- Tests relay, LED, and buzzer outputs
- Serial commands: 'relay on/off', 'led on/off', 'buzzer', 'test'
- Verifies all output control functionality

### WiFi/API Test
```bash
pio run -e test_wifi_api --target upload
```
- Tests WiFi connection and Raspberry Pi API communication
- Update WiFi credentials and server IP
- Commands: 'auth', 'users', 'log'

## Pin Assignments (Same as main system)

### Display & Touch:
- TFT CS: GPIO 5
- TFT DC: GPIO 2  
- TFT RST: GPIO 4
- Touch CS: GPIO 21
- Touch IRQ: GPIO 22

### Sensors:
- RFID SS: GPIO 15
- RFID RST: GPIO 0
- Fingerprint RX: GPIO 16
- Fingerprint TX: GPIO 17

### Outputs:
- Relay: GPIO 2
- LED: GPIO 4
- Buzzer: GPIO 26

## Testing Workflow

1. **Start with TFT test** - Verify display works
2. **Test individual sensors** - RFID, fingerprint
3. **Test outputs** - Relay, LED, buzzer
4. **Test communication** - ESP-NOW, WiFi/API
5. **Integration testing** - Full system components

Each test is completely isolated and won't interfere with main source code.