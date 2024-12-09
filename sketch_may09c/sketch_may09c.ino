#include "Parking.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <time.h>

// Définition des broches
#define SERVO_IN_PIN D3       // Servo pour l'entrée
#define ULTRASONIC_IN_PIN D7  // Capteur ultrasonique pour l'entrée
#define SERVO_OUT_PIN D5      // Servo pour la sortie
#define ULTRASONIC_OUT_PIN D8 // Capteur ultrasonique pour la sortie

// Création de l'objet Parking
Parking parking(SERVO_IN_PIN, ULTRASONIC_IN_PIN, SERVO_OUT_PIN, ULTRASONIC_OUT_PIN, 9);

// Configuration du WiFi et du serveur Web
const char* ssid = "iPhone de Hassan";
const char* password = "azerty12345";
ESP8266WebServer server(80);

// Gestion de la page principale (interface HTML)
void handleRoot() {
    String html = R"====(
    <!DOCTYPE html>
    <html>
    <head>
        <meta charset="UTF-8">
        <title>Parking Intelligent</title>
        <style>
            body {
                font-family: Arial, sans-serif;
                background-color: #f0f4f8;
                color: #333;
                margin: 0;
                padding: 0;
                text-align: center;
            }
            h1 {
                background-color: #4CAF50;
                color: white;
                padding: 20px;
                margin: 0;
            }
            .content {
                padding: 20px;
            }
            .stats {
                display: flex;
                justify-content: center;
                align-items: center;
                gap: 20px;
                margin-bottom: 20px;
            }
            .stats div {
                background-color: #ffffff;
                border-radius: 10px;
                padding: 20px;
                box-shadow: 0px 4px 6px rgba(0, 0, 0, 0.1);
                text-align: center;
            }
            table {
                margin: 0 auto;
                width: 80%;
                border-collapse: collapse;
                margin-top: 20px;
            }
            th, td {
                border: 1px solid #ddd;
                padding: 10px;
            }
            th {
                background-color: #4CAF50;
                color: white;
            }
            tr:nth-child(even) {
                background-color: #f9f9f9;
            }
            tr:nth-child(odd) {
                background-color: #ffffff;
            }
            tr:hover {
                background-color: #f1f1f1;
            }
            .car-image {
                display: block;
                margin: 20px auto;
                width: 100px;
            }
        </style>
        <script>
            function updateData() {
                fetch('/data')
                    .then(response => response.json())
                    .then(data => {
                        document.getElementById('currentCars').innerText = data.currentCars;
                        document.getElementById('maxPlaces').innerText = data.maxPlaces;
                        document.getElementById('currentTime').innerText = data.currentTime; // Mise à jour de l'heure

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
        <div class="content">
            <div class="stats">
                <div>
                    <img src="http://www.captain-drive.com/wp-content/uploads/2014/12/Lamborghini-Gallardo.jpg" alt="Voiture" class="car-image">
                    <h2>Voitures Actuelles</h2>
                    <p id="currentCars" style="font-size: 2em; font-weight: bold;"></p>
                </div>
                <div>
                    <h2>Capacité Maximale</h2>
                    <p id="maxPlaces" style="font-size: 2em; font-weight: bold;"></p>
                </div>
            </div>
            <h2>Journal des Événements</h2>
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
            <div>
                <h2>Heure Actuelle</h2>
                <p id="currentTime" style="font-size: 2em; font-weight: bold;"></p> <!-- Affichage de l'heure actuelle -->
            </div>
        </div>
    </body>
    </html>
    )====";

    server.send(200, "text/html", html);
}

// Fonction pour obtenir l'heure actuelle
String getCurrentTime() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return "Impossible de récupérer l'heure";
    }

    char timeStr[20];
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo); // Formater l'heure au format HH:MM:SS
    return String(timeStr);
}

// Gestion des données en temps réel
void handleData() {
    String json = "{";
    json += "\"currentCars\":" + String(parking.getCurrentCars()) + ",";
    json += "\"maxPlaces\":" + String(parking.getMaxPlaces()) + ",";
    json += "\"eventLog\":[";
    for (size_t i = 0; i < parking.getEventLog().size(); i++) {
        if (i > 0) json += ",";
        json += "{\"description\":\"" + parking.getEventLog()[i].description + "\","; 
        json += "\"time\":\"" + parking.getEventLog()[i].time + "\","; 
        json += "\"date\":\"" + parking.getEventLog()[i].date + "\"}"; 
    }
    json += "],";
    json += "\"currentTime\":\"" + getCurrentTime() + "\""; // Ajouter l'heure actuelle dans la réponse
    json += "}";
    server.send(200, "application/json", json);
}

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connexion WiFi...");
    }

    Serial.println("Connecté au WiFi !");
    Serial.print("Adresse IP : ");
    Serial.println(WiFi.localIP());

    configTime(0, 0, "pool.ntp.org"); // Synchronisation NTP pour l'heure

    server.on("/", handleRoot);
    server.on("/data", handleData);
    server.begin();

    parking.setup();
}

void loop() {
    server.handleClient();
    parking.loop();
}
