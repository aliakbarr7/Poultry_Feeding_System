// #pragma once
// #include <Arduino.h>
// #include <ArduinoJson.h> // Pastikan ini disertakan untuk JSON handling
// #include <WiFiClientSecure.h>
// #include <dev_config.h>
// #include <string>
// #include <interfaces/communication_interface.h>
// #include <interfaces/display_interface.h>
// #include <interfaces/loadcell_interface.h>
// #include <interfaces/network_interface.h>
// #include <interfaces/stepper_interface.h>
// #include <interfaces/storage_interface.h>
// #include <interfaces/time_interface.h>

// class FeedingApp
// {
// public:
//     FeedingApp(network_interface &net, communication_interface &comm, display_interface &disp, loadcell_interface &sens,
//                stepper_interface &step, storage_interface &stg, time_interface &time);
//     void init();
//     void loop();

// private:
//     network_interface &_net;
//     communication_interface &_comm;
//     display_interface &_disp;
//     loadcell_interface &_sens;
//     stepper_interface &_step;
//     storage_interface &_stg;
//     time_interface &_time;

//     unsigned long lastSendDataTime;
//     const unsigned long sendDataInterval = 10000; // Interval untuk mengirim data loadcell (10 detik)
//     unsigned long feedingStartTime;

//     unsigned long calibrationStartTime;
//     const unsigned long calibrationDuration = 10000; // Durasi kalibrasi dalam milidetik

//     bool isFeeding;
//     bool isFeed_ManualMode;
//     bool isCalibrating;
//     bool isFinishCalibrating;
//     bool wifiStatusDisplayed = false;

//     unsigned long startMillisLog = 0;
//     unsigned long intervalShowLog = 0;
//     unsigned int dataIndex = 0; // index untuk tracking data yang sedang ditampilkan
//     bool showDataInProgress = false;

//     String feedModeLog;
//     String feedTimeLog;
//     String durasiFeedLog;
//     String totalPakanLog;
//     float rawDurasiFeedLog;
//     float rawTotalPakanLog;

//     JsonArray array;         // Pastikan ini dideklarasikan di sini
//     WiFiClientSecure client; // Declaring WiFiClientSecure for HTTPS

//     String schedule1;
//     String schedule2;
//     String schedule3;
//     float totalFeed;            // Total pakan yang harus dihabiskan dalam 3 kali penjadwalan (kg)
//     float feedRate;             // Pakan yang dapat dikeluarkan selama 1 menit (kg)
//     unsigned long feedDuration; // Durasi waktu untuk mengeluarkan pakan per jadwal (ms)
//     float calibrationValue;     // Nilai kalibrasi dari load cell

//     bool schedule1Active;
//     bool schedule2Active;
//     bool schedule3Active;

//     bool hasFedSchedule1;
//     bool hasFedSchedule2;
//     bool hasFedSchedule3;

//     bool schedule1HasRun;
//     bool schedule2HasRun;
//     bool schedule3HasRun;

//     int lastMinute;

//     String prevSchedule1;
//     String prevSchedule2;
//     String prevSchedule3;

//     float prevTotalFeed;
//     float prevCalibration;

//     unsigned long lastDebounceTime = 0;     // Waktu terakhir tombol diproses
//     const unsigned long debounceDelay = 50; // Debounce delay dalam milidetik
//     unsigned long pressStartTime;           // Waktu saat tombol ditekan pertama kali
//     bool buttonPressed;                     // Status apakah tombol sedang ditekan

//     String GAS_ID = GAS;
//     const char *host_googlesheet = host_gs;
//     const int httpsPort_googlesheet = https_port_gs;

//     void callback(char *topic, byte *payload, unsigned int length);
//     void handleScheduleMessage(const String &message);
//     void handleManualMessage(const String &message);
//     void handleSetFeedMessage(const String &message);
//     // void handleCalibrationMessage(const String &message);
//     void handleStartFeedLog(String mode);
//     void handleStopFeedLog();
//     void showDat();
//     void deleteLog();

//     void checkSchedule();
//     bool isTimeToFeed(const String &schedule);
//     void startFeeding();
//     void stopFeeding();
//     void startCalibration();
//     void stopCalibration();
//     void saveConfig();
//     void loadConfig();
//     void saveLog();
//     void loadLog();
//     bool hasConfigChanged();
//     void updateFeedDuration();
//     void resetSystem();
//     void buttonConfig(int button_pin);
//     bool sendDataToGoogle(const std::string &mode, const std::string &durasi, const std::string &totalPakan, const std::string &waktu);
//     bool checkGoogleResponse(String response);
// };