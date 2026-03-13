#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoHA.h>

#include <IRremote.h>
#include "MyConfig.h"
#include "SBTN.h"

#define ARDUINOHA_DEBUG



SBTN btn1(PIN_BTN1);
SBTN btn2(PIN_BTN2);
SBTN btn3(PIN_BTN3);
//SBTN btn4(PIN_BTN4);

WiFiClient wifiClient;
//PubSubClient mqtt(wifiClient);

HADevice device;
HAMqtt mqtt(wifiClient, device);
//HAMqtt ha(mqtt, device);

//HAButton h_btn1("button_1");
//HAButton h_btn2("button_2");
//HAButton h_btn3("button_3");

HASwitch switch1("ON_OFF"); //Кома
HASwitch switch2("Full");
HASwitch switch3("Medium");
HASwitch switch4("Low");
HASwitch switch5("Warm");
HASwitch switch6("Neutral");
HASwitch switch7("Cool");



bool isIrReceive = false;
//bool flag = false;
bool is_mqtt = false;
bool is_wifi = false;
LIGHT_BRIGHTNESS_t flag_br = LBT_FULL;
LIGHT_COLOR_t flag_color = LCT_NEUTRAL;
  

const int buffer_size = 100;
char recieved_codes[buffer_size];
char str[50];
char code_word[10] = "command:";
unsigned long code;
int char_index = 0;


void WiFiEvent(WiFiEvent_t event);
void onSwitchCommand(bool state, HASwitch* sender);
void cmdIR1(uint8_t cmd);
void cmdIR2(uint8_t cmd1, uint8_t cmd2);
void cmdIR(uint8_t cmd1, uint8_t cmd2=0xff);
void onMqttConnected();
void onMqttDisconnected();
void onMqttStateChanged(HAMqtt::ConnectionState state);
void ha_setup();

void setup(){
  Serial.begin(115200);
  vTaskDelay(1000);
  Serial.println("Start IR blaster");

  pinMode(PIN_BTN2,INPUT_PULLUP);
  pinMode(PIN_IR_OUT,OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED,LOW);

// Button init
  btn1.setTimer(2000);
//  btn2.setTimer(3000);
//  btn3.setTimer(3000);





  IrReceiver.begin(PIN_IR_IN, ENABLE_LED_FEEDBACK, USE_DEFAULT_FEEDBACK_LED_PIN);
  IrSender.begin(PIN_IR_OUT); // Specify send pin and enable feedback LED at }
//  IrSender.begin(); // Start with IR_SEND_PIN -which is defined in PinDefinitionsAndMore.h- as send pin and enable feedback LED at default feedback LED pin
  disableLEDFeedback(); // Disable feedback LED at default feedback LED pin


//  xTaskCreateUniversal(taskNet, "net", 10000, NULL, 3, NULL, CORE);
//  vTaskDelay(500);
//  xTaskCreateUniversal(taskButton, "btn", 4096, NULL, 4, NULL,CORE);

   WiFi.onEvent(WiFiEvent);
   WiFi.mode(WIFI_STA);
   WiFi.begin(WIFI_NAME, WIFI_PASS);

   Serial.println(F("!!! WiFi Connecting..."));
   
// Home Assistant device 
   byte mac[6];
   WiFi.macAddress(mac);
   device.setUniqueId(mac, sizeof(mac));   
   device.setName("ESP32C3 IR пульт");
   device.setManufacturer("DIY");
   device.setModel("ESP32C3-IR-3BTN");

   switch1.setName("ON/OFF");
   switch1.setIcon("mdi:lightbulb");
   switch1.onCommand(onSwitchCommand);

   switch2.setName("LIGHT 100%");
   switch2.setIcon("mdi:lightbulb");
   switch2.onCommand(onSwitchCommand);    

   switch3.setName("LIGHT 50%");
   switch3.setIcon("mdi:lightbulb");
   switch3.onCommand(onSwitchCommand);    

   switch4.setName("LIGHT 20%");
   switch4.setIcon("mdi:lightbulb");
   switch4.onCommand(onSwitchCommand);

   switch5.setName("LIGHT WARM");
   switch5.setIcon("mdi:lightbulb");
   switch5.onCommand(onSwitchCommand);    

   switch6.setName("LIGHT NEUTRAL");
   switch6.setIcon("mdi:lightbulb");
   switch6.onCommand(onSwitchCommand);

   switch7.setName("LIGHT COOL");
   switch7.setIcon("mdi:lightbulb");
   switch7.onCommand(onSwitchCommand);
}




uint8_t cmd = 0;

void loop(){
//   if( mqtt.isConnected() ){
//      if( is_mqtt == false ){
//         Serial.println("!!! MQTT connected");
//         is_mqtt = true;
//      }
      mqtt.loop();
//   }
   switch( btn1.loop() ){
      case SB_TIMER :
         Serial.println("!!! Start receiver 1 channel");
         IrReceiver.start(); 
         isIrReceive = true;
         break;
      case SB_RELEASE :
         Serial.println("!!! Stop receiver");
         IrReceiver.stop(); 
         isIrReceive = false;
         break;
      case SB_PRESS :
         Serial.println("!!! Press 1");

         cmdIR(0x46);
         break;
   }
   switch( btn2.loop() ){
      case SB_PRESS :
         Serial.println("!!! Press 2");
         switch(flag_br){
            case LBT_FULL:   flag_br = LBT_LOW;    cmdIR(0x15); break;
            case LBT_LOW:    flag_br = LBT_MEDIUM; cmdIR(0x53); break;
            case LBT_MEDIUM: flag_br = LBT_FULL;   cmdIR(0x52); break;
         } 
         break;
   }
   switch( btn3.loop() ){
      case SB_PRESS :
         Serial.println("!!! Press 3");
         switch(flag_color){
           case LCT_WARM:    flag_color = LCT_NEUTRAL; cmdIR(0x5e); break;
           case LCT_NEUTRAL: flag_color = LCT_COOL;    cmdIR(0x18); break;
           case LCT_COOL:    flag_color = LCT_WARM;    cmdIR(0x0C); break;
         }
         break;
   }


//   USBSerial.println("USB test");
//   Serial.println("Serial test");
   if(isIrReceive)check_recieved();
}

