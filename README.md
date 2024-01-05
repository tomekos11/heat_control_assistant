1. Introduction
The "Heat Control Assistant" project aims to create a system for managing temperature in homes or offices. With its help, we can stop worrying about remembering to feed the boiler. This saves time and brain capacity for more important matters. The system's operation is simple and based on the methodology of gas boilers. The temperature is measured from the air, and based on that, along with the latest data on feeding, the system calculates the next feeding time and informs us about it. The information the system sends to users includes notifications about temperature increases and decreases. However, that's not all. The project also takes care of situations when another household member has taken care of it, and thanks to them, we don't have to worry for the next few hours.

2. Construction
2.1. Construction Description
The system is built around the Arduino UNO R3 microcontroller, which controls the temperature sensor, LEDs, infrared receiver, and display. Another essential element without which the project would not find practical use is the ESP32 microcontroller, which handles WiFi and Bluetooth. The microcontrollers communicate with each other through a serial connection. The Arduino RX pin is connected to the ESP TX pin, and the ESP RX pin is connected to the Arduino TX pin. This way, we get bidirectional communication between the devices. Therefore, Arduino's task is to monitor physical conditions and present them to the user who is near the device – this is done by LEDs and the display. Meanwhile, the ESP ensures that the user who is away from home or the office is informed about everything.

2.2. Specific Kit Components
Arduino UNO R3 Microcontroller: Responsible for controlling the temperature sensor, LEDs, infrared receiver, and display. It sends messages to ESP and receives responses from it.
ESP32 Microcontroller: Receives messages from Arduino UNO R3, reacts to them, and then sends responses.
DHT11 Temperature and Humidity Sensor Module: Responsible for measuring temperature and humidity.
Blue LED: Indicates temperature readings below 21 degrees Celsius, signaling that it's cold and needs refilling.
Red LED: Indicates moderate temperature, i.e., above 21 degrees Celsius.
Two 1kΩ Resistors: Reduce voltage on LEDs to prevent them from burning out.
2x16 Character Blue LCD Display: Displays information to the user.
Potentiometer: Adjusts the voltage supplied to the display.
Infrared Receiver: Reacts to button presses on the remote control, allowing communication with Arduino.
ELEGOO IR Remote: Used to send commands for Arduino execution.
830-Hole Breadboard: Allows for electronic circuit creation without soldering.
Male-to-Male and Male-to-Female Jumper Wires: Facilitate building circuits.
