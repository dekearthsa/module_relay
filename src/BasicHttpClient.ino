
#include "M5Atom.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>


WiFiClient espClient;
PubSubClient client(espClient);

const char *PUB_DATA_TOPIC = "test/message";
// const char *PUB_DATA_TOPIC = "v1/events/data/update/json";

const char* ssid        = "Bannok Kiang Chan3";
const char* password    = "0818462519";
const char* mqtt_server = "192.168.1.49";

char baseMacChr[18] = {0};
unsigned long LED_turnON_time;
StaticJsonDocument<256> doc;

void setup() {
    M5.begin(true, false, true);
    M5.dis.fillpix(0xffff00);
    setupWifi();
    serial_setup(); 
    mqtt_setup();
    LED_ON();
}

void loop() {
    M5.update();
    // Serial.println(M5.Btn.wasPressed());
    if(!client.connected()){
        mqttConnect();
    }else{  
        if(millis() - LED_turnON_time > 5000){
        // client.publish(PUB_DATA_TOPIC, "standby...");
        LED_ON();
        }

        if(M5.Btn.isPressed()){
                LED_OFF();
                Serial.println("Sending motion data to MQTT");
                doc.clear();
                json_setup();
                doc["batteryLevel"] = 100;
                JsonObject data = doc.createNestedObject("data");
                data["motionDetected"] = 1;
                char buffer[256];
                size_t n = serializeJson(doc, buffer);
                Serial.print("Serialized data: ");
                Serial.println(buffer);
                client.publish(PUB_DATA_TOPIC, buffer, n);
                delay(5000);
            }
            
        }
}

inline void LED_ON()
{
    // client.publish(PUB_DATA_TOPIC, "LED blue on...");
    M5.dis.fillpix(0x0000ff);
}

inline void LED_OFF()
{
    M5.dis.fillpix(0x000000);
    LED_turnON_time = millis();
}

inline void json_setup()
{
    doc["deviceID"] = String(baseMacChr);
    doc["deviceType"] = "motion";
}

void setupWifi() {
    delay(10);
    Serial.print("\nConnection to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);  // Start Wifi connection.  开始wifi连接
    int i = 1;
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        if(i == 1){
            i = 2;
            M5.dis.fillpix(0xffff00);
            Serial.print(".");
        }else{
            i = 1;
            M5.dis.fillpix(0x0000ff);
            Serial.print(".");
        }
    }
    Serial.printf("\nSuccess\n");
    Serial.println(WiFi.localIP());
    M5.dis.fillpix(0x00ff00);
    delay(3000);
}

char *getMacAddress()
{
    uint8_t baseMac[6];
    esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
    sprintf(baseMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
    return baseMacChr;
}

inline void serial_setup()
{
    Serial.begin(115200);
    Serial.println("Start serial");

    getMacAddress();
    Serial.print("Device UUID : ");
    Serial.println(String(baseMacChr));
}

void mqttConnect()
{
    Serial.print(client.connected());
    while (!client.connected())
    {
        M5.dis.fillpix(0xa5ff00);
        Serial.print("Attempting MQTT connection...");
        if (client.connect(baseMacChr)){
            Serial.println("connected");
            M5.dis.fillpix(0x000000);
        }else{
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            M5.dis.fillpix(0x00ff00);
            delay(5000);
        }
    }
}


inline void mqtt_setup()
{
    client.setServer(mqtt_server,1883); 
    if (!client.connected())
    {
        mqttConnect();
    }
    client.loop();
}