// 0x46 --питание
// 0x45 - круг
// 0x47 - 30минут
// 0x40 - свет +
// 0x19 - свет -
// 0x07 - K-
// 0x09 - K+
// 0x1C - 20%
// 0x53 - 50%
// 0x52 - 100%
// 0x0C - 3000K
// 0x18 - 4000K
// 0x5e - 6500K



/*checks for IR codes picked up by the reciever*/
void check_recieved(){
  if(IrReceiver.decode()){
    if(IrReceiver.decodedIRData.decodedRawData != 0){
      IrReceiver.printIRResultShort(&Serial);
      if(IrReceiver.decodedIRData.decodedRawData == 0xFFFFFFFF){
        //Repeat command(used by some IR remotes
        //to indicate that a button has been held
        Serial.println("...");
      }else{
        //adds the code to a char array containing all of the recieved codes
        //as the client can only initiate connections with the 
        //server(the esp32), codes are only sent to the client after it requests them
        
        Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);
        
        sprintf(str,"%X",IrReceiver.decodedIRData.decodedRawData);
      
        for(int i = 0; str[i] != '\0'; i++){
          if(char_index < buffer_size){
            recieved_codes[char_index] = code_word[i];
            char_index++;
          }else{
            break;
          }
        }
      
        for(int j = 0; str[j] != '\0'; j++){
          if(char_index < buffer_size){
            recieved_codes[char_index] = str[j];
            char_index++;
          }else{
            break;
          }
        }
        
        if(char_index < buffer_size){
          recieved_codes[char_index] = '\n';
          char_index++;
        }
      }
    }
    IrReceiver.resume(); //Enable receiving of the next value
  }
}


void WiFiEvent(WiFiEvent_t event) {
    switch (event) {
        case ARDUINO_EVENT_WIFI_STA_START:
            Serial.println(F("!!! WiFi STA started"));
            break;

        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            Serial.println(F("!!! WiFi Connected to AP"));
            break;

        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            Serial.print(F("!!! WiFi IP address: "));
            Serial.println(WiFi.localIP());
            ha_setup();
            break;

        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            Serial.println(F("!!! WiFi Disconnected, retrying..."));
            WiFi.reconnect();
            break;

        default:
            break;
    }
}


void onSwitchCommand(bool state, HASwitch* sender){
//    if( state ){
       if(sender == &switch1) cmdIR(0x46);
       else if(sender == &switch2) {cmdIR(0x52); flag_br = LBT_FULL;       }
       else if(sender == &switch3) {cmdIR(0x53); flag_br = LBT_MEDIUM;     }
       else if(sender == &switch4) {cmdIR(0x15); flag_br = LBT_LOW;        }
       else if(sender == &switch5) {cmdIR(0x0C); flag_color = LCT_WARM;    }
       else if(sender == &switch6) {cmdIR(0x5e); flag_color = LCT_NEUTRAL; }
       else if(sender == &switch7) {cmdIR(0x18); flag_color = LCT_COOL;    }
//    }
    sender->setState(LOW); // report state back to the Home Assistant
}

void cmdIR(uint8_t cmd1, uint8_t cmd2){
   if( cmd2 == 0xff )cmdIR1(cmd1);
   else cmdIR2(cmd1, cmd2);
}


void cmdIR1(uint8_t cmd){
   digitalWrite(PIN_LED,HIGH);
   IrSender.sendNEC(0x00, cmd, 5);
   Serial.printf("!!! Send smd 0x%02x\n",(int)cmd);
   digitalWrite(PIN_LED,LOW);
}

void cmdIR2(uint8_t cmd1, uint8_t cmd2){
   digitalWrite(PIN_LED,HIGH);
   IrSender.sendNEC(0x00, cmd1, 5);
   delay(IR_DELAY);
   IrSender.sendNEC(0x00, cmd2, 5);
   Serial.printf("!!! Send smd2 0x%02x 0x%02x\n",(int)cmd1, (int)cmd2);
   digitalWrite(PIN_LED,LOW);
}


void ha_setup(){
   delay(1000);
   Serial.println("!!! MQTT begin");
   mqtt.onConnected(onMqttConnected);
   mqtt.onDisconnected(onMqttDisconnected);
   mqtt.onStateChanged(onMqttStateChanged);

   mqtt.begin(MQTT_ADDR, MQTT_USER, MQTT_PASS);
//   delay(1000);
//   mqtt.publish("/homeassistant/ir_blaster", "test");

}


void onMqttConnected() {
    Serial.println("!!! MQTT Connected!");
    switch1.setState(LOW);
    switch2.setState(LOW);
    switch3.setState(LOW);
    switch4.setState(LOW);
}

void onMqttDisconnected() {
    Serial.println("!!! MQTT Disconnected!");
}

void onMqttStateChanged(HAMqtt::ConnectionState state) {
    Serial.print("!!! MQTT state changed to: ");
    Serial.println(static_cast<int8_t>(state));
}