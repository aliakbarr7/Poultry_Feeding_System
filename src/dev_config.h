// Konfigurasi

// Koneksi WiFi
#define WIFI_SSID "EWS_AP"
#define WIFI_PASSWORD "12345678910"


// MQTT Connection
#define MQTT_SERVER "test.mosquitto.org"
#define MQTT_CLIENTID "Poultry Feeding System"
// MQTT Topic
#define TOPIC_setSchedule "feeder/setschedule"
#define TOPIC_setPakan "feeder/setpakan"
#define TOPIC_sendData "feeder/sendData"
#define TOPIC_calibrate "feeder/calibrate"
#define TOPIC_manual "feeder/manual"


// Inisialisasi Pin Driver HX711
const int LOADCELL_DOUT_PIN = 4;
const int LOADCELL_SCK_PIN = 5;
// Nilai kalibrasi pengeluaran pakan per menit (Ex 0.5kg/m)
#define VAL_CALLIBRATION 1.5 


// Inisialisasi Pin Stepper
#define DIR 17 // Saat ini masih menggunakan LED
#define STEP 23
// Settingan Step Per Revolution pada Motor Stepper
#define SPR 200
