#include <LiquidCrystal.h> //Dołączenie bilbioteki
#include <IRremote.h>
#include <DHT.h>

#define DHTPIN 12
#define DHTTYPE    DHT11
DHT dht(DHTPIN, DHTTYPE);

LiquidCrystal lcd(2, 3, 4, 5, 6, 7); //Informacja o podłączeniu nowego wyświetlacza
const int pinBlueDiod = 8;
const int pinRedDiod = 9;
IRrecv irrecv(13);


int code = -1;
bool LCD_On = true;
int monitor = 0;
int lastMonitorIndex = 3;

byte degreeSign = B11011111;
int temperature, maxTemperature = dht.readTemperature();
int minTemperature = 100;
int humidity = dht.readHumidity();

String lastUnderlayTime = "17:23";
String nextUnderlayTime = "19:23";
int timeToNextUnderlay = 22;

unsigned int woodAmount = 0;
unsigned int coalAmount = 0;

unsigned long lastMeasureExecutionTime, lastIRExecutionTime, lastChangeTimeToNextUnderlay = 0;
const unsigned long measuresInterval = 1000;
const unsigned long IRinterval = 500;
const unsigned long nextUnderlayInterval = 60000;

bool underlaying = false;
bool settingWood = false;
bool settingCoal = false;

void clearMonitor(){
  lcd.clear();
}

void showMonitor(){
  clearMonitor();
  Serial.print("Zmieniono obraz na: ");
  Serial.println(monitor);
  switch(monitor){
    case 0:
    // first row
      lcd.setCursor(0,0);
      lcd.print("IN ");
      lcd.print(temperature); //2
      lcd.print((char)degreeSign); //1
      lcd.print("C "); //1
      lcd.print("OUT ");
      lcd.print(humidity);
      lcd.print("%");
    // second row
      lcd.setCursor(0,1);
      lcd.print(timeToNextUnderlay);
      lcd.print(" MINS LEFT");
    break;
    case 1:
      lcd.setCursor(0,0);
      lcd.print("LAST: ");
      lcd.print(lastUnderlayTime);
      lcd.setCursor(0,1);
      lcd.print("NEXT: ");
      lcd.print(nextUnderlayTime);
    break;
    case 2:
      lcd.setCursor(0,0);
      lcd.print("  MIN   MAX");
      lcd.setCursor(0,1);

      lcd.print(" ");
      lcd.print(minTemperature);
      lcd.print((char)degreeSign); //1
      lcd.print("C "); //1
      lcd.print("  ");
      lcd.print(maxTemperature);
      lcd.print((char)degreeSign); //1
      lcd.print("C"); //1
    break;
    case 3:
      lcd.setCursor(0,0);
      lcd.print("OSTATNIE PODL.");
      lcd.setCursor(0,1);
      lcd.print("Drewna:");
      lcd.print(woodAmount);
      lcd.print(" Wegla:");
      lcd.print(coalAmount);
    break;
  }
}

void setMonitor(int m){
  if(monitor != m){
    monitor = m;
    showMonitor();
  }
}

// sprawdź temperature i zapisz do minimalnej lub maksymalnej odnotowanej tego dnia wartosci
void checkNewMinMaxTemperature(){
  if(temperature > maxTemperature){
    maxTemperature = temperature;
  }
  if(temperature < minTemperature){
    minTemperature = temperature;
  }
}

// odczytaj temperature i jesli jest inna od poprzednio zarejestrowanej, to odswiez to na ekranie
void readTemperatureAndHumidity(int temp, int hum){
  unsigned long currentMillis = millis();

  if (currentMillis - lastMeasureExecutionTime >= measuresInterval) {
    temperature = (int)dht.readTemperature();
    humidity = (int)dht.readHumidity();

    if(temp != temperature || hum != hum){
      SwitchDiod();
      if(temperature > temp) Serial.println("TO_ESP:Temperatura wzrosla z " + String(temp) + " na " + String(temperature) + "stopnie");
      if(temperature < temp) Serial.println("TO_ESP:Temperatura zmalala z " + String(temp) + " na " + String(temperature) + "stopnie");
      checkNewMinMaxTemperature();
      showMonitor();
      Serial.print("Nastapila zmiana odczytow: ");
      Serial.print(temperature);
      Serial.print(" *C ");
      Serial.print(humidity);
      Serial.println(" %");
    }

    lastMeasureExecutionTime = currentMillis;
  }
}

