#include "Parking.h"
#include <time.h>
#include <ESP8266WiFi.h>
#include <String>
#include <stdexcept> // Inclure la bibliothèque pour les exceptions standard

// Variables pour suivre les demandes simultanées d'entrée et de sortie
bool entryRequested = false;
bool exitRequested = false;

// Afficher l'erreur sur l'écran LCD et la console
void Parking::handleError(String message) {
    Serial.println(message);
    lcd.clear();
    lcd.setRGB(255, 0, 0);  // Texte rouge pour signaler l'erreur
    lcd.setCursor(0, 0);
    lcd.print("Erreur: " + message);
    delay(1000);  // Attente prolongée pour que l'utilisateur puisse lire l'erreur
    lcd.clear();  // Effacer l'écran
    updateLCD();  // Mettre à jour l'affichage normal
}

// Vérifie la connexion Wi-Fi et reconnecte si nécessaire
void Parking::checkWiFiConnection() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Déconnexion WiFi détectée. Tentative de reconnexion...");

        // Affiche sur l'écran LCD
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Wifi non connecte");
        lcd.setCursor(0, 1);
        lcd.print("Reconnexion...");

        WiFi.reconnect();
        int attemptCount = 0;

        // Indicateur de tentative
        while (WiFi.status() != WL_CONNECTED && attemptCount < 10) {
            delay(500);
            Serial.print(".");
            lcd.setCursor(14, 1);  // Affiche le numéro de tentative
            lcd.print(attemptCount + 1);
            attemptCount++;
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("Reconnexion WiFi réussie.");

            // Mise à jour de l'affichage LCD
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("WiFi reconnecte");
            lcd.setCursor(0, 1);
            lcd.print(WiFi.localIP());
        } else {
            Serial.println("Échec de la reconnexion WiFi après 10 tentatives.");

            // Affichage d'erreur sur LCD
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("WiFi erreur !");
            lcd.setCursor(0, 1);
            lcd.print("Verifier config.");

            // Lancer une exception
            throw std::runtime_error("Pas de connexion WiFi");
        }

        delay(3000);  // Affiche l'état final pendant 3 secondes
        lcd.clear();  // Nettoie l'écran
    }
}

Parking::Parking(int servoInPin, int ultrasonicInPin, int servoOutPin, int ultrasonicOutPin, int maxPlaces) {
    this->servoInPin = servoInPin;
    this->ultrasonicInPin = ultrasonicInPin;
    this->servoOutPin = servoOutPin;
    this->ultrasonicOutPin = ultrasonicOutPin;
    this->maxPlaces = maxPlaces;
    this->currentCars = 0;
}

void Parking::setup() {
    pinMode(ultrasonicInPin, OUTPUT);
    pinMode(ultrasonicOutPin, OUTPUT);

    servoIn.attach(servoInPin);
    servoOut.attach(servoOutPin);

    servoIn.write(0);  // Barrière d'entrée fermée
    servoOut.write(0); // Barrière de sortie fermée

    lcd.begin(16, 2);
    lcd.setRGB(0, 255, 0);  // Texte vert par défaut
    lcd.setCursor(0, 0);
    lcd.print("Parking Intell.");
    delay(2000);
    lcd.clear();
}

void Parking::loop() {
    delay(100);
    try {
        checkWiFiConnection();
    } catch (const std::runtime_error& e) {
        // Gestion de l'erreur WiFi
        handleError(e.what());
        // Ici, vous pouvez choisir de réessayer plus tard ou simplement continuer
        // afin que le programme ne reste pas bloqué
    }

    manageEntryAndExit();  // Nouvelle gestion combinée
    updateLCD();
}

void Parking::manageEntryAndExit() {
    long distanceIn = measureDistance(ultrasonicInPin);
    long distanceOut = measureDistance(ultrasonicOutPin);

    // Détection d'entrée
    if (distanceIn > 0 && distanceIn <= 5 && !entryRequested) {
        if (currentCars >= maxPlaces) {
            handleError("Parking plein - Entrée refusée");
        } else {
            entryRequested = true;
        }
    }

    // Détection de sortie
    if (distanceOut > 0 && distanceOut <= 5 && !exitRequested && currentCars > 0) {
        exitRequested = true;
    }

    // Affichage "0 voiture" si aucune voiture détectée devant les barrières
    if (distanceIn == 0 && distanceOut == 0 && currentCars == 0) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.setRGB(0, 255, 0);  // Texte vert
        lcd.print("Voitures en attente: 0");
        handleError("0 car");
    }

    // Exécute les actions simultanément si nécessaire
    if (entryRequested || exitRequested) {
        if (entryRequested) {
            servoIn.write(90);  // Ouvrir barrière d'entrée
            currentCars++;
            eventLog.push_back({"Entrée", getCurrentTime(), getCurrentDate()});
        }
        if (exitRequested) {
            servoOut.write(90); // Ouvrir barrière de sortie
            currentCars--;
            eventLog.push_back({"Sortie", getCurrentTime(), getCurrentDate()});
        }

        // Attente pour laisser les voitures passer
        delay(5000);

        // Fermer les barrières
        if (entryRequested) {
            servoIn.write(0);
            entryRequested = false;
        }
        if (exitRequested) {
            servoOut.write(0);
            exitRequested = false;
        }
    }
}

