#include "wifi_setup.h"
#include <WiFi.h>

void wifi_setup(String wifi_ssid, String wifi_pwd) {
    WiFi.mode(WIFI_STA);

    if (wifi_pwd.length() == 0) {
        WiFi.begin(wifi_ssid);
    }
    else {
        WiFi.begin(wifi_ssid, wifi_pwd);
    }
    Serial.println("MAC Address: " + String(WiFi.macAddress()));
    Serial.print("Connecting to " + wifi_ssid);
    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 20) {
        delay(1000);
        Serial.print(".");
        timeout++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        Serial.println("MAC Address: " + String(WiFi.macAddress()));
    } else {
        Serial.println("\nConnection failed. Check MAC registration.");
    }
}