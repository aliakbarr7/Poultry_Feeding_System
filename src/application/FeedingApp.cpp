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
      schedule1HasRun(false), schedule2HasRun(false), schedule3HasRun(false),
      startMillisLog(0), intervalShowLog(0), dataIndex(0), showDataInProgress(false),
      feedModeLog(""), feedTimeLog(""), rawDurasiFeedLog(0.0), rawTotalPakanLog(0.0),
      durasiFeedLog("0"), totalPakanLog("0"),
      pressStartTime(0), buttonPressed(false), lastDebounceTime(0), debounceDelay(50),
      lastMessagePublish(0), loadCellStart(0.0), loadCellEnd(0.0)
{
    client.setInsecure();
}

void FeedingApp::init()
{
    Serial.begin(115200);
    pinMode(button, INPUT_PULLUP);
    _disp.begin();
    _step.begin();
    _net.init();
    _comm.init();
    _time.init();

    if (!_stg.init())
    {
        Serial.println("Failed to initialize storage");
        while (1)
            ;
    }

    if (_net.init())
    {
        wifiStatusDisplayed = true;
        _disp.showMessage("      WiFi     ", "   Connected!   ");
        Serial.println("WiFi initialized and connected!");
        delay(1000);

        if (_comm.init() && _comm.connect())
        {

            // Mengatur waktu dari NTP
            if (_time.setTimeFromNTP())
            {
                Serial.println("Time set from NTP.");
                _disp.showMessage("Time", "Synchronized...");
                delay(1000);
            }
            else
            {
                Serial.println("Failed to set time from NTP.");
            }

            _disp.showMessage("Connected to....", "MQTT-Broker");
            delay(1000);
            Serial.println("Connected to MQTT-Broker");
            subscribeMQTTTopics();

            client.setInsecure();
        }

        if (_comm.init() && !_comm.connect())
        {

            wifiStatusDisplayed = false;
            Serial.println("Failed to connect to MQTT.");
            _disp.showMessage("      MQTT     ", " Not Connected! ");
        }
    }
    else
    {
        _disp.showMessage("      WiFi     ", " Not Connected! ");
        Serial.println("Failed to initialize WiFi.");
    }

    loadConfig();
    _sens.begin();

    _disp.setSchedule1(schedule1.c_str());
    _disp.setSchedule2(schedule2.c_str());
    _disp.setSchedule3(schedule3.c_str());
    _disp.setLoadValue(_sens.getWeight());
    _disp.setDateTime(_time.getCurrentTime().c_str());
    _disp.setFeedStatus("Standby");
    _disp.setFeedPerDay(totalFeed);
    _disp.setWifiConn(false);

    updateFeedDuration();
}

