#include <WiFi.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <HTML.h>
namespace AP
{

    const char *ssid = "asd";

    const char *password = "asdads";

    WiFiServer server(IPAddress(0, 0, 0, 0), 80);

    uint16_t headerEnd = 0;
    uint16_t messageEnd = 0;
    uint8_t *buff = new uint8_t[512];

    char *method = new char[8];
    char *route = new char[16];

    void handleRoute(WiFiClient *client)
    {
        client->println("HTTP/1.1 200 OK");

        if (!strcmp(route, "/"))
        {
            client->println("Connection: close");
            client->println("Content-type:text/html");
            client->println(HTML::apPage);
        }
        else if (!strcmp(route, "/sheet.css"))
        {
            client->println("Connection: close");
            client->println("Cache-Control: max-age: 31536000, immutable");
            client->println("Content-type:text/css");
            client->println(HTML::styleSheet);
        }
        else if (!strcmp(route, "/jquery.js"))
        {
            client->println("Connection: close");
            client->println("Cache-Control: max-age: 31536000, immutable");
            client->println("Content-type:text/javascript");
            client->println(HTML::jquery);
        }
        // else if (!strcmp(route, ""))
        //{

        //}
        else if (!strcmp(route, "/getAps"))
        {
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
        pinMode(LED_BUILTIN, OUTPUT);
        WiFi.mode(WIFI_AP);
        WiFi.softAPConfig(IPAddress(192, 168, 0, 1), IPAddress(192, 168, 0, 1), IPAddress(255, 255, 255, 0));
        WiFi.softAP(ssid);

        Serial.println(WiFi.softAPIP());
        server.begin();
        Serial.begin(9600);

        xTaskCreate(apLoop, "aploop", 8192, 0, tskIDLE_PRIORITY, 0);
    }

}