// Konfigurasi
#pragma once

// Koneksi WiFi
#define AP_WIFI "Chicken-Feeder"
#define AP_PASSWORD "admin1234"

// OTA Connection
#define DEVICE_ID "CF1"
#define OTA_PORT 443
#define OTA_SERVER "angkasatimelapse.com"

// MQTT Connection
#define MQTT_SERVER "angkasatimelapse.com"
#define MQTT_CLIENTID "Poultry Feeding System"

// MQTT Topic
#define TOPIC_setSchedule "feeder/setschedule"
#define TOPIC_setPakan "feeder/setpakan"
#define TOPIC_sendData "feeder/sendData"
#define TOPIC_manual "feeder/manual"
#define TOPIC_response "fedeer/response"

// Spreadsheet Connectio
#define GAS "AKfycbwQHCQ-DtpNvTgnMncRBVDb3OHax9y0-V_NCsjvrUGooRNu4NUrNiFUrXldDauv4F0d"
#define host_gs "script.google.com"
#define https_port_gs 443

// Inisialisasi Pin Driver HX711
const int LOADCELL_DOUT_PIN = 18;
const int LOADCELL_SCK_PIN = 19;
// Nilai kalibrasi pengeluaran pakan per menit (2.2kg/m)
#define VAL_CALLIBRATION 2.2

// Inisialisasi Pin Stepper

// Mode Debug menggunakan LED
// #define DIR 17

// Menggunakan Stepper
#define DIR 26
#define STEP 25

// Settingan Step Per Revolution pada Motor Stepper
#define SPR 1500
#define RPM 10

// Pin Trigger WiFi Manager
#define button 0
