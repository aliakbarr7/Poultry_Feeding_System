#include "FeedingApp.h"
#include <dev_config.h>

FeedingApp::FeedingApp(network_interface &net, communication_interface &comm, display_interface &disp, loadcell_interface &sens, stepper_interface &step, storage_interface &stg, time_interface &time)
    : _net(net), _comm(comm), _disp(disp), _sens(sens), _step(step), _stg(stg), _time(time),
      lastSendDataTime(0), feedingStartTime(0), isFeeding(false), isFeed_ManualMode(false),
      isCalibrating(false), isFinishCalibrating(false), wifiStatusDisplayed(false),
      schedule1("08:00"), schedule2("12:00"), schedule3("16:00"), totalFeed(5.0),
      feedRate(VAL_CALLIBRATION), feedDuration((totalFeed / 3) / feedRate * 60000),
      calibrationValue(1.0), schedule1Active(false), schedule2Active(false),
      schedule3Active(false), prevSchedule1(""), prevSchedule2(""), prevSchedule3(""),
      prevTotalFeed(0), prevCalibration(1.0), hasFedSchedule1(false),
      hasFedSchedule2(false), hasFedSchedule3(false), lastMinute(-1),
      schedule1HasRun(false), schedule2HasRun(false), schedule3HasRun(false), // Inisialisasi flag
      startMillisLog(0), intervalShowLog(0), dataIndex(0), showDataInProgress(false),
      feedModeLog(""), feedTimeLog(""), rawDurasiFeedLog(0.0), rawTotalPakanLog(0.0),
      durasiFeedLog("0"), totalPakanLog("0")
{
    client.setInsecure();
}

void FeedingApp::init()
{
    Serial.begin(115200);
    pinMode(button, INPUT_PULLUP);
    _disp.begin();
    _step.begin();
    _time.init();

    _net.init();
    _comm.init();

    if (!_stg.init())
    {
        Serial.println("Failed to initialize storage");
        while (1)
            ; // Halt execution if storage initialization fails
    }

    if (_net.init())
    {
        wifiStatusDisplayed = true;
        _disp.showMessage("      WiFi     ", "   Connected!   ");
        Serial.println("WiFi initialized and connected!");
        delay(1000);
        if (_comm.init() && _comm.connect())
        {
            _disp.showMessage("Connected to....", "mosquitto.org");
            delay(1000);
            Serial.println("Connected to test.mosquitto.org");
            _comm.subscribe(TOPIC_setSchedule);
            _comm.subscribe(TOPIC_manual);
            _comm.subscribe(TOPIC_setPakan);
            _comm.subscribe(TOPIC_calibrate);
            _comm.setCallback([this](char *topic, uint8_t *payload, unsigned int length)
                              { this->callback(topic, payload, length); });
        }
        else
        {
            wifiStatusDisplayed = false;
            Serial.println("Failed to connect to MQTT.");
            _disp.showMessage("      MQTT     ", " Not Connected! ");
        }

        client.setInsecure();
    }
    else
    {
        _disp.showMessage("      WiFi     ", " Not Connected! ");
        Serial.println("Failed to initialize WiFi.");
    }

    loadConfig();
    _sens.begin(calibrationValue);

    _disp.setSchedule1(schedule1.c_str());
    _disp.setSchedule2(schedule2.c_str());
    _disp.setSchedule3(schedule3.c_str());
    _disp.setLoadValue(_sens.getLoad());
    _disp.setDateTime(_time.getCurrentTime().c_str());
    _disp.setFeedStatus("Standby");
    _disp.setFeedPerDay(totalFeed);
    _disp.setWifiConn(false);

    updateFeedDuration();
}

