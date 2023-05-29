#include <WiFi.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <HTML.h>
#include <MQTT.h>
#include "esp_camera.h"

#define TIMEOUT 5

#define CAM_PIN_PWDN 32
#define CAM_PIN_RESET -1 // software reset will be performed
#define CAM_PIN_XCLK 0
#define CAM_PIN_SIOD 26
#define CAM_PIN_SIOC 27

#define CAM_PIN_D7 35
#define CAM_PIN_D6 34
#define CAM_PIN_D5 39
#define CAM_PIN_D4 36
#define CAM_PIN_D3 21
#define CAM_PIN_D2 19
#define CAM_PIN_D1 18
#define CAM_PIN_D0 5
#define CAM_PIN_VSYNC 25
#define CAM_PIN_HREF 23
#define CAM_PIN_PCLK 22
namespace AP
{
    String json = String();

    bool apMode = true;

    const char *ssid = "asd";

    const char *password = "asdads";

    WiFiServer server(IPAddress(0, 0, 0, 0), 80);

    uint16_t headerEnd = 0;
    uint16_t messageEnd = 0;
    uint8_t *buff = new uint8_t[512];

    char *method = new char[8];
    char *route = new char[16];

    char *trySsid = new char[MAX_LEN];
    char *tryPassword = new char[MAX_LEN];

    static const char *falseTrue[] = {"false", "true"};

    bool checkConnection()
    {

        int counter = -1;

        sscanf((const char *)buff + headerEnd + 1, "%[a-zA-Z0-9_ !@#$%^&*(]\\%[a-zA-Z0-9_ !@#$%^&*(]\\%[a-zA-Z0-9_ !@#$%^&*(.]\\", trySsid, tryPassword, CONFIG::mqttServer);

        if (strlen(tryPassword) < 2)
        {
            Serial.printf("No password\n");
            tryPassword = 0;
        }

        Serial.printf("Trying to connect to %s with password %s...\n", trySsid, tryPassword);

        WiFi.begin(trySsid, tryPassword);

        while (WiFi.status() != WL_CONNECTED)
        {
            counter++;
            delay(100);
            if (counter == TIMEOUT * 10)
            {
                Serial.printf("Failed to connect after %d awaits.", counter);
                return false;
            }
        }

        WiFi.disconnect();

        apMode = false;

        return true;
    }

    void scanWifi()
    {

        json.clear();

        json += "{ \"networks\" : [";

        int count = WiFi.scanNetworks();

        for (size_t i = 0; i < count; i++)
        {
            json += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"open\":" + falseTrue[WiFi.encryptionType(i) == WIFI_AUTH_OPEN] + "}";
            if (i != count - 1)
                json += ',';
        }

        json += "]}";

        Serial.printf(json.c_str());
    }

    void handleRoute(WiFiClient *client)
    {
        if (!strcmp(route, "/connect"))
        {
            if (checkConnection())
            {
                client->println("HTTP/1.1 200 OK");
                client->println("Connection: close");
                client->println();
            }
            else
            {
                client->println("HTTP/1.1 400 Bad Request");
                client->println("Connection: close");
                client->println();
            }
            return;
        }

        client->println("HTTP/1.1 200 OK");
        client->println("Connection: close");

        if (!strcmp(route, "/") || !strcmp(route, "/?"))
        {
            client->println("Content-type:text/html");
            client->println(HTML::apPage);
        }
        else if (!strcmp(route, "/sheet.css"))
        {
            client->println("Cache-Control: max-age: 31536000, immutable");
            client->println("Content-type:text/css");
            client->println(HTML::styleSheet);
        }
        else if (!strcmp(route, "/script.js"))
        {
            client->println("Cache-Control: max-age: 31536000, immutable");
            client->println("Content-type:text/javascript");
            client->println(HTML::script);
        }
        // else if (!strcmp(route, ""))
        //{

        //}
        else if (!strcmp(route, "/scanAps"))
        {
            scanWifi();
            client->println("Content-type:application/json");
            client->println();
            client->println(json.c_str());
        }
        else if (!strcmp(route, "/scanAps"))
        {
            scanWifi();
            client->println("Content-type:application/json");
            client->println();
            client->println(json.c_str());
        }

        client->println();
    }

    void determineLimits()
    {
        uint16_t index = 0;
        while (true)
        {
            index++;

            if (buff[index] == '\r' && buff[index + 1] == '\n' && !isalnum(buff[index + 2]))
                break;
        }

        headerEnd = index + 3;
    }