long Parking::measureDistance(int pin) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    delayMicroseconds(2);
    digitalWrite(pin, HIGH);
    delayMicroseconds(10);
    digitalWrite(pin, LOW);

    pinMode(pin, INPUT);
    long duration = pulseIn(pin, HIGH, 30000);
    return duration * 0.034 / 2;
}

void Parking::updateLCD() {
    lcd.clear();

    if (currentCars == 0) {
        lcd.setRGB(255, 0, 0);  // Texte rouge pour "0 car"
        lcd.setCursor(0, 0);
        lcd.print("Voitures: 0");
        lcd.setCursor(0, 1);
        lcd.print("Parking Vide");
    } else if (currentCars >= maxPlaces) {
        lcd.setRGB(255, 0, 0);  // Texte rouge pour "Parking Plein"
        lcd.setCursor(0, 0);
        lcd.print("Parking Plein!");
    } else {
        lcd.setRGB(0, 255, 0);  // Texte vert pour places disponibles
        lcd.setCursor(0, 0);
        lcd.print("Places libres: ");
        lcd.setCursor(0, 1);
        lcd.print(maxPlaces - currentCars);
    }
}

String Parking::generateHTML() {
    String html = R"====(
    <!DOCTYPE html>
    <html>
    <head>
        <title>Parking Intelligent</title>
        <script>
            function updateData() {
                fetch('/data')
                    .then(response => response.json())
                    .then(data => {
                        document.getElementById('currentCars').innerText = data.currentCars;
                        document.getElementById('maxPlaces').innerText = data.maxPlaces;

                        const logTable = document.getElementById('eventLog');
                        logTable.innerHTML = ''; // Clear previous entries
                        data.eventLog.forEach(event => {
                            const row = `<tr>
                                <td>${event.description}</td>
                                <td>${event.time}</td>
                                <td>${event.date}</td>
                            </tr>`;
                            logTable.innerHTML += row;
                        });
                    })
                    .catch(error => console.error('Erreur lors de la mise à jour des données:', error));
            }

            setInterval(updateData, 5000);
            window.onload = updateData;
        </script>
    </head>
    <body>
        <h1>Parking Intelligent</h1>
        <p>Nombre de places : <strong id="maxPlaces"></strong></p>
        <p>Voitures actuelles : <strong id="currentCars"></strong></p>
        <table>
            <thead>
                <tr>
                    <th>Événement</th>
                    <th>Heure</th>
                    <th>Date</th>
                </tr>
            </thead>
            <tbody id="eventLog"></tbody>
        </table>
    </body>
    </html>
    )====";
    return html;
}

String Parking::getCurrentTime() {
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    char buffer[10];
    strftime(buffer, sizeof(buffer), "%H:%M", timeinfo);
    return String(buffer);
}

String Parking::getCurrentDate() {
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    char buffer[12];
    strftime(buffer, sizeof(buffer), "%d/%m/%Y", timeinfo);
    return String(buffer);
}

int Parking::getCurrentCars() {
    return currentCars;
}

int Parking::getMaxPlaces() {
    return maxPlaces;
}

std::vector<Event> Parking::getEventLog() {
    return eventLog;
}

// Implémentation de MqttClient
MqttClient::MqttClient(const char* server, int port, const char* user, const char* password)
    : server(server), port(port), user(user), password(password), mqttClient(wifiClient) {
    mqttClient.setServer(server, port);
}

void MqttClient::connectMQTT() {
    while (!mqttClient.connected()) {
        Serial.println("Connexion au serveur MQTT...");
        String clientId = "ESP8266Client-";
        clientId += String(random(0xffff), HEX);
        if (mqttClient.connect(clientId.c_str(), user, password)) {
            Serial.println("Connecté au serveur MQTT !");
        } else {
            Serial.print("Échec, rc=");
            Serial.println(mqttClient.state());
            delay(5000);
        }
    }
}

void MqttClient::publishData(const char* topic, const char* data) {
    char payload[50];
    snprintf(payload, sizeof(payload), "%s", data); // Pas de conversion, car 'data' est déjà une chaîne
    mqttClient.publish(topic, payload);
    Serial.print("Publié sur ");
    Serial.print(topic);
    Serial.print(" : ");
    Serial.println(payload);
}

void MqttClient::loop() {
    mqttClient.loop();
}