void SwitchDiod(){
  if(temperature >= 21){
    digitalWrite(pinBlueDiod, LOW); 
    digitalWrite(pinRedDiod, HIGH);
  }
  else {
    digitalWrite(pinRedDiod, LOW); 
    digitalWrite(pinBlueDiod, HIGH);
    timeToNextUnderlay = 0;
    Serial.println("TO_ESP:Temperatura spadla na poziom 21*C\rWymagane natychmiastowe podlozenie!");
  }
}

int CalcTimeForNextUnderlay(){
  return woodAmount * 20 + coalAmount * 60;
}
 void receiveDataFromESP(){
  if (Serial.available() > 0) {
    char terminator = '\n'; // Znak końca linii
    String receivedText = Serial.readStringUntil(terminator);
    Serial.println(receivedText);
    if (receivedText.startsWith("TO_ARDUINO:")) {
      Serial.print("FROM_ESP: ");
      String receivedMessage = receivedText.substring(11);
      Serial.println(receivedMessage);
      if(receivedMessage.startsWith("Godzina podlozenia")){
        lastUnderlayTime = receivedMessage.substring(19,24);
        nextUnderlayTime = addTime(lastUnderlayTime, CalcTimeForNextUnderlay());
        timeToNextUnderlay = CalcTimeForNextUnderlay();
        delay(1000);
        Serial.println("TO_ESP:Nastepne podlozenie przewidywane jest za: "+String(timeToNextUnderlay) + "minut \rCzyli o godzinie: "+String(nextUnderlayTime));
        showMonitor();
      }
    }
  }
}

void startUnderlay(){
  underlaying = true;
  settingWood = true;
  settingCoal = true;
  setWoodAmount();
  setCoalAmount();
  underlaying = false;
}

void setWoodAmount(){
  clearMonitor();
  woodAmount = 0;
  while(settingWood == true){
    reactToIRDuringUnderlay();
    lcd.setCursor(0,0);
    lcd.print("Ile drewienek?");
    // second row
    lcd.setCursor(0,1);
    lcd.print(woodAmount);
  }
}

void setCoalAmount(){
  clearMonitor();
  coalAmount = 0;
  while(settingCoal == true){
    reactToIRDuringUnderlay();
    lcd.setCursor(0,0);
    lcd.print("Ile wegla?");
    // second row
    lcd.setCursor(0,1);
    lcd.print(coalAmount);
  }
  clearMonitor();
  lcd.setCursor(0,0);
  lcd.print("Pomyslna akcja!");
  // second row
  lcd.setCursor(0,1);
  lcd.print("Drewna:");
  lcd.print(woodAmount);
  lcd.print(" Wegla:");
  lcd.print(coalAmount);
  delay(4000);
  showMonitor();
}

void reactToIRDuringUnderlay(){
  unsigned long currentMillis = millis();
  //Eliminacja podwojnych klikow
  if (currentMillis - lastIRExecutionTime >= IRinterval) {
    if(IrReceiver.decode()){
      code = IrReceiver.decodedIRData.command;
      if(code != 0){
        Serial.print("Underlay + Nacisnieto kod= ");
        Serial.println(code);
        switch(code){
          // 69 = czerwony wlacznik
          case 69:
            if(LCD_On){
              lcd.noDisplay();
              LCD_On = false;
            }
            else{
              lcd.display();
              LCD_On = true;
            }
          break;
          // ST/REPT
          case 13:
          // PLAY/PAUSE
          case 64:
            if( underlaying == true && settingWood == true){
              settingWood = false;
            }
            else if(underlaying == true && settingCoal == true){
              settingCoal = false;
            }
          break;
          // STRZALKA W PRAWO
          case 67:
          // STRZALKA W DOL
          case 9:
            if(settingWood == true) woodAmount++;
            else coalAmount++;
            clearMonitor();
          break;
          // STRZALKA W LEWO
          case 68:
          // STRZALKA W DOL
          case 7:
            if(settingWood == true){
              if(woodAmount > 0) woodAmount--;
            }
            else{
              if(coalAmount > 0) coalAmount--;
            }
            clearMonitor();
          break;
          // FUNC/STOP
          case 71:
            if(settingWood == true){
              woodAmount = woodAmount / 10;
            }
            else{
              coalAmount = coalAmount / 10;
            }
            clearMonitor();
          break;
        }
        lastIRExecutionTime = currentMillis;
      }
    }
  }
  IrReceiver.resume();
}

