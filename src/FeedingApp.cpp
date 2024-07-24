#include "FeedingApp.h"
#include <dev_config.h>
#include <ArduinoJson.h>

FeedingApp::FeedingApp(network_interface &net, communication_interface &comm, display_interface &disp, loadcell_interface &sens, stepper_interface &step, storage_interface &stg, time_interface &time)
    : _net(net), _comm(comm), _disp(disp), _sens(sens), _step(step), _stg(stg), _time(time), lastSendDataTime(0), feedingStartTime(0), isFeeding(false), isCalibrating(false), isFinishCalibrating(false), schedule1("08:00"), schedule2("12:00"), schedule3("16:00"), totalFeed(5.0), feedRate(VAL_CALLIBRATION), feedDuration((totalFeed / 3) / feedRate * 60000), calibrationValue(1.0), schedule1Active(false), schedule2Active(false), schedule3Active(false), prevSchedule1(""), prevSchedule2(""), prevSchedule3(""), prevTotalFeed(0), prevCalibration(1.0), hasFedSchedule1(false), hasFedSchedule2(false), hasFedSchedule3(false) {}

void FeedingApp::init()
{
    Serial.begin(115200);
    _disp.begin();
    _step.begin();
    _time.init();

    if (!_stg.init())
    {
        Serial.println("Failed to initialize storage");
        while (1)
            ; // Halt execution if storage initialization fails
    }

    _net.init();
    _comm.init();

    if (_net.init())
    {
        Serial.println("WiFi initialized and connected!");
        if (_comm.init() && _comm.connect())
        {
            Serial.println("MQTT connected!");
            _comm.subscribe(TOPIC_setSchedule);
            _comm.subscribe(TOPIC_manual);
            _comm.subscribe(TOPIC_setPakan);
            _comm.subscribe(TOPIC_calibrate);
            _comm.setCallback([this](char *topic, uint8_t *payload, unsigned int length)
                              { this->callback(topic, payload, length); });
        }
        else
        {
            Serial.println("Failed to connect to MQTT.");
        }
    }
    else
    {
        Serial.println("Failed to initialize WiFi.");
    }

    // Load and display configuration from storage
    loadConfig();

    _sens.begin(calibrationValue);

    // Display initial values on LCD
    _disp.setSchedule1(schedule1.c_str());
    _disp.setSchedule2(schedule2.c_str());
    _disp.setSchedule3(schedule3.c_str());
    _disp.setLoadValue(_sens.getLoad());
    _disp.setDateTime(_time.getCurrentTime().c_str());
    _disp.setFeedStatus("Standby");
    _disp.setFeedPerDay(totalFeed);
    _disp.setWifiConn(false);

    updateFeedDuration(); // Hitung durasi pakan awal

    // Wait for 5 seconds to display loaded configuration on Serial Monitor
    delay(5000);
}

void FeedingApp::loop()
{
    // _net.checkStatus();
    _comm.loop();

    _step.update();

    _disp.update();

    unsigned long currentMillis = millis();
    if (currentMillis - lastSendDataTime >= sendDataInterval)
    {
        lastSendDataTime = currentMillis;
        float loadcellData = _sens.getLoad();
        String payload = "*" + String(004) + "," + String(loadcellData) + "#";
        _comm.publish(TOPIC_sendData, payload.c_str());
        _disp.setLoadValue(loadcellData);

        _disp.setDateTime(_time.getCurrentTime().c_str());
        Serial.println();
        Serial.println(_time.getCurrentTime());
        Serial.println("Feed Duration per Schedule: " + String(feedDuration) + " ms");
        Serial.println();
    }

    if (isFeeding && (currentMillis - feedingStartTime >= feedDuration))
    {
        stopFeeding();
    }

    if (isCalibrating && (currentMillis - calibrationStartTime >= calibrationDuration))
    {
        stopCalibration();
    }

    // if (isCalibrating)
    // {
    //     _disp.setCalibration(_sens.getLoad());
    // }

    checkSchedule();
    // Tambahkan ini untuk memeriksa status stepper
}

void FeedingApp::callback(char *topic, byte *payload, unsigned int length)
{
    String message;
    for (int i = 0; i < length; i++)
    {
        message += (char)payload[i];
    }
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    Serial.println(message);

    if (String(topic) == TOPIC_setSchedule)
    {
        handleScheduleMessage(message);
    }
    else if (String(topic) == TOPIC_manual)
    {
        handleManualMessage(message);
    }
    else if (String(topic) == TOPIC_setPakan)
    {
        handleSetFeedMessage(message);
    }
    else if (String(topic) == TOPIC_calibrate)
    {
        handleCalibrationMessage(message);
    }

    // Save config if there are changes
    if (hasConfigChanged())
    {
        saveConfig();
    }
}

