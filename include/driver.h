#include <PubSubClient.h>
#include "esp_camera.h"

#define MOVEMENT 13

namespace DRIVER
{

    PubSubClient *mqttClient;
    bool running = true;

    bool cameraRunning = false;

    void sendMessage(const char *topic, const char *message)
    {
        mqttClient->publish(topic, message);
        mqttClient->endPublish();
    }

    void sendImage(const char *topic)
    {
        camera_fb_t *frameBuffer = esp_camera_fb_get();
        Serial.printf("Image is at address %d length %d bytes\n", frameBuffer->buf, frameBuffer->len);
        mqttClient->beginPublish(topic, (unsigned int)frameBuffer->len, false);
        mqttClient->write(frameBuffer->buf, frameBuffer->len);
        if (!mqttClient->endPublish())
            Serial.println("Failed sending mqtt packet");
        esp_camera_fb_return(frameBuffer);
    }

    void imageLoop(void *params)
    {
        Serial.println("Starting camera");
        while (cameraRunning)
        {
            sendImage("face");
            delay(1000);
        }

        vTaskDelete(NULL);
    }

    void startCamera()
    {
        if (cameraRunning)
            return;

        cameraRunning = true;

        xTaskCreate(imageLoop, "imageLoop", 4096, 0, tskIDLE_PRIORITY, 0);
    }

    void stopCamera()
    {
        Serial.println("Stopping camera");
        cameraRunning = false;
    }

    void always(void *params)
    {
        int previousVal = -1;

        while (running)
        {

            int val = digitalRead(MOVEMENT);
            Serial.println(val);

            if (val != previousVal)
            {
                if (val)
                    sendMessage("movement", "found");
                else
                    sendMessage("movement", "not found");
            }

            previousVal = val;
            delay(100);
        }

        vTaskDelete(NULL);
    }

    void start()
    {
        running = true;
        Serial.println("Starting always loop...");

        pinMode(MOVEMENT, INPUT);

        xTaskCreate(always, "alwaysLoop", 4096, 0, tskIDLE_PRIORITY, 0);
    }

    void stop()
    {
        running = false;
    }

}