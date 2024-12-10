#ifndef PARKING_H
#define PARKING_H

#include<PubSubClient.h>
#include <ESP8266WiFi.h>
#include <Servo.h>
#include <rgb_lcd.h>
#include <vector>
#include <String>

struct Event {
    String description;
    String time;
    String date;
};

// Classe de base abstraite
class BaseParking {
public:
    virtual ~BaseParking() {}
    virtual int getCurrentCars() = 0;
    virtual int getMaxPlaces() = 0;
    virtual std::vector<Event> getEventLog() = 0;

    // Méthode virtuelle pour afficher une info basique
    virtual void displayInfo() = 0;
};

class Parking : public BaseParking { // Parking hérite maintenant de BaseParking
public:
    Parking(int servoInPin, int ultrasonicInPin, int servoOutPin, int ultrasonicOutPin, int maxPlaces);

    void setup();
    void loop();
    String generateHTML();

    // Implémentations des méthodes virtuelles de BaseParking
    int getCurrentCars() override;
    int getMaxPlaces() override;
    std::vector<Event> getEventLog() override;

    // Implémentation de displayInfo() depuis BaseParking
    void displayInfo() override {
        Serial.print("Parking actuel: ");
        Serial.print(currentCars);
        Serial.print("/");
        Serial.println(maxPlaces);
    }

    // Redéfinition d'un opérateur (exemple : opérateur ++ pour simuler l'arrivée d'une voiture)
    Parking& operator++() {
        if (currentCars < maxPlaces) {
            currentCars++;
            eventLog.push_back({"Arrivée simulée", getCurrentTime(), getCurrentDate()});
        }
        return *this;
    }

private:
    int maxPlaces;
    int currentCars;

    int servoInPin, servoOutPin;
    int ultrasonicInPin, ultrasonicOutPin;

    Servo servoIn, servoOut;
    rgb_lcd lcd;

    std::vector<Event> eventLog;

    long measureDistance(int pin);
    void updateLCD();
    void handleEntry();
    void handleExit();
    void handleError(String message);
    void envoyerNotification(String message);
    void checkWiFiConnection();
    String getCurrentTime();
    String getCurrentDate();
    void manageEntryAndExit();
};

class MqttClient {
private:
    const char* server;
    const int port;
    const char* user;
    const char* password;
    WiFiClient wifiClient;
    PubSubClient mqttClient;

public:
    MqttClient(const char* server, int port, const char* user, const char* password);
    void connectMQTT();
    void publishData(const char* topic, const char* data);
    void loop();
};

#endif