#include <Arduino.h>
#include <config.h>

#include <ESP32Ping.h>
#include <driver.h>
namespace MQTT
{
    WiFiClient client;
    PubSubClient mqttClient(client);

    bool mqttMode = false;

    char *messageBuff = new char[128];

    void reset()
    {
        Serial.println("Resetting and rebooting...");
        CONFIG::clear();
        ESP.restart();
    }

    void handleMessage(char *topic, byte *message, unsigned int length)
    {
        Serial.print("Message arrived on topic: ");
        Serial.println(topic);

        for (size_t i = 0; i < length; i++)
        {
            /* code */
            messageBuff[i] = (char)message[i];
        }

        messageBuff[length] = 0;

        Serial.println(messageBuff);

        if (!strcmp(topic, "disconnect"))
        {
            if (!strcmp(messageBuff, CONFIG::id))
                reset();
        }

        else if (!strcmp(topic, "start"))
        {
            if (!strcmp(messageBuff, CONFIG::id))
                DRIVER::startCamera();
        }

        else if (!strcmp(topic, "stop"))
        {
            if (!strcmp(messageBuff, CONFIG::id))
                DRIVER::stopCamera();
        }
    }

    void connect()
    {
        int attempts = 0;

        while (!mqttClient.connected())
        {
            DRIVER::running = false;
            Serial.print("Attempting MQTT connection...");
            // Attempt to connect
            if (mqttClient.connect("espClient", "mqtt-test", "mqtt-test"))
            {
                Serial.println("connected");
                // Subscribe
                mqttClient.subscribe("disconnect");
                mqttClient.subscribe("start");
                mqttClient.subscribe("stop");

                DRIVER::mqttClient = &mqttClient;

                DRIVER::start();
            }
            else
            {
                if (attempts == 2)
                    reset();
                Serial.print("failed, rc=");
                Serial.print(mqttClient.state());
                Serial.println(" try again in 2 seconds");
                // Wait 5 seconds before retrying
                delay(2);

                attempts++;
            }
        }
    }

    void mqttLoop(void *params)
    {

        while (mqttMode)
        {
            if (!mqttClient.connected())
                connect();

            mqttClient.loop();
        }

        vTaskDelete(NULL);
    }

    bool ping()
    {
        int attempts = 0;

        while (attempts < 2)
        {
            Serial.printf("Pinging attempt %d...\n", attempts);

            if (Ping.ping(IPAddress((uint8_t *)CONFIG::mqttServer)))
            {
                Serial.println("Ping successful!");
                return 1;
            }
            attempts++;
        }

        return 0;
    }

    void start()
    {
        bool foundServer = ping();
        if (!foundServer)
        {
            reset();
        }

        mqttMode = true;
        Serial.println("Beggining mqtt connection");
        Serial.printf("MQTT address is %s\n", CONFIG::mqttServer);

        mqttClient.setServer(CONFIG::mqttServer, 1883);
        mqttClient.setCallback(handleMessage);

        xTaskCreate(mqttLoop, "mqttLoop", 4096, 0, tskIDLE_PRIORITY, 0);
    }

}