void FeedingApp::loop()
{
    _comm.loop();
    _disp.update();

    bool wifiConnected = _net.checkStatus();

    if (wifiConnected && !wifiStatusDisplayed)
    {
        _disp.showMessage("      WiFi     ", "   Connected!   ");
        wifiStatusDisplayed = true;

        if (_comm.init() && _comm.connect())
        {
            _disp.showMessage("Connected to....", "mosquitto.org");
            delay(1000);
            Serial.println("Connected to test.mosquitto.org");
            _comm.subscribe(TOPIC_setSchedule);
            _comm.subscribe(TOPIC_manual);
            _comm.subscribe(TOPIC_setPakan);
            _comm.subscribe(TOPIC_calibrate);
            _comm.setCallback([this](char *topic, uint8_t *payload, unsigned int length)
                              { this->callback(topic, payload, length); });

            loadLog();
        }
        else
        {
            wifiStatusDisplayed = false;
            Serial.println("Failed to connect to MQTT.");
            _disp.showMessage("      MQTT     ", " Not Connected! ");
        }
    }
    else if (!wifiConnected && wifiStatusDisplayed)
    {
        _disp.showMessage("      WiFi     ", " Not Connected! ");
        wifiStatusDisplayed = false;
    }

    if (!showDataInProgress)
    {
        buttonConfig(button);

        unsigned long currentMillis = millis();
        if (currentMillis - lastSendDataTime >= sendDataInterval)
        {
            lastSendDataTime = currentMillis;
            float loadcellData = _sens.getLoad();
            String payload = String(loadcellData);
            _comm.publish(TOPIC_sendData, payload.c_str());
            _disp.setLoadValue(loadcellData);

            _disp.setDateTime(_time.getCurrentTime().c_str());
            Serial.println();
            Serial.println(_time.getCurrentTime());
            Serial.println("Feed Duration per Schedule: " + String(feedDuration / 1000) + " detik");
            Serial.println();
        }

        if (isFeeding && (currentMillis - feedingStartTime >= feedDuration))
        {
            if (!isFeed_ManualMode)
            {
                stopFeeding();
                handleStopFeedLog();
            }
        }

        if (isCalibrating && (currentMillis - calibrationStartTime >= calibrationDuration))
        {
            stopCalibration();
        }

        checkSchedule();
    }

    else
    {
        loadLog();
    }

    // _step.update();
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
        Serial.println("Manual message received");
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

    if (hasConfigChanged())
    {
        saveConfig();
    }
}

void FeedingApp::handleScheduleMessage(const String &message)
{
    if (message.startsWith("*") && message.endsWith("#"))
    {
        String msg = message.substring(1, message.length() - 1);
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

            _disp.showMessage("Memperbarui", "Jadwal...");
            _disp.setSchedule1(schedule1.c_str());
            _disp.setSchedule2(schedule2.c_str());
            _disp.setSchedule3(schedule3.c_str());

            String response = "*" + String(001) + "," + "Update Schedule Succes!" + "#";
            _comm.publish(TOPIC_response, response.c_str());
        }
    }
}

void FeedingApp::handleManualMessage(const String &message)
{
    if (message.startsWith("*") && message.endsWith("#"))
    {
        String msg = message.substring(1, message.length() - 1);
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
                Serial.println("Starting feeding manually");

                String response = "*" + String(002) + "," + "Fedeer Activated!" + "#";
                _comm.publish(TOPIC_response, response.c_str());
                handleStartFeedLog("Manual");
                isFeed_ManualMode = true;
                startFeeding();
            }
            else if (state == "0")
            {
                Serial.println("Stopping feeding manually");

                String response = "*" + String(002) + "," + "Fedeer Non Activated!" + "#";
                _comm.publish(TOPIC_response, response.c_str());
                handleStopFeedLog();
                isFeed_ManualMode = false;
                stopFeeding();
            }
        }
    }
}

void FeedingApp::handleSetFeedMessage(const String &message)
{
    Serial.println("handleManualMessage called with message: " + message);

    if (message.startsWith("*") && message.endsWith("#"))
    {
        String msg = message.substring(1, message.length() - 1);
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

            String response = "*" + String(003) + "," + "Feed Settings Updated!" + "#";
            _comm.publish(TOPIC_response, response.c_str());

            Serial.println("Feed settings updated:");
            Serial.println("Total Feed: " + String(totalFeed) + " kg");
            Serial.println("Feed Duration per Schedule: " + String(feedDuration / 1000) + " Detik");
        }
    }
}

void FeedingApp::handleCalibrationMessage(const String &message)
{
    if (message.startsWith("*") && message.endsWith("#"))
    {
        String msg = message.substring(1, message.length() - 1);
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

            String response = "*" + String(005) + "," + "Calibration Settings Updated!" + "#";
            _comm.publish(TOPIC_response, response.c_str());

            startCalibration(calibrationValue);
        }
    }
}

void FeedingApp::startCalibration(float knownWeight)
{
    isCalibrating = true;
    calibrationStartTime = millis();
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
}

