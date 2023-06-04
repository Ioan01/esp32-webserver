#include <EEPROM.h>
#include <Preferences.h>

#define SSID_PASSWORD_ADDRESS 0
#define MAX_LEN 32
#define MASK 179

namespace CONFIG
{
    const char *id = "two";
    Preferences pref;

    char *ssid = new char[MAX_LEN];
    char *password = new char[MAX_LEN];

    char *mqttServer = new char[MAX_LEN];

    bool foundPassword = 0;
    bool foundSSID = 0;

    bool connected = 0;

    void tryConnect()
    {
        WiFi.mode(WIFI_STA);

        Serial.printf("Connecting to %s\n", ssid);
        WiFi.begin(ssid, password);
        int timeout = 0;
        while (timeout < 50 && WiFi.status() != WL_CONNECTED)
        {
            timeout++;
            delay(100);
        }

        switch (WiFi.status())
        {
        case WL_CONNECTED:
            Serial.println("Connected!");
            connected = 1;

            break;
        default:
            Serial.println("Failed to connect.");
            WiFi.disconnect();

            WiFi.mode(WIFI_MODE_NULL);
            break;
        }
    }

    void initialize()
    {
        Serial.println("Trying to find password and username...");
        pref.begin("config", true);

        size_t readSsid = pref.getBytes("ssid", ssid, MAX_LEN);
        pref.getBytes("password", password, MAX_LEN);
        pref.getBytes("mqttServer", mqttServer, MAX_LEN);

        pref.end();

        if (readSsid > 0)
            tryConnect();
    }

    void clear()
    {
        pref.begin("config", false);

        pref.remove("ssid");
        pref.remove("password");
        pref.remove("mqttServer");

        pref.end();
    }
}