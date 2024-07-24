// #include <Arduino.h>
// #include <WiFi.h>
// #include <PubSubClient.h>
// #include <Wire.h>
// #include <LiquidCrystal_I2C.h>
// #include "sensor_HX711.h"

// // Inisialisasi objek load cell dan LCD
// sensor_HX711 loadCell(4, 5);
// LiquidCrystal_I2C lcd(0x27, 16, 2);

// // Konfigurasi WiFi dan MQTT
// const char* ssid = "your_ssid";
// const char* password = "your_password";
// const char* mqtt_server = "your_mqtt_server";
// WiFiClient espClient;
// PubSubClient client(espClient);

// // Variabel untuk timer dan status kalibrasi
// unsigned long calibrationStartTime = 0;
// bool isCalibrating = false;
// float knownWeight = 0.0;

// void setup_wifi()
// {
//     delay(10);
//     Serial.println();
//     Serial.print("Connecting to ");
//     Serial.println(ssid);
//     WiFi.begin(ssid, password);

//     while (WiFi.status() != WL_CONNECTED)
//     {
//         delay(500);
//         Serial.print(".");
//     }
//     Serial.println("");
//     Serial.println("WiFi connected");
//     Serial.println("IP address: ");
//     Serial.println(WiFi.localIP());
// }

// void parseMessage(const String& message, String parsedData[])
// {
//     int start = 0;
//     int end = message.indexOf(',');

//     for (int i = 0; i < 3; i++)
//     {
//         if (end == -1)
//         {
//             parsedData[i] = message.substring(start);
//         }
//         else
//         {
//             parsedData[i] = message.substring(start, end);
//             start = end + 1;
//             end = message.indexOf(',', start);
//         }
//     }
// }

// void callback(char* topic, byte* payload, unsigned int length)
// {
//     String message;
//     for (int i = 0; i < length; i++)
//     {
//         message += (char)payload[i];
//     }
//     Serial.print("Message arrived [");
//     Serial.print(topic);
//     Serial.print("] ");
//     Serial.println(message);

//     if (String(topic) == "sensor/kalibrasi")
//     {
//         // Parsing data dari pesan
//         if (message.startsWith("*") && message.endsWith("#"))
//         {
//             message = message.substring(1, message.length() - 1); // Hilangkan karakter '*' dan '#'
//             String parsedData[3];
//             parseMessage(message, parsedData);

//             if (parsedData[0] == "003" && parsedData[1] == "kalibrasi")
//             {
//                 knownWeight = parsedData[2].toFloat();

//                 lcd.clear();
//                 lcd.setCursor(0, 0);
//                 lcd.print("Letakkan benda");
//                 lcd.setCursor(0, 1);
//                 lcd.print("dengan berat yg");
//                 lcd.setCursor(0, 2);
//                 lcd.print("diketahui diatas");

//                 calibrationStartTime = millis();
//                 isCalibrating = true;
//             }
//         }
//     }
// }

// void reconnect()
// {
//     while (!client.connected())
//     {
//         Serial.print("Attempting MQTT connection...");
//         if (client.connect("ESP32Client"))
//         {
//             Serial.println("connected");
//             client.subscribe("sensor/kalibrasi");
//         }
//         else
//         {
//             Serial.print("failed, rc=");
//             Serial.print(client.state());
//             Serial.println(" try again in 5 seconds");
//             delay(5000);
//         }
//     }
// }

// void setup()
// {
//     Serial.begin(115200);
//     lcd.init();
//     lcd.backlight();

//     setup_wifi();
//     client.setServer(mqtt_server, 1883);
//     client.setCallback(callback);

//     if (loadCell.begin())
//     {
//         Serial.println("Load cell initialized successfully.");
//     }
//     else
//     {
//         Serial.println("Failed to initialize load cell.");
//         while (1);
//     }
// }

// void loop()
// {
//     if (!client.connected())
//     {
//         reconnect();
//     }
//     client.loop();

//     if (isCalibrating)
//     {
//         if (millis() - calibrationStartTime >= 10000) // Tunggu 10 detik
//         {
//             loadCell.calibrate(knownWeight);
//             lcd.clear();
//             lcd.setCursor(0, 0);
//             lcd.print("Kalibrasi selesai");
//             isCalibrating = false;
//         }
//     }
//     else
//     {
//         float weight = loadCell.getLoad();
//         Serial.print("Weight: ");
//         Serial.print(weight);
//         Serial.println(" grams");

//         lcd.setCursor(0, 0);
//         lcd.print("Berat: ");
//         lcd.print(weight);
//         lcd.print(" gram");
//     }

//     delay(100); // Tunggu sebentar sebelum iterasi berikutnya
// }
