// #include <Arduino.h>
// #include <WiFi.h>
// #include "network_WiFi.h"
// #include "communication_mqtt.h"

// // Ganti dengan SSID dan Password WiFi Anda
// const char* ssid = "your_ssid";
// const char* pass = "your_password";

// // Ganti dengan detail server MQTT Anda
// const char* mqtt_server = "your_mqtt_server";
// const char* mqtt_clientID = "your_client_id";
// const char* mqtt_user = "your_mqtt_user"; // Opsional
// const char* mqtt_pass = "your_mqtt_pass"; // Opsional

// network_WiFi wifi(ssid, pass);
// communication_mqtt mqtt(mqtt_server, mqtt_clientID, mqtt_user, mqtt_pass);

// void setup() {
//     Serial.begin(115200);
//     if (wifi.init()) {
//         Serial.println("WiFi initialized and connected!");
//         if (mqtt.init() && mqtt.connect()) {
//             Serial.println("MQTT connected!");
//             mqtt.subscribe("test/topic");
//             mqtt.publish("test/topic", "Hello, MQTT!");
//         } else {
//             Serial.println("Failed to connect to MQTT.");
//         }
//     } else {
//         Serial.println("Failed to initialize WiFi.");
//     }
// }

// void loop() {
//     wifi.checkStatus();
//     mqtt.loop();
//     delay(5000); // Tambahkan delay sesuai kebutuhan
// }