void reactToIR(){
  unsigned long currentMillis = millis();
  //Eliminacja podwojnych klikow
  if (currentMillis - lastIRExecutionTime >= IRinterval) {
    if(IrReceiver.decode()){
      code = IrReceiver.decodedIRData.command;
      if(code != 0){
        Serial.print("Nacisnieto kod= ");
        Serial.println(code);
        switch(code){
          // 7 = strzalka w gore
          case 7:
            if(monitor < lastMonitorIndex){
              monitor++;
            }
            else{
              monitor = 0;
            }
            showMonitor();
          break;
          // 9 = strzalka w dol
          case 9:
            if(monitor > 0){
              monitor--;
            }
            else{
              monitor = lastMonitorIndex;
            }
            showMonitor();
          break;
          // 69 = czerwony wlacznik
          case 69:
            if(LCD_On){
              lcd.noDisplay();
              LCD_On = false;
            }
            else{
              lcd.display();
              LCD_On = true;
            }
          break;
          // 64 = srodkowy play/stop
          case 64:
            lastIRExecutionTime = currentMillis;
            startUnderlay();
            Serial.println("TO_ESP:Podlozono pod kotlem\rIlosc drewna: " + String(woodAmount) + " Ilosc wegla: " + String(coalAmount));
          break;
          case 12:
            setMonitor(0);
          break;
          case 24:
            setMonitor(1);
          break;
          case 94:
            setMonitor(2);
          break;
        }
        lastIRExecutionTime = currentMillis;
      }
    }
  }
  IrReceiver.resume();
}


String addTime(String originalTime, int minutesToAdd) {
  // Rozdziel godzinę i minutę
  int originalHour = originalTime.substring(0, 2).toInt();
  int originalMinute = originalTime.substring(3).toInt();

  // Dodaj godziny i minuty
  int totalMinutes = originalHour * 60 + originalMinute + minutesToAdd;

  int newHour = (totalMinutes / 60) % 24;
  int newMinute = totalMinutes % 60;

  // Sformatuj wynik w postaci "hh:mm"
  String newTime = String(newHour / 10) + String(newHour % 10) + ":" +
                   String(newMinute / 10) + String(newMinute % 10);

  return newTime;
}

void changeTimeToNextUnderlay(){
 unsigned long currentMillis = millis();
  //Eliminacja podwojnych klikow
  if (currentMillis - lastChangeTimeToNextUnderlay >= nextUnderlayInterval && timeToNextUnderlay > 0) {
    timeToNextUnderlay--;
    Serial.print("Nowy czas to nastepnego podlozenia: ");
    Serial.println(timeToNextUnderlay);
    showMonitor();
    lastChangeTimeToNextUnderlay = currentMillis;
  }
}

void setup() {
  lcd.begin(16, 2); //Deklaracja typu
  Serial.begin(9600); //Wysyłanie
  dht.begin();
  IrReceiver.begin(13);
  showMonitor();
  pinMode(pinBlueDiod, OUTPUT);
  pinMode(pinRedDiod, OUTPUT);
}
 
void loop() {
  // Odbieranie danych
  if(underlaying != true){
    receiveDataFromESP();
    reactToIR();
    changeTimeToNextUnderlay();
    readTemperatureAndHumidity(temperature, humidity);
  }
}