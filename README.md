An ESP32-based autonomous robot for vertical pipe navigation using IR and ultrasonic sensing with a custom OLED UI.

📌 Features
Autonomous climb → ceiling detect → descend
Optional mid-hold at half-height (ROUND2)
Continuous buzzer during HOLD
Real-time OLED status display
Simple, reliable embedded logic
🛠️ Hardware
ESP32
L298N Motor Driver
SSD1306 OLED Display
IR sensor
Ultrasonic sensor (HC-SR04)
Buzzer, LEDs, Push buttons
🔌 Pin Configuration
IN1 → 19  
IN2 → 18  
PWM → 23  
IR → 33  
TRIG → 32  
ECHO → 25  
BUZZER → 4  
BTN_START → 27  
BTN_MODE → 26
⚙️ How It Works
Boot animation runs on OLED
Select mode (ROUND1 / ROUND2)
Press start → robot climbs
IR detects ceiling → short pause
Robot descends
(ROUND2) stops at half-height → HOLD
Stops near ground
▶️ Usage
Power the system
Select mode using MODE button
Press START to begin
Robot runs automatically
📷 Demo

Add images or video here

📈 Future Improvements
PID motor control
Battery monitoring
Wireless control (WiFi/Bluetooth)
Custom PCB
