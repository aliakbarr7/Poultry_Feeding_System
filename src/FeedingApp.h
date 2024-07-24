#pragma once
#include <Arduino.h>
#include <string>
#include <interfaces/communication_interface.h>
#include <interfaces/display_interface.h>
#include <interfaces/loadcell_interface.h>
#include <interfaces/network_interface.h>
#include <interfaces/stepper_interface.h>
#include <interfaces/storage_interface.h>
#include <interfaces/time_interface.h>

class FeedingApp
{
public:
    FeedingApp(network_interface &net, communication_interface &comm, display_interface &disp, loadcell_interface &sens,
               stepper_interface &step, storage_interface &stg, time_interface &time);
    void init();
    void loop();

private:
    network_interface &_net;
    communication_interface &_comm;
    display_interface &_disp;
    loadcell_interface &_sens;
    stepper_interface &_step;
    storage_interface &_stg;
    time_interface &_time;

    unsigned long lastSendDataTime;
    const unsigned long sendDataInterval = 10000; // Interval untuk mengirim data loadcell (10 detik)
    unsigned long feedingStartTime;

    unsigned long calibrationStartTime;
    const unsigned long calibrationDuration = 10000; // Durasi kalibrasi dalam milidetik

    bool isFeeding;
    bool isCalibrating;
    bool isFinishCalibrating;

    String schedule1;
    String schedule2;
    String schedule3;
    float totalFeed;            // Total pakan yang harus dihabiskan dalam 3 kali penjadwalan (kg)
    float feedRate;             // Pakan yang dapat dikeluarkan selama 1 menit (kg)
    unsigned long feedDuration; // Durasi waktu untuk mengeluarkan pakan per jadwal (ms)
    float calibrationValue;     // Nilai kalibrasi dari load cellF

    bool schedule1Active;
    bool schedule2Active;
    bool schedule3Active;

    bool hasFedSchedule1;
    bool hasFedSchedule2;
    bool hasFedSchedule3;

    String prevSchedule1;
    String prevSchedule2;
    String prevSchedule3;
    
    float prevTotalFeed;
    float prevCalibration;

    int lastMinute;

    void callback(char *topic, byte *payload, unsigned int length);
    void handleScheduleMessage(const String &message);
    void handleManualMessage(const String &message);
    void handleSetFeedMessage(const String &message);
    void handleCalibrationMessage(const String &message);
    void checkSchedule();
    bool isTimeToFeed(const String &schedule);
    void startFeeding();
    void stopFeeding();
    void startCalibration(float knownWeight);
    void stopCalibration();
    void saveConfig();
    void loadConfig();
    bool hasConfigChanged();
    void updateFeedDuration(); // Tambahkan metode ini
    void resetSystem();
};