    void handleClient(void *params)
    {
        WiFiClient *client = (WiFiClient *)params;
        Serial.println(client->remoteIP());

        while (client->connected())
        {
            if (client->available())
            {
                messageEnd = client->available();
                client->read(buff, messageEnd);
                buff[messageEnd - 1] = 0;

                determineLimits();

                Serial.println(headerEnd);

                sscanf((char *)buff, "%s %s", method, route);

                Serial.printf("Called %s on route %s\n", method, route);
                Serial.write(buff, headerEnd);
                Serial.println("------");
                Serial.write(buff + headerEnd + 1, messageEnd - headerEnd);
                Serial.println();

                handleRoute(client);
                break;
            }
        }
        client->stop();
    }

    void apLoop(void *params)
    {

        while (apMode)
        {
            WiFiClient client = server.available();

            if (client)
                handleClient(&client);

            delay(100);
        }

        WiFi.softAPdisconnect();
        WiFi.mode(WIFI_STA);
        Serial.println("Switched to STA mode");

        Serial.printf("Connecting to %s\n", trySsid);

        WiFi.begin(trySsid, tryPassword);
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(100);
        }

        Serial.printf("Connected at %s\n", WiFi.localIP().toString().c_str());

        CONFIG::pref.begin("config", false);

        CONFIG::pref.putBytes("ssid", trySsid, MAX_LEN);

        CONFIG::pref.putBytes("password", tryPassword, MAX_LEN);

        CONFIG::pref.putBytes("mqttServer", CONFIG::mqttServer, MAX_LEN);

        CONFIG::pref.end();

        MQTT::start();

        vTaskDelete(NULL);
    }

    esp_err_t camera_init()
    {

        static camera_config_t camera_config = {
            .pin_pwdn = CAM_PIN_PWDN,
            .pin_reset = CAM_PIN_RESET,
            .pin_xclk = CAM_PIN_XCLK,
            .pin_sscb_sda = CAM_PIN_SIOD,
            .pin_sscb_scl = CAM_PIN_SIOC,

            .pin_d7 = CAM_PIN_D7,
            .pin_d6 = CAM_PIN_D6,
            .pin_d5 = CAM_PIN_D5,
            .pin_d4 = CAM_PIN_D4,
            .pin_d3 = CAM_PIN_D3,
            .pin_d2 = CAM_PIN_D2,
            .pin_d1 = CAM_PIN_D1,
            .pin_d0 = CAM_PIN_D0,
            .pin_vsync = CAM_PIN_VSYNC,
            .pin_href = CAM_PIN_HREF,
            .pin_pclk = CAM_PIN_PCLK,

            // XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
            .xclk_freq_hz = 20000000,
            .ledc_timer = LEDC_TIMER_0,
            .ledc_channel = LEDC_CHANNEL_0,

            .pixel_format = PIXFORMAT_JPEG, // YUV422,GRAYSCALE,RGB565,JPEG
            .frame_size = FRAMESIZE_QVGA,   // QQVGA-UXGA Do not use sizes above QVGA when not JPEG

            .jpeg_quality = 12, // 0-63 lower number means higher quality
            .fb_count = 1,      // if more than one, i2s runs in continuous mode. Use only with JPEG
            .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
        };
        // power up the camera if PWDN pin is defined
        if (CAM_PIN_PWDN != -1)
        {
            pinMode(CAM_PIN_PWDN, OUTPUT);
            digitalWrite(CAM_PIN_PWDN, LOW);
        }

        // initialize the camera
        esp_err_t err = esp_camera_init(&camera_config);
        if (err != ESP_OK)
        {
            Serial.println("Camera Init Failed");
            return err;
        }

        return ESP_OK;
    }

    esp_err_t camera_capture()
    {
        // acquire a frame
        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb)
        {
            Serial.println("Camera Capture Failed");
            return ESP_FAIL;
        }
        // replace this with your own function

        // return the frame buffer back to the driver for reuse
        esp_camera_fb_return(fb);
        return ESP_OK;
    }

    extern void start()
    {

        camera_init();

        camera_capture();

                json.reserve(512);

        WiFi.mode(WIFI_AP_STA);
        WiFi.softAPConfig(IPAddress(192, 168, 0, 1), IPAddress(192, 168, 0, 1), IPAddress(255, 255, 255, 0));
        WiFi.softAP(ssid);

        Serial.println(WiFi.softAPIP());
        server.begin();

        xTaskCreate(apLoop, "aploop", 8192, 0, tskIDLE_PRIORITY, 0);
    }

}