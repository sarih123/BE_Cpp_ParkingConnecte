#ifndef PARKING_H
#define PARKING_H

#include <Servo.h>
#include <rgb_lcd.h>
#include <vector>
#include <String>

struct Event {
    String description;
    String time;
    String date;
};

class Parking {
public:
    Parking(int servoInPin, int ultrasonicInPin, int servoOutPin, int ultrasonicOutPin, int maxPlaces);
    
    void setup();
    void loop();
    String generateHTML(); // Générer l'interface HTML

    // Getters pour l'accès aux données
    int getCurrentCars();
    int getMaxPlaces();
    std::vector<Event> getEventLog();

private:
    int maxPlaces;
    int currentCars;

    int servoInPin, servoOutPin;
    int ultrasonicInPin, ultrasonicOutPin;

    Servo servoIn, servoOut;
    rgb_lcd lcd;

    std::vector<Event> eventLog; // Journal des événements

    long measureDistance(int pin);
    void updateLCD();
    void handleEntry();
    void handleExit();
    void handleError(String message);
    void envoyerNotification(String message); // Déclaration de la fonction pour envoyer une notification
    void checkWiFiConnection(); // Déclaration de la fonction pour vérifier la connexion WiFi
    String getCurrentTime();
    String getCurrentDate();

    // Nouvelle méthode pour gérer à la fois l'entrée et la sortie
    void manageEntryAndExit(); 
};

#endif