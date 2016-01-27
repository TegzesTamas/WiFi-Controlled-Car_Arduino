#include <SoftwareSerial.h>

#define ESP_RX 8 //Connected to the TX pin of the ESP8266
#define ESP_TX 7 //Connected to the RX pin of the ESP8266

//Change these to correspond with the data you use to connect to your home wifi network!
#define WIFI_SSID <SSID of your wifi>
#define WIFI_PASSWORD <Password to your wifi>

SoftwareSerial esp(ESP_RX,ESP_TX);

String sendCommand(String command, int ETA){
  String response = "";
  do{
    esp.flush();
    esp.println(command);
    delay(ETA);
    while(!esp.available());
    delay(100);
    while(esp.available()){
      char c = esp.read();
      response = response + String(c);
    }
    Serial.print(response);
    if(response.indexOf("busy p...") != (-1))
      delay(ETA);
  }while((response.indexOf(command) == (-1)) || (response.indexOf("OK") == (-1)));
  return response;
}
boolean sendData(String data, int cNum){
  int pos = 0;
  while(data.length()-pos > 32){
    sendCommand("AT+CIPSEND="+String(cNum)+String(',')+String(32)+data.substring(pos,pos+32),1000);
    pos = pos+32;
    delay(100);
    esp.flush();
  }
  sendCommand("AT+CIPSEND="+String(cNum)+String(',')+String(data.length()-pos),1000);
  esp.print(data.substring(pos));
  delay(100);
  esp.flush();
}
void setup() {
  esp.begin(9600);
  Serial.begin(9600);
  pinMode(13,OUTPUT);
  digitalWrite(13,HIGH);
  sendCommand("AT+RST",1000);
  sendCommand("AT",100);
  Serial.println("ESP Seems fine");
  Serial.println("Attempting to connect to home WiFi");
  sendCommand("AT+CWMODE=1",100);
  sendCommand("AT+CWJAP="\""+WIFI_SSID+"\",\""+WIFI_PASSWORD+"\"",5000);
  Serial.println("Connected");
  Serial.println("Starting server");
  sendCommand("AT+CIPMUX=1",100);
  sendCommand("AT+CIPSERVER=1,80",1000);
  Serial.println("Server started");
  digitalWrite(13,LOW);
}

int processConnection(){
  delay(500);
  if(esp.find("+IPD,")){
    Serial.println("Probably a connection");
    char c = esp.read();
    Serial.print("Number appears to be: ");
    Serial.println(c);
    esp.flush();
    return ((int)c) - ((int)'0');
  }
  esp.flush();
  Serial.println("Probably not a connection");
  return -1;
}

void sendWebsite(int cNum){
  Serial.println("Sending website");
  String website = "<html><body><a href=../forward>forward</a><br><a href=../backward>backward</a></body></html>";
  sendCommand("AT+CIPSEND="+String(cNum)+String(',')+String(website.length()),100);
  sendData(website,cNum);
  Serial.println("Sent");
  esp.flush();
}

void loop() {
  while(!esp.available());
  Serial.println("Possible Connection");
  digitalWrite(13,HIGH);
  int cNum = processConnection();
  while(esp.available())
    Serial.write(esp.read());
  if(cNum >=0 && cNum <=4)
    sendWebsite(cNum);
    //sendCommand("AT+CIPCLOSE="+String(cNum),1000);
  esp.flush();
  digitalWrite(13,LOW);
    
}