void FeedingApp::loop()
{
    _comm.loop(); // Ensure MQTT loop is called regularly
    _disp.update();

    loadCellData = _sens.getWeight();

    if (loadCellData <= 0)
    {
        loadCellData = 0;
    }

    bool wifiConnected = _net.checkStatus();

    if (wifiConnected && !wifiStatusDisplayed)
    {
        _disp.showMessage("      WiFi     ", "   Connected!   ");
        wifiStatusDisplayed = true;

        if (_comm.init() && _comm.connect())
        {
            delay(500);
            _disp.showMessage("Connected to....", "MQTT-Broker");
            delay(750);
            Serial.println("Connected to MQTT-Broker");
            subscribeMQTTTopics();
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

            String payload = String(loadCellData);
            _comm.publish(TOPIC_sendData, payload.c_str());
            lastMessagePublish = millis();
            _disp.setLoadValue(loadCellData);

            _disp.setDateTime(_time.getCurrentTime().c_str());
            Serial.println();
            Serial.println(_time.getCurrentTime());
            Serial.println("Feed Duration per Schedule: " + String(feedDuration / 1000) + " detik");
            Serial.print("Berat: ");
            Serial.println(loadCellData);
            Serial.println();
        }

        if (isFeeding && (currentMillis - feedingStartTime >= feedDuration))
        {
            if (!isFeed_ManualMode)
            {
                stopFeeding();
                delay(1000);
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

    checkMQTTConnection();
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

    if (hasConfigChanged())
    {
        saveConfig();
    }
}

void FeedingApp::startCalibration()
{
    isCalibrating = true;
    calibrationStartTime = millis();
    _disp.setCalibration();
    _sens.calibrate();
    Serial.println("Calibration started:");
}

void FeedingApp::stopCalibration()
{
    _disp.showMessage("Proses Kalibrasi", "Selesai!");
    Serial.println("Calibration stopped");
    isCalibrating = false;
}

void FeedingApp::updateFirmware()
{

    _disp.showMessage("     Update     ", "    Started!    ");
    String response = "Update Firmware Started!";
    _comm.publish(TOPIC_response, response.c_str());
    bool result = ota.update(OTA_SERVER, OTA_PORT, (String) "/update/firmware?id=" + (String)DEVICE_ID, false);

    if (result)
    {
        _disp.showMessage("     Update     ", "    Success!    ");
        String response = "Update Succes!";
        _comm.publish(TOPIC_response, response.c_str());
        delay(1000);
        ESP.restart();
    }

    else
    {
        _disp.showMessage("     Update     ", "     Failed!    ");
        String response = "Update Failed!";
        _comm.publish(TOPIC_response, response.c_str());
        delay(1000);
        ESP.restart();
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
            schedule1 = String(data[1]);
            schedule2 = String(data[2]);
            schedule3 = String(data[3]);

            _disp.showMessage("Memperbarui", "Jadwal...");
            _disp.setSchedule1(schedule1.c_str());
            _disp.setSchedule2(schedule2.c_str());
            _disp.setSchedule3(schedule3.c_str());

            String response = "Update Schedule Succes!";
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
            String state = String(data[1]);

            if (state == "1")
            {
                handleStartFeedLog("Manual");
                isFeed_ManualMode = true;
                startFeeding();
            }
            else if (state == "0")
            {
                stopFeeding();
                handleStopFeedLog();
                isFeed_ManualMode = false;
            }
        }
    }
}

void FeedingApp::handleSetFeedMessage(const String &message)
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
            totalFeed = String(data[1]).toFloat();

            updateFeedDuration();
            saveConfig();

            _disp.setFeedPerDay(totalFeed);
            _disp.showMessage("Memperbarui", "Sett Pakan/hari!");

            String response = "Feed Settings Updated!";
            _comm.publish(TOPIC_response, response.c_str());
        }
    }
}

void FeedingApp::checkSchedule()
{
    int currentMinute = _time.getMinute();

    if (currentMinute != lastMinute)
    {
        hasFedSchedule1 = false;
        hasFedSchedule2 = false;
        hasFedSchedule3 = false;

        lastMinute = currentMinute;
    }

    if (isTimeToFeed(schedule1) && !schedule1Active && !isFeed_ManualMode && !hasFedSchedule1)
    {
        handleStartFeedLog("Auto");
        startFeeding();
        hasFedSchedule1 = true;
    }
    else if (isTimeToFeed(schedule2) && !schedule2Active && !isFeed_ManualMode && !hasFedSchedule2)
    {
        handleStartFeedLog("Auto");
        startFeeding();
        hasFedSchedule2 = true;
    }
    else if (isTimeToFeed(schedule3) && !schedule3Active && !isFeed_ManualMode && !hasFedSchedule3)
    {
        handleStartFeedLog("Auto");
        startFeeding();
        hasFedSchedule3 = true;
    }
}

bool FeedingApp::isTimeToFeed(const String &schedule)
{
    int currentHour = _time.getHour();
    int currentMinute = _time.getMinute();

    int scheduleHour = schedule.substring(0, 2).toInt();
    int scheduleMinute = schedule.substring(3, 5).toInt();

    return (currentHour == scheduleHour && currentMinute == scheduleMinute);
}

