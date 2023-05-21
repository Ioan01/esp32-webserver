#include <Arduino.h>
#include <config.h>
#include <PubSubClient.h>
namespace MQTT
{
    WiFiClient client;
    PubSubClient mqttClient(client);

    bool mqttMode = false;

    void handleMessage(char *topic, byte *message, unsigned int length)
    {
        Serial.print("Message arrived on topic: ");
        Serial.print(topic);
        Serial.print(". Message: ");
        Serial.printf("Lenmth  : %u\n", length);
    }

    void connect()
    {
        while (!mqttClient.connected())
        {
            Serial.print("Attempting MQTT connection...");
            // Attempt to connect
            if (mqttClient.connect("espClient", "mqtt-test", "mqtt-test"))
            {
                Serial.println("connected");
                // Subscribe
                mqttClient.subscribe("esp32");
            }
            else
            {
                Serial.print("failed, rc=");
                Serial.print(mqttClient.state());
                Serial.println(" try again in 2 seconds");
                // Wait 5 seconds before retrying
                delay(2);
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

    void start()
    {
        mqttMode = true;
        Serial.println("Beggining mqtt connection");
        Serial.printf("MQTT address is %s\n", CONFIG::mqttServer);

        mqttClient.setServer(CONFIG::mqttServer, 1883);
        mqttClient.setCallback(handleMessage);

        xTaskCreate(mqttLoop, "mqttLoop", 4096, 0, tskIDLE_PRIORITY, 0);
    }

}