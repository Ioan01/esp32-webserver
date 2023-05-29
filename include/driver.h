#include <PubSubClient.h>

namespace DRIVER
{

    PubSubClient *mqttClient;
    bool running = true;

    void sendMessage(const char *topic, const char *message)
    {
        mqttClient->publish(topic, message);
        mqttClient->endPublish();
    }

    void always(void *params)
    {
        int previousVal = -1;

        while (running)
        {

            int val = digitalRead(26);
            if (val != previousVal)
            {
                if (val)
                    sendMessage("door", "open");
                else
                    sendMessage("door", "closed");
            }

            previousVal = val;
            delay(100);
        }

        vTaskDelete(NULL);
    }

    void start()
    {
        Serial.println("Starting always loop...");

        pinMode(26, INPUT);

        xTaskCreate(always, "alwaysLoop", 4096, 0, tskIDLE_PRIORITY, 0);
    }

    void stop()
    {
        running = false;
    }

}