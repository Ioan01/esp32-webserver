#include <EEPROM.h>

namespace CONFIG {

    #define SSID_PASSWORD_ADDRESS 0
    #define MAX_LEN 32
    #define MASK 179


    uint8_t* ssid = new uint8_t[MAX_LEN];
    uint8_t* password = new uint8_t[MAX_LEN];

    bool foundPassword = 0;
    bool foundSSID = 0;

    void tryConnect()
    {
        WiFi.mode(WIFI_STA);

        Serial.printf("Connecting to %s\n",ssid);
        WiFi.begin((char*)ssid ,(char*)password);

        switch (WiFi.status())
        {
            case WL_CONNECTED:
                Serial.println("Connected!");
            break;
            default:
                Serial.println("Failed to connect.");
            break;
        }
        
    }


    extern void initialize()
    {
        Serial.println("Trying to find password and username..."); 

        uint8_t index = 0;

        int address = SSID_PASSWORD_ADDRESS;

        // try to read ssid
        do
        {
            ssid[index] = EEPROM.read(address++);
            ssid[index] ^= MASK;
        } while (ssid[index++] && index < MAX_LEN);

        if (ssid[0] == 0)
        {
            Serial.println("No SSID found. Exiting config.");
            delete ssid;
            delete password;
            return;
        }
        foundSSID = 1;

        index = 0;

        Serial.printf("Found SSID : %s\n",ssid);

        // try to read password
        do
        {
            password[index] = EEPROM.read(address++);
            password[index] ^= MASK;
        } while (password[index++] && index < MAX_LEN);

        if (password[0] == 0)
        {
            Serial.println("No Password found. Exiting config.");
            delete password;
            password = 0;
        }
        else {
            foundPassword = true;
            Serial.printf("Found SSID : %s\n",password);
        }

        tryConnect();
    }
}