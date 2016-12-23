//////////////////////////////////////////////
///  The project for No pain gun.
///  Edit by Yi-chun Hung, 2016/12/23
///
//////////////////////////////////////////////

#include <ArduinoJson.h>
#include <PCD8544.h>

///////////////////////////
/// Define the input and output
/// 
///////////////////////////
#define trigger A0
#define LED A1
#define Photopin A2
#define DataRequest A3

//////////////////////////
/// LCD module and parameter
/// DO NOT CHANGE!!!!
//////////////////////////
static const byte plyph[] = { B00010000, B00110100, B00110000, B00110100, B00010000 };
static PCD8544 lcd;

///////////////////////////////////////
/// When been shot, photovalue is high
///////////////////////////////////////
int photovalue;


////////////////////////////////////////
/// To control duration of one pulse,
/// Trigger buttom and data request button
/////////////////////////////////////////
int last_time_trigger= 0;
int now_time_trigger = 0;
int last_time_data = 0;
int now_time_data = 0;



/////////////////////////
/// Realize one pulse.
///
/////////////////////////
int DataRequestState = 0;
int NowDataRequestPush = 0;
int LastDataRequestPush = 0;
int push = 0;
int push_last = 0;


////////////////////////////
/// The initial value of JSON
///
///////////////////////////
int const Name = 0;
int Blood = 100;
int Bullet = 100;
int Request = 0;

///////////////////////////
/// Json encoding memory.
/// DO NOT CHANGE
///////////////////////////
StaticJsonBuffer<200> jsonBuffer;
JsonObject& root = jsonBuffer.createObject();


void setup()
{
  Serial.begin(9600);
  Serial1.begin(57600);
  pinMode(trigger,INPUT);  
  pinMode(LED,OUTPUT);
  pinMode(DataRequest,OUTPUT);
  
  lcd.begin(84, 48);
  lcd.createChar(0,plyph);
  
  ////////////////////
  /// Structure of Json
  ///
  ///////////////////
  root["name"] = Name;
  root["blood"] = Blood;
  root["bullet"] = Bullet;
  root["request"] = Request;
}

void loop()
{
  push_last = push;
  push = digitalRead(trigger);
  
  now_time_trigger = millis();
  
  ///////////////////////////
  /// Trigger function
  ///
  ///////////////////////////
  if(push == 0 && push_last == 1){    ///Falling edge trigger  
      if((now_time_trigger - last_time_trigger) > 300){  
                                      ///Duration of tigger one pulse
        digitalWrite(LED,1);          ///Trigger the bullet(lazer).
        Bullet--;
        root["bullet"] = Bullet;      ///Update the bullet number to server
        root["request"] = -1;         ///Do not request any info
        last_time_trigger = now_time_trigger;  ///Reset the state of trigger
        root.printTo(Serial1);        ///Send Json to MT7688
        Serial1.print("\n");
      }
  }
  else{ 
    digitalWrite(LED,0);              ///No trigger, no bullet  
  }
  
  
  //////////////////////////
  /// Data request button function
  ///
  //////////////////////////
  
  LastDataRequestPush = NowDataRequestPush;
  NowDataRequestPush = digitalRead(DataRequest);
  now_time_data = millis();
  
  if(NowDataRequestPush ==0 && LastDataRequestPush ==1){
                                              ///Falling edge trigger
      if((now_time_data - last_time_data) > 300){  ///Duration of changing request
        DataRequestState = (DataRequestState + 1) % 6;    ///Update the request
        last_time_data = now_time_data;
        root["request"] = DataRequestState ;        ///Change data of Json
        root.printTo(Serial1);                      ///Send to MT7688
        Serial1.print("\n");
        
        ////////////////////
        /// Encode the Json from MT7688
        ///
        /////////////////////
        String str = Serial1.readString();
        StaticJsonBuffer<200> jsonBufferLCD;
        JsonObject& rootLCD = jsonBufferLCD.parseObject(str);
        
        ///////////////////////////
        /// Update the LCD info
        ///
        ///////////////////////////
        lcd.setCursor(0,0);                ///First line
        lcd.print("Name:");
        String nameprint = rootLCD["name"];
        lcd.print(nameprint);

        lcd.setCursor(0,1);                ///Second line
        lcd.print("Blood:");
        String bloodprint = rootLCD["blood"];
        lcd.print(bloodprint);

        lcd.setCursor(0,2);                ///Third line
        lcd.print("Bullet:");
        String bulletprint = rootLCD["bullet"];
        lcd.print(bulletprint);
        
        delay(10);
      }
      else{
        DataRequestState = DataRequestState;
      }
  }
  else{
    DataRequestState = DataRequestState;
  }
  
  delay(10);
  
  
}