void FeedingApp::handleScheduleMessage(const String &message)
{
    if (message.startsWith("*") && message.endsWith("#"))
    {
        String msg = message.substring(1, message.length() - 1); // Hilangkan karakter '*' dan '#'
        char data[4][10];
        int index = 0;
        int startPos = 0;
        for (int i = 0; i < msg.length(); i++)
        {
            if (msg[i] == ',' || i == msg.length() - 1)
            {
                int endPos = (i == msg.length() - 1) ? i + 1 : i;
                msg.substring(startPos, endPos).toCharArray(data[index], 10);
                startPos = i + 1;
                index++;
            }
        }

        if (index == 4)
        {
            String id = String(data[0]);
            schedule1 = String(data[1]);
            schedule2 = String(data[2]);
            schedule3 = String(data[3]);

            Serial.println("Schedules updated:");
            Serial.println("Schedule 1: " + schedule1);
            Serial.println("Schedule 2: " + schedule2);
            Serial.println("Schedule 3: " + schedule3);

            // Update _disp
            _disp.showMessage("Memperbarui", "Jadwal...");
            _disp.setSchedule1(schedule1.c_str());
            _disp.setSchedule2(schedule2.c_str());
            _disp.setSchedule3(schedule3.c_str());
        }
    }
}

void FeedingApp::handleManualMessage(const String &message)
{
    if (message.startsWith("*") && message.endsWith("#"))
    {
        String msg = message.substring(1, message.length() - 1); // Hilangkan karakter '*' dan '#'
        char data[2][10];
        int index = 0;
        int startPos = 0;
        for (int i = 0; i < msg.length(); i++)
        {
            if (msg[i] == ',' || i == msg.length() - 1)
            {
                int endPos = (i == msg.length() - 1) ? i + 1 : i;
                msg.substring(startPos, endPos).toCharArray(data[index], 10);
                startPos = i + 1;
                index++;
            }
        }

        if (index == 2)
        {
            String id = String(data[0]);
            String state = String(data[1]);

            if (state == "1")
            {
                // _step.start();
                startFeeding();
            }
            else if (state == "0")
            {
                // _step.stop();
                stopFeeding();
            }
        }
    }
}

void FeedingApp::handleSetFeedMessage(const String &message)
{
    if (message.startsWith("*") && message.endsWith("#"))
    {
        String msg = message.substring(1, message.length() - 1); // Hilangkan karakter '*' dan '#'
        char data[2][10];
        int index = 0;
        int startPos = 0;
        for (int i = 0; i < msg.length(); i++)
        {
            if (msg[i] == ',' || i == msg.length() - 1)
            {
                int endPos = (i == msg.length() - 1) ? i + 1 : i;
                msg.substring(startPos, endPos).toCharArray(data[index], 10);
                startPos = i + 1;
                index++;
            }
        }

        if (index == 2)
        {
            String id = String(data[0]);
            totalFeed = String(data[1]).toFloat();

            updateFeedDuration();
            saveConfig();

            _disp.setFeedPerDay(totalFeed);
            _disp.showMessage("Memperbarui", "Sett Pakan/hari!");
            Serial.println("Feed settings updated:");
            Serial.println("Total Feed: " + String(totalFeed) + " kg");
            Serial.println("Feed Duration per Schedule: " + String(feedDuration) + " ms");
        }
    }
}

void FeedingApp::handleCalibrationMessage(const String &message)
{
    if (message.startsWith("*") && message.endsWith("#"))
    {
        String msg = message.substring(1, message.length() - 1); // Hilangkan karakter '*' dan '#'
        char data[2][10];
        int index = 0;
        int startPos = 0;
        for (int i = 0; i < msg.length(); i++)
        {
            if (msg[i] == ',' || i == msg.length() - 1)
            {
                int endPos = (i == msg.length() - 1) ? i + 1 : i;
                msg.substring(startPos, endPos).toCharArray(data[index], 10);
                startPos = i + 1;
                index++;
            }
        }

        if (index == 2)
        {
            String id = String(data[0]);
            calibrationValue = String(data[1]).toFloat();
            startCalibration(calibrationValue);
        }
    }
}

void FeedingApp::startCalibration(float knownWeight)
{
    isCalibrating = true;
    calibrationStartTime = millis(); // Catat waktu mulai kalibrasi
    _disp.setCalibration(knownWeight);
    _sens.calibrate(knownWeight);
    Serial.println("Calibration started:");
    Serial.println("Known Weight: " + String(knownWeight) + " kg");
}

void FeedingApp::stopCalibration()
{

    _disp.showMessage("Proses Kalibrasi", "Selesai!");
    Serial.println("Calibration stopped");
    isCalibrating = false;
    // _disp.setCalibration(0); // Reset the calibration _disp
}

void FeedingApp::checkSchedule()
{
    if (isTimeToFeed(schedule1) && !schedule1Active)
    {
        schedule1Active = true;
        startFeeding();
    }
    else if (isTimeToFeed(schedule2) && !schedule2Active)
    {
        schedule2Active = true;
        startFeeding();
    }
    else if (isTimeToFeed(schedule3) && !schedule3Active)
    {
        schedule3Active = true;
        startFeeding();
    }
}

bool FeedingApp::isTimeToFeed(const String &schedule)
{
    String now = _time.getCurrentTime();
    int currentHour = _time.getHour();
    int currentMinute = _time.getMinute();

    int scheduleHour = schedule.substring(0, 2).toInt();
    int scheduleMinute = schedule.substring(3, 5).toInt();

    // // Debug print untuk perbandingan waktu
    // Serial.print("Jam sekarang: ");
    // Serial.println(currentHour);
    // Serial.print("Menit sekarang: ");
    // Serial.println(currentMinute);
    // Serial.print("Jam jadwal: ");
    // Serial.println(scheduleHour);
    // Serial.print("Menit jadwal: ");
    // Serial.println(scheduleMinute);

    return (currentHour == scheduleHour && currentMinute == scheduleMinute);
}

void FeedingApp::startFeeding()
{
    _step.start();
    feedingStartTime = millis();
    isFeeding = true;
    _disp.setFeedStatus("Feeding");
    _disp.showMessage("Fedeer", "Diaktifkan");
    Serial.println("Feeding started");
}

void FeedingApp::stopFeeding()
{
    _step.stop();
    isFeeding = false;
    _disp.setFeedStatus("Standby");
    _disp.showMessage("Fedeer", "Dinonaktifkan");
    Serial.println("Feeding stopped");

    // Reset schedule active flags
    schedule1Active = false;
    schedule2Active = false;
    schedule3Active = false;
}

void FeedingApp::saveConfig()
{
    DynamicJsonDocument doc(1024);
    doc["schedule1"] = schedule1;
    doc["schedule2"] = schedule2;
    doc["schedule3"] = schedule3;
    doc["totalFeed"] = totalFeed;
    doc["calibration"] = calibrationValue;

    std::string configData;
    serializeJson(doc, configData);

    if (!_stg.writeFile("/config.json", configData))
    {
        Serial.println("Failed to write config to storage");
    }

    // Update previous configuration
    prevSchedule1 = schedule1;
    prevSchedule2 = schedule2;
    prevSchedule3 = schedule3;
    prevTotalFeed = totalFeed;
    prevCalibration = calibrationValue;
}

void FeedingApp::loadConfig()
{
    if (!_stg.exists("/config.json"))
    {
        Serial.println("Config file does not exist");
        return;
    }

    std::string configData = _stg.readFile("/config.json");
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, configData);

    if (error)
    {
        Serial.println("Failed to parse config file");
        return;
    }

    schedule1 = doc["schedule1"].as<String>();
    schedule2 = doc["schedule2"].as<String>();
    schedule3 = doc["schedule3"].as<String>();
    totalFeed = doc["totalFeed"].as<float>();
    calibrationValue = doc["calibration"].as<float>();

    // Update previous configuration
    prevSchedule1 = schedule1;
    prevSchedule2 = schedule2;
    prevSchedule3 = schedule3;
    prevTotalFeed = totalFeed;
    prevCalibration = calibrationValue;

    // Print loaded configuration
    Serial.println("Loaded configuration from LittleFS:");
    Serial.println("Schedule 1: " + schedule1);
    Serial.println("Schedule 2: " + schedule2);
    Serial.println("Schedule 3: " + schedule3);
    Serial.print("Total Feed: ");
    Serial.println(totalFeed, 2);
    Serial.print("Calibration Value: ");
    Serial.println(calibrationValue, 2);
}

bool FeedingApp::hasConfigChanged()
{
    return (schedule1 != prevSchedule1 ||
            schedule2 != prevSchedule2 ||
            schedule3 != prevSchedule3 ||
            totalFeed != prevTotalFeed ||
            calibrationValue != prevCalibration);
}

void FeedingApp::updateFeedDuration()
{
    feedDuration = (totalFeed / 3) / feedRate * 60000; // Update durasi waktu untuk mengeluarkan pakan per jadwal
    Serial.println("Feed Duration updated:");
    Serial.println("Feed Duration per Schedule: " + String(feedDuration) + " ms");
}

void FeedingApp::resetSystem()
{
    Serial.println("System will reset now...");
    ESP.restart();
}
