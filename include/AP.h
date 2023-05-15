#include <WiFi.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <HTML.h>
namespace AP
{
    String json = String();
    const char *ssid = "asd";

    const char *password = "asdads";

    WiFiServer server(IPAddress(0, 0, 0, 0), 80);

    uint16_t headerEnd = 0;
    uint16_t messageEnd = 0;
    uint8_t *buff = new uint8_t[512];

    char *method = new char[8];
    char *route = new char[16];

    static const char *falseTrue[] = {"false", "true"};

    void scanWifi()
    {

        json.clear();

        json += "{ \"networks:\" : [";

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
        else if (!strcmp(route, "/jquery.js"))
        {
            client->println("Cache-Control: max-age: 31536000, immutable");
            client->println("Content-type:text/javascript");
            client->println(HTML::jquery);
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
                messageEnd = client->available() - 1;
                client->read(buff, messageEnd);
                determineLimits();

                Serial.println(headerEnd);

                sscanf((char *)buff, "%s %s", method, route);

                Serial.printf("Called %s on route %s\n", method, route);
                Serial.write(buff, headerEnd);
                Serial.println("------");
                Serial.write(buff + headerEnd + 1, messageEnd - headerEnd);

                handleRoute(client);
                break;
            }
        }
        client->stop();
    }

    void apLoop(void *params)
    {

        while (true)
        {
            WiFiClient client = server.available();

            if (client)
                handleClient(&client);

            delay(100);
        }

        vTaskDelete(NULL);
    }

    extern void start()
    {
        json.reserve(512);

        WiFi.mode(WIFI_AP_STA);
        WiFi.softAPConfig(IPAddress(192, 168, 0, 1), IPAddress(192, 168, 0, 1), IPAddress(255, 255, 255, 0));
        WiFi.softAP(ssid);

        Serial.println(WiFi.softAPIP());
        server.begin();
        Serial.begin(9600);

        xTaskCreate(apLoop, "aploop", 8192, 0, tskIDLE_PRIORITY, 0);
    }

}