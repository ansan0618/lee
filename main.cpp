#include <IBMIOTF32.h>
#include <Adafruit_sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define LED 2
#define DHTPIN 34
#define DHTTYPE DHT22

DHT_Unified dht(DHTPIN,DHTTYPE);


String user_html = "";

char*               ssid_pfix = (char*)"iothome";
unsigned long       lastPublishMillis = 0;

int tem = 0;
bool ledonoff = 0;




void TemHumCheck(){
    sensors_event_t event;

    dht.temperature().getEvent(&event);
    int tem = event.temperature;

    Serial.print(F("Temperature : "));
    Serial.print(event.temperature);
    

    dht.humidity().getEvent(&event);

    
    Serial.print(F("Humidity : "));
    Serial.println(event.relative_humidity);
    

    
    delay(2000);
}




void publishData() {
    StaticJsonDocument<512> root;
    JsonObject data = root.createNestedObject("d");

    // YOUR CODE for status reporting
    char buffer[10];

    // sprintf(buffer,"%d",light_value);
    data["temperature"] = tem;
    //memset(buffer, '\0', sizeof(buffer));

    // sprintf(buffer,"%d",ledonoff);
    // data["LED"] = ledonoff;
    // memset(buffer, '\0', sizeof(buffer));




    serializeJson(root, msgBuffer);
    client.publish(publishTopic, msgBuffer);


}
void handleUserCommand(JsonDocument* root) {
    JsonObject d = (*root)["d"];

    // YOUR CODE for command handling

     if(d.containsKey("LED")) {
        if (strcmp(d["LED"], "on")) {
            digitalWrite(LED,HIGH);

            Serial.println("LED on");
        } 
        else {
            digitalWrite(LED,LOW);

            Serial.println("LED off");
        }
    }


}
void message(char* topic, byte* payload, unsigned int payloadLength) {
    byte2buff(msgBuffer, payload, payloadLength);
    StaticJsonDocument<512> root;
    DeserializationError error = deserializeJson(root, String(msgBuffer));

    if (error) {
        Serial.println("handleCommand: payload parse FAILED");
        return;
    }

    handleIOTCommand(topic, &root);
    if (strstr(topic, "/device/update")) {
        JsonObject meta = cfg["meta"];

    // YOUR CODE for meta data synchronization

    } else if (strstr(topic, "/cmd/")) {            // strcmp return 0 if both string matches
        handleUserCommand(&root);
    }
}

void setup() {
    Serial.begin(9600);
    initDevice();

    pinMode(LED,OUTPUT);

    digitalWrite(LED,LOW);

    JsonObject meta = cfg["meta"];
    pubInterval = meta.containsKey("pubInterval") ? atoi((const char*)meta["pubInterval"]) : 0;
    lastPublishMillis = - pubInterval;
    startIOTWatchDog((void*)&lastPublishMillis, (int)(pubInterval * 5));
    // YOUR CODE for initialization of device/environment

    WiFi.mode(WIFI_STA);
    WiFi.begin((const char*)cfg["ssid"], (const char*)cfg["w_pw"]);
    int i = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        if(i++ > 10) reboot();
    }
    Serial.printf("\nIP address : "); Serial.println(WiFi.localIP());

    client.setCallback(message);
    set_iot_server();
}

void loop() {
    if (!client.connected()) {
        iot_connect();
    }
    client.loop();
    // YOUR CODE for routine operation in loop

    if ((pubInterval != 0) && (millis() - lastPublishMillis > pubInterval)) {
        publishData();
        lastPublishMillis = millis();
    }
    TemHumCheck();
}