void FeedingApp::checkSchedule()
{
    int currentMinute = _time.getMinute(); // Dapatkan menit saat ini

    if (currentMinute != lastMinute)
    {
        // Reset flag saat menit berubah
        hasFedSchedule1 = false;
        hasFedSchedule2 = false;
        hasFedSchedule3 = false;
        schedule1HasRun = false; // Reset flag ini saat menit berubah
        schedule2HasRun = false;
        schedule3HasRun = false;

        lastMinute = currentMinute;
    }

    // Eksekusi hanya jika jadwal belum pernah dijalankan untuk menit ini
    if (isTimeToFeed(schedule1) && !schedule1Active && !isFeed_ManualMode && !schedule1HasRun)
    {
        schedule1Active = true;
        hasFedSchedule1 = true;
        schedule1HasRun = true; // Tandai bahwa schedule 1 sudah dijalankan
        handleStartFeedLog("Auto");
        startFeeding();
    }
    else if (isTimeToFeed(schedule2) && !schedule2Active && !isFeed_ManualMode && !schedule2HasRun)
    {
        schedule2Active = true;
        hasFedSchedule2 = true;
        schedule2HasRun = true; // Tandai bahwa schedule 2 sudah dijalankan
        handleStartFeedLog("Auto");
        startFeeding();
    }
    else if (isTimeToFeed(schedule3) && !schedule3Active && !isFeed_ManualMode && !schedule3HasRun)
    {
        schedule3Active = true;
        hasFedSchedule3 = true;
        schedule3HasRun = true; // Tandai bahwa schedule 3 sudah dijalankan
        handleStartFeedLog("Auto");
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

    prevSchedule1 = schedule1;
    prevSchedule2 = schedule2;
    prevSchedule3 = schedule3;
    prevTotalFeed = totalFeed;
    prevCalibration = calibrationValue;

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
    feedDuration = (totalFeed / 3) / feedRate * 60000;
    Serial.println("Feed Duration updated:");
    Serial.println("Feed Duration per Schedule: " + String(feedDuration / 1000) + " Detik");
}

void FeedingApp::buttonConfig(int button_pin)
{
    if (digitalRead(button_pin) == LOW)
    {
        _disp.setAPConfig();
        _net.WiFiConfig();

        if (_net.checkStatus())
        {
            _disp.showMessage("      WiFi     ", "   Connected!   ");
        }
        else
        {
            _disp.showMessage("      WiFi     ", " Not Connected! ");
        }
    }
}

void FeedingApp::saveLog()
{
    std::string data = "*" + std::string(feedModeLog.c_str()) + "," + std::string(durasiFeedLog.c_str()) + "," + std::string(totalPakanLog.c_str()) + "," + std::string(feedTimeLog.c_str()) + "#\n";

    std::string existingData = _stg.readFile("/data.txt");
    existingData += data;

    _stg.writeFile("/data.txt", existingData);

    Serial.println("Data yang disimpan:");
    Serial.println(data.c_str());
}

void FeedingApp::loadLog()
{
    static unsigned long lastAttemptTime = 0;
    String currentLine;
    static int retryCount = 0;
    static bool dataValidated = false;

    // Baca data hanya jika tidak sedang menunjukkan data sebelumnya
    if (!showDataInProgress)
    {
        dataIndex = 0;
        intervalShowLog = millis();
        showDataInProgress = true;

        if (_stg.exists("/data.txt"))
        {
            std::string data = _stg.readFile("/data.txt");
            if (data.length() > 0)
            {
                Serial.println("Menampilkan data...");
            }
            else
            {
                Serial.println("Tidak ada data yang tersimpan.");
                showDataInProgress = false;
                return;
            }
        }
        else
        {
            Serial.println("Tidak ada data yang tersimpan.");
            showDataInProgress = false;
            return;
        }
    }

    if (showDataInProgress && millis() - intervalShowLog >= 3000)
    {
        std::string data = _stg.readFile("/data.txt");
        int lineStart = 0;
        int lineEnd = data.find('\n', lineStart);

        for (int i = 0; i < dataIndex; i++)
        {
            lineStart = lineEnd + 1;
            lineEnd = data.find('\n', lineStart);
            if (lineEnd == std::string::npos)
                break;
        }

        if (lineStart < data.length() && lineEnd != std::string::npos)
        {
            std::string line = data.substr(lineStart, lineEnd - lineStart);

            Serial.println();
            Serial.print("Index = ");
            Serial.println(dataIndex);

            line = line.substr(1, line.length() - 2);
            int firstComma = line.find(',');
            int secondComma = line.find(',', firstComma + 1);
            int thirdComma = line.find(',', secondComma + 1);

            std::string mode = line.substr(0, firstComma);
            std::string durasi = line.substr(firstComma + 1, secondComma - firstComma - 1);
            std::string totalPakan = line.substr(secondComma + 1, thirdComma - secondComma - 1);
            std::string waktu = line.substr(thirdComma + 1);

            Serial.println("Mode = " + String(mode.c_str()));
            Serial.println("Durasi = " + String(durasi.c_str()) + " minutes");
            Serial.println("Total Pakan = " + String(totalPakan.c_str()) + " kg");
            Serial.println("Waktu = " + String(waktu.c_str()));
            Serial.println();

            // Hanya lakukan retry jika waktu retry sudah terlewati
            if (millis() - lastAttemptTime >= 2000)
            {
                // Pengiriman data ke Google Spreadsheet dengan retry 5 kali jika gagal
                if (!dataValidated && retryCount < 5)
                {
                    Serial.print("Mengirim data: ");
                    Serial.println(currentLine.c_str());

                    // Mengirim data ke Google Spreadsheet dan memvalidasi
                    bool stateLogSpreadsheet = sendDataToGoogle(mode, durasi, totalPakan, waktu);

                    if (stateLogSpreadsheet)
                    {
                        Serial.println("Data berhasil terkirim.");
                        dataValidated = true;
                        retryCount = 0; // Reset retry untuk data berikutnya
                    }
                    else
                    {
                        Serial.println("Pengiriman data gagal, mencoba ulang...");
                        retryCount++;
                    }
                    lastAttemptTime = millis();
                }

                if (!dataValidated && retryCount >= 5)
                {
                    Serial.println("Percobaan maksimal tercapai. Melewati data.");
                    retryCount = 0;
                    dataValidated = true; // Lanjut ke data berikutnya
                }
            }

            // Jika data sudah divalidasi, lanjut ke data berikutnya
            if (dataValidated)
            {
                dataIndex++;
                intervalShowLog = millis();
                dataValidated = false; // Reset status validasi untuk data berikutnya
            }
        }
        else
        {
            showDataInProgress = false;
            Serial.println("Semua data telah ditampilkan.");
            _stg.deleteFile("/data.txt");
            Serial.println("Log Data berhasil dihapus.");
        }
    }
}

void FeedingApp::handleStartFeedLog(String mode)
{
    feedModeLog = mode;
    feedTimeLog = _time.getCurrentTime();
    startMillisLog = millis();
}

void FeedingApp::handleStopFeedLog()
{
    unsigned long stopMillisLog = millis();
    rawDurasiFeedLog = (stopMillisLog - startMillisLog) / 60000.0;
    rawTotalPakanLog = rawDurasiFeedLog * feedRate;

    durasiFeedLog = String(rawDurasiFeedLog, 2);
    totalPakanLog = String(rawTotalPakanLog, 2);

    if (_net.checkStatus())
    {
        if (feedModeLog != "")
        {
            Serial.println();
            Serial.println("Mode = " + feedModeLog);
            Serial.println("Durasi = " + String(durasiFeedLog) + " minutes");
            Serial.println("Total Pakan = " + String(totalPakanLog) + " kg");
            Serial.println("Waktu = " + feedTimeLog);
            Serial.println();

            // Konversi variabel String ke std::string
            std::string mode = std::string(feedModeLog.c_str());
            std::string durasi = std::string(durasiFeedLog.c_str());
            std::string totalPakan = std::string(totalPakanLog.c_str());
            std::string waktu = std::string(feedTimeLog.c_str());

            // Panggil fungsi sendDataToGoogle
            bool result = sendDataToGoogle(mode, durasi, totalPakan, waktu);

            if (result)
            {
                Serial.println("Data berhasil terkirim.");
            }

            else
            {
                Serial.println("Pengiriman data gagal.");
            }
        }
    }
    else
    {
        saveLog();
    }

    feedModeLog = "";
}

void FeedingApp::resetSystem()
{
    Serial.println("System will reset now...");
    ESP.restart();
}

bool FeedingApp::sendDataToGoogle(const std::string &mode, const std::string &durasi, const std::string &totalPakan, const std::string &waktu)
{
    Serial.println("Sending data to Google Sheets...");

    if (!client.connect(host_googlesheet, httpsPort_googlesheet))
    {
        Serial.println("Connection to Google failed.");
        return false;
    }

    String url = String("/macros/s/") + GAS_ID + String("/exec?mode=") + mode.c_str() +
                 String("&durasi=") + durasi.c_str() +
                 String("&totalPakan=") + totalPakan.c_str() +
                 String("&waktu=") + waktu.c_str();

    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host_googlesheet + "\r\n" +
                 "User-Agent: ESP32\r\n" +
                 "Connection: close\r\n\r\n");

    Serial.println("Request sent: " + url);

    // Read the response
    String responseBody = "";
    while (client.connected())
    {
        while (client.available())
        {
            String line = client.readStringUntil('\n');
            responseBody += line;
        }
    }

    client.stop(); // Close connection
    // Serial.println("Response from server: " + responseBody);

    // Check if the response indicates success
    bool success = checkGoogleResponse(responseBody);

    if (!success)
    {
        Serial.println("Failed to send data to Google Sheets.");
    }
    return success;
}

bool FeedingApp::checkGoogleResponse(String response)
{
    // Cek jika respons mengandung "state":"success" atau "2a8" atau kode redirect 302
    if (response.indexOf("\"state\":\"success\"") >= 0 || response.indexOf("2a8") >= 0)
    {
        return true; // Data berhasil dikirim
    }

    // Jika respons menunjukkan pengalihan (302 Moved Temporarily), anggap sukses
    if (response.indexOf("HTTP/1.1 302") >= 0 || response.indexOf("Moved Temporarily") >= 0)
    {
        return true; // Redirect, tapi dianggap sukses
    }

    return false; // Jika tidak ada tanda-tanda sukses, anggap gagal
}