void FeedingApp::startFeeding()
{

    if (loadCellData <= 0.0)
    {
        _disp.setFeedStatus("Pakan Kosong");
        _disp.showMessage("Fedeer", "Pakan Kosong");
        String response = "Feed is Empty!";
        _comm.publish(TOPIC_response, response.c_str());
        return;
    }

    if (_net.checkStatus() && _comm.connect())
    {
        String response = "Fedeer ON!";
        _comm.publish(TOPIC_response, response.c_str());
    }

    _step.start();
    feedingStartTime = millis();
    isFeeding = true;
    _disp.setFeedStatus("Feeding");
    _disp.showMessage("Fedeer", "Diaktifkan");
    Serial.println("Feeding started");
}

void FeedingApp::stopFeeding()
{
    if (_net.checkStatus() && _comm.connect())
    {
        String response = "Fedeer OFF!";
        _comm.publish(TOPIC_response, response.c_str());
    }

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
    Serial.println();
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

            if (millis() - lastAttemptTime >= 2000)
            {
                if (!dataValidated && retryCount < 5)
                {
                    bool stateLogSpreadsheet = sendDataToGoogle(mode, durasi, totalPakan, waktu);

                    if (stateLogSpreadsheet)
                    {
                        Serial.println("Data berhasil terkirim.");
                        dataValidated = true;
                        retryCount = 0;
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
                    dataValidated = true;
                }
            }

            if (dataValidated)
            {
                dataIndex++;
                intervalShowLog = millis();
                dataValidated = false;
            }
        }
        else
        {
            showDataInProgress = false;
            _disp.showMessage("Backup FeedLog", "Sukses Terkirim!");
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

    // Membaca load cell saat mulai pemberian pakan
    loadCellStart = loadCellData;
    Serial.println("Load Cell Start: " + String(loadCellStart, 2));
}

void FeedingApp::handleStopFeedLog()
{
    unsigned long stopMillisLog = millis();
    rawDurasiFeedLog = (stopMillisLog - startMillisLog) / 60000.0;

    loadCellEnd = loadCellData;
    Serial.println("Load Cell End: " + String(loadCellEnd, 2));

    rawTotalPakanLog = loadCellStart - loadCellEnd;

    durasiFeedLog = String(rawDurasiFeedLog, 2);
    totalPakanLog = String(rawTotalPakanLog, 2);

    if (_net.checkStatus())
    {
        if (feedModeLog != "")
        {
            std::string mode = std::string(feedModeLog.c_str());
            std::string durasi = std::string(durasiFeedLog.c_str());
            std::string totalPakan = std::string(totalPakanLog.c_str());
            std::string waktu = std::string(feedTimeLog.c_str());

            bool result = sendDataToGoogle(mode, durasi, totalPakan, waktu);

            if (result)
            {
                _disp.showMessage("FeedLog Berhasil", "Terkirim!");
            }
            else
            {
                _disp.showMessage("FeedLog Gagal", "Terkirim!");
            }
        }
    }
    else
    {
        saveLog();
        _disp.showMessage("FeedLog Berhasil", "Disimpan! (FS)");
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
    client.setTimeout(10000); // Set timeout 10 detik
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

    String responseBody = "";
    while (client.connected())
    {
        while (client.available())
        {
            String line = client.readStringUntil('\n');
            responseBody += line;
        }
        _comm.loop(); // Keep calling MQTT loop to maintain connection
    }

    client.stop();

    bool success = checkGoogleResponse(responseBody);
    if (!success)
    {
        Serial.println("Failed to send data to Google Sheets.");
    }
    return success;
}

bool FeedingApp::checkGoogleResponse(String response)
{
    if (response.indexOf("\"state\":\"success\"") >= 0 || response.indexOf("2a8") >= 0)
    {
        return true;
    }

    if (response.indexOf("HTTP/1.1 302") >= 0 || response.indexOf("Moved Temporarily") >= 0)
    {
        return true;
    }

    return false;
}

void FeedingApp::buttonConfig(int button_pin)
{
    int buttonState = digitalRead(button_pin); // Membaca status tombol

    // Jika tombol ditekan (LOW)
    if (buttonState == LOW && !buttonPressed)
    {
        pressStartTime = millis(); // Catat waktu saat tombol ditekan pertama kali
        buttonPressed = true;      // Menandai bahwa tombol sedang ditekan
    }

    // Jika tombol dilepaskan (HIGH)
    if (buttonState == HIGH && buttonPressed)
    {
        unsigned long pressDuration = millis() - pressStartTime; // Hitung durasi tombol ditekan

        // Jika tombol ditekan kurang dari 5 detik
        if (pressDuration < 3000)
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
        // Jika tombol ditekan lebih dari 5 detik dan kurang dari sama dengan 10 detik
        else if (pressDuration >= 5000 && pressDuration <= 10000)
        {
            startCalibration();
        }

        // Jika tombol ditekan lebih dari 10 detik
        else if (pressDuration > 10000)
        {
            updateFirmware();
        }

        buttonPressed = false; // Reset status tombol setelah dilepas
    }
}

void FeedingApp::checkMQTTConnection()
{
    unsigned long currentMillis = millis();

    // Cek koneksi MQTT setiap 1 menit
    if (currentMillis - lastMQTTCheckTime >= mqttCheckInterval)
    {
        lastMQTTCheckTime = currentMillis;

        if (_comm.connected()) // Jika terhubung ke MQTT
        {
            Serial.println("MQTT connected.");
            mqttConnected = true;
            lastMQTTConnectAttemptTime = currentMillis; // Reset waktu terakhir koneksi berhasil
            // Periksa apakah ada timeout pada pesan
            if (millis() - lastMessagePublish >= 30000)
            {
                Serial.println("Message timeout: resetting MQTT connection...");
                resetMQTTConnection();
            }
        }
        else
        {
            Serial.println("MQTT not connected.");
            mqttConnected = false;

            // Jika tidak terhubung selama lebih dari 5 menit, lakukan reset
            if (currentMillis - lastMQTTConnectAttemptTime >= mqttMaxReconnectTime)
            {
                Serial.println("MQTT connection failed for 5 minutes. Resetting connection...");
                resetMQTTConnection();
                lastMQTTConnectAttemptTime = currentMillis; // Reset waktu setelah reset
            }
            else
            {
                // Coba hubungkan kembali ke MQTT
                Serial.println("Attempting to reconnect to MQTT...");
                if (_comm.connect())
                {
                    Serial.println("Reconnected to MQTT.");
                    _disp.showMessage("Reconnected", "to MQTT...");
                    subscribeMQTTTopics(); // Subscribe kembali ke topik
                }
                else
                {
                    Serial.println("Failed to reconnect to MQTT.");
                }
            }
        }
    }
}

void FeedingApp::resetMQTTConnection()
{
    Serial.println("Resetting MQTT connection...");
    _comm.disconnect();  // Disconnect MQTT
    delay(1000);         // Tunggu sebentar sebelum mencoba kembali
    if (_comm.connect()) // Coba sambungkan ulang ke broker MQTT
    {
        Serial.println("Reconnected to MQTT after reset.");
        subscribeMQTTTopics(); // Subscribe kembali ke topik yang diperlukan
    }
    else
    {
        Serial.println("Failed to reconnect to MQTT after reset.");
    }
}

void FeedingApp::subscribeMQTTTopics()
{
    _comm.subscribe(TOPIC_setSchedule);
    _comm.subscribe(TOPIC_manual);
    _comm.subscribe(TOPIC_setPakan);
    _comm.setCallback([this](char *topic, uint8_t *payload, unsigned int length)
                      { this->callback(topic, payload, length); });
    Serial.println("Subscribed to necessary MQTT topics.");
}
