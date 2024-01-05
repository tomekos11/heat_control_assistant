#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "Poco F5";
const char* password = "123";
const char* discordServerAddress = "https://discord.com/api/webhooks/1180942362697007174/FsqEZ-OewmxBlL80kcGlFR5AyVUGpTd-ZpWq06tHh6w-v1hMrapcOjZZCa5R7y0Sh6WD";  // Adres serwera docelowego
const char* timeServerAddress = "http://worldtimeapi.org/api/timezone/Europe/Warsaw";
const int serverPort = 80;  // Port serwera HTTP

void receiveDataFromArduino(){
  if (Serial.available() > 0) {
    char terminator = '\n'; // Znak końca linii
    String receivedText = Serial.readStringUntil(terminator);
    if (receivedText.startsWith("TO_ESP:")) {
      String receivedMessage = receivedText.substring(7);
      Serial.print("FROM_ARDUINO: ");
      Serial.println(receivedMessage);

      sendHttpPostRequest(receivedMessage);
      if(receivedMessage.startsWith("Podlozono pod kotlem")){
        getTimeRequest();
      }
    }
  }
}

void getTimeRequest(){
  // Utwórz obiekt klienta HTTP
  HTTPClient http;
  if (!WiFi.status()== WL_CONNECTED){
    Serial.println("Nie ma polaczenia z WiFi - nie mozna wykonac getTimeRequest()");
  }
  // Połącz z serwerem
  if (http.begin(timeServerAddress)) {
    Serial.println("Wykonuje getTimeRequest()");

    // Skonfiguruj nagłówki
    http.addHeader("Content-Type", "application/json");

    // Wyślij żądanie POST
    int httpResponseCode = http.GET();

    // Odczytaj odpowiedź z serwera
    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);

      String response = http.getString();
      Serial.print("Response: ");
      Serial.println(response);

      StaticJsonDocument<200> doc;
      deserializeJson(doc, response);
      String datetimeValue = doc["datetime"];
      datetimeValue = datetimeValue.substring(11, 19);

      Serial.print("TO_ARDUINO:Godzina podlozenia=");
      Serial.println(datetimeValue);
    } else {
      Serial.print("Funkcja getTimeRequest() zakonczyla sie bledem: ");
      Serial.println(httpResponseCode);
    }

    http.end();

  } else {
    Serial.println("Blad podczas wykonywania funkcji getTimeRequest()");
  }
} 

void sendHttpPostRequest(String content) {
  // Utwórz obiekt klienta HTTP
  HTTPClient http;
  if (!WiFi.status()== WL_CONNECTED){
    Serial.println("Nie ma polaczenia z WiFi - nie mozna wykonac sendHttpPostRequest()");
  }
  // Połącz z serwerem
  if (http.begin(discordServerAddress)) {
    Serial.println("Wykonuje sendHttpPostRequest()");

    // Skonfiguruj nagłówki
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<200> doc;
    doc["content"] = content;

    String postData;
    serializeJson(doc, postData);

    // Wyślij żądanie POST
    int httpResponseCode = http.POST(postData);

    // Odczytaj odpowiedź z serwera
    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Funkcja sendHttpPostRequest() zakonczyla sie bledem: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("Blad podczas wykonywania funkcji sendHttpPostRequest()");
  }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print("Trwa laczenie\n");
  }

  sendHttpPostRequest("Połączono z siecią WiFi!");
  Serial.println("Połączono z siecią WiFi!");
  sendHttpPostRequest("Adres IP: " + WiFi.localIP().toString());
  Serial.println("Adres IP: " + WiFi.localIP().toString());
}

void loop() {
  receiveDataFromArduino();
}
