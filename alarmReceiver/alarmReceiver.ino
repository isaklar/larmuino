/*******************************************************************
 * LIBRARIES - contains
 * WIFI
 * KEYPAD
 * RF - (receiver & transmitter)
 * LCD
*******************************************************************/
#include <Keypad.h>
#include <VirtualWire.h>
#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
/*******************************************************************
 * INSTANCE VARIABLES - contains
 * WIFI
 * LCD
 * KEYPAD
 * GENERAL
 * FUNCITON DECLARATION
*******************************************************************/
// WIFI
const byte rxPin = 2; // Wire this to Tx Pin of ESP8266
const byte txPin = 3; // Wire this to Rx Pin of ESP8266
SoftwareSerial ESP8266 (rxPin, txPin);
#define WIFI_NAME     ""
#define WIFI_PASSWORD ""
#define SERVER_IP     ""
#define SERVER_PORT   ""
#define HALT          true
#define CONTINUE      false
#define TIMEOUT       5000 //ms = 5 sekunder

// LCD (LIQUIDCRYSTAL)
LiquidCrystal_I2C lcd(0x20,20,4);

// KEYPAD
#define Password_Length 7 // Give enough room for six chars + NULL char
char Data[Password_Length]; // 6 is the number of chars it can hold + the null char = 7
char Master[Password_Length] = "123456";
byte data_count = 0, master_count = 0;
bool Pass_is_good;
char customKey;

const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {2,3,4,5}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {10,9,8}; //connect to the column pinouts of the keypad
Keypad customKeypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS); //initialize an instance of class NewKeypad

// GENERAL
bool alarm_is_activated = false;
const int led_pin = 13;
const int receive_pin = 12;
int pinSpeaker = 10;

// PORT DECLARATION(TODO)


//FUNCITON DECLARATION
void activate_alarm();
void deactivate_alarm();
void clearData();
void trigger_alarm();
void triggerCountDown();
/*******************************************************************
 * SETUP - contains
 * WIFI
 * RF
 * LCD
*******************************************************************/
void setup() {

  // Setting up WIFI
  ESP8266.begin(9600);

  // Setting up RF
  vw_set_rx_pin(receive_pin);
  vw_setup(4000); // Transmission rate

  // Setting up LCD
  lcd.begin();// initialize the lcd
  lcd.backlight();
}

/*******************************************************************
 * LOOP
 * Utför allt som behövs för att det ska funka.
 * Där i ingår det att hantera funktioner.
*******************************************************************/
void loop() {
 // initialize the wifi-connection
  esp8266Init();
/*******************************************************************
 * Kod för keypad
 * kollar om lösenordet är rätt och sätter på alarmet.
 * TODO: Lägg till så att alla enheter har pin-platser
*******************************************************************/
  lcd.setCursor(0,0);
  lcd.print("Enter Password");

  customKey = customKeypad.getKey();
  if (customKey) // makes sure a key is actually pressed, equal to (customKey != NO_KEY)
  {
    Data[data_count] = customKey; // store char into data array
    lcd.setCursor(data_count,1); // move cursor to show each new char
    lcd.print(Data[data_count]); // print char at said cursor
    data_count++; // increment data array by 1 to store new char, also keep track of the number of chars entered
  }

  if(data_count == Password_Length-1) // if the array index is equal to the number of expected chars, compare data to master
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Password is ");

    if(!strcmp(Data, Master))
      { // equal to (strcmp(Data, Master) == 0)
        lcd.print("Good");
        if(!alarm_is_activated)
      {
        activate_alarm();
      }
      else
      {
        deactivate_alarm();
      }
     }
     else
     {
      lcd.print("Bad");
     }
    delay(1000);// added 1 second delay to make sure the password is completely shown on screen before it gets cleared.
    lcd.clear();
    clearData();
  }

/*******************************************************************
 * Kod för transmitter
 * Tar emot ett meddelande och utför handlingar beroende på resultat
 * TODO: Lägg till den gör ljud när man öppnar dörren.
*******************************************************************/
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;

  if (vw_get_message(buf, &buflen))
  {
    if(buf[0] = '1')
    {
      lcd.print("Motion detected!");
      trigger_alarm();
    }

    if(buf[0] = '2')
    {
      lcd.print("Door opened!");
      triggerCountDown();
    }
  }
}

/*******************************************************************
 * void clearData()
 * Tömmer arrayen på data, funkar för vilken array som helst
*******************************************************************/
  void clearData()
  {
    while(data_count != 0)
    {   // This can be used for any array size,
      Data[data_count--] = 0; //clear array for new data
    }
    return;
  }

/*******************************************************************
 * void activate_alarm()
 * Startar alarmet och tar då emot info från transmitters
*******************************************************************/
  void activate_alarm()
  {
    if(!alarm_is_activated)
    {
      lcd.print("Activating alarm in 30 seconds!");
      delay(30000);
      vw_rx_start(); // Start the receiver PLL
      alarm_is_activated = true;
      sendMail("Alarm is activated.");
    }
  }

/*******************************************************************
 * void deactivate_alarm()
 * stoppar alarmet och tar då inte emot info från transmitters
*******************************************************************/
  void deactivate_alarm()
  {
    if(alarm_is_activated)
    {
      lcd.print("Deactivating alarm!");
      vw_rx_stop();
      alarm_is_activated = false;
      sendMail("Alarm is deactivated.");
    }
  }

/*******************************************************************
 * void trigger_alarm()
 * triggar alarmet om rörelse märks av
 * TODO: Lägg till så att det gör ljud när alarmet går gång.
*******************************************************************/
  void trigger_alarm()
  {
    if(alarm_is_activated)
    {
      lcd.print("Alarm is triggered!");
      sendMail("Alarm is triggered!");
    }
  }

/*******************************************************************
 * void trigger_alarm()
 * Triggar alarmet om rörelse märks av.
 * TODO: Lägg till så att det går en countdown på lcdskärmen.
*******************************************************************/
  void triggerCountDown()
  {
    if(alarm_is_activated)
    {
      lcd.print("You have 30 seconds to disarm the alarm!");
      delay(30000);
      trigger_alarm();
    }
  }

/*******************************************************************
 * bool readResponseData(String keyword)
 * tar emot meddelandet från esp och tolkar
*******************************************************************/
  bool readResponseData(String keyword)
  {
    String response;
    long deadline = millis() + TIMEOUT;
    while(millis() < deadline)
    {
      if(ESP8266.available())
      {
        char ch = ESP8266.read();
        response += ch;
        if(keyword != "")
        {
          if(response.indexOf(keyword) > 0)
          {
            Serial.println(response);
            return true;
          }
        }
      }
    }
    Serial.println(response);
    return false;
  }

/*******************************************************************
 * bool sendCommand(String command, String ack, bool stopOnError)
 * Skickar meddelande i form av AT-kommand
 * command: vilket AT-kommando
 * ack: vad du får för svar tillbaka
 * stopOnError: Vad som ska hända efter utförande av kommando.
*******************************************************************/
bool sendCommand(String command, String ack, bool stopOnError)
{
  ESP8266.println(command);
  #ifdef ECHO_COMMANDS
  Serial.print("--");
  Serial.println(command);
  #endif

  if(!readResponseData(ack))
  {
    if(stopOnError)
    {
      exception(command + "Failed to execute.");
    }
    else
    {
      return false;
    }
  }
  else
  {
    return true;
  }
}

/*******************************************************************
 * void exception(String exception)
 * Tar hand om exceptions
*******************************************************************/
  void exception(String exception)
  {
    Serial.println(exception);
    Serial.println("HALT");
    while(true)
    {
      readResponseData("");
      delay(10000); // ska tydligen sättas till 60000
    }
  }

/*******************************************************************
 * void esp8266Init()
 * initializerar wifi-kortet
*******************************************************************/
  void esp8266Init()
  {
    ESP8266.begin(9600);
    ESP8266.setTimeout(TIMEOUT);
    delay(2000);

    sendCommand("AT+GMR", "OK", CONTINUE);
    sendCommand("AT+CWMODE?", "OK", CONTINUE);
    sendCommand("AT+CWMODE=1", "OK", HALT);
    sendCommand("AT+CIPMUX=1", "OK", HALT);

    String cmd = "AT+CWJAP=\""; cmd += WIFI_NAME; cmd += "\",\""; cmd += WIFI_PASSWORD; cmd +="\"";
    for(int i = 0; i < 5; i++)
    {
      if(sendCommand(cmd, "OK", CONTINUE))
      {
        Serial.println("Connected to wifi.");
        break;
      }
      else if(i == 4)
      {
        exception("Connection to wifi failed. ");
      }
      delay(TIMEOUT);
      sendCommand("AT+CWSAP=?", "OK", CONTINUE);
      sendCommand("AT+CIFSR", "OK", HALT);
    }
  }

/*******************************************************************
 * void sendMail(String type)
 * Skickar mail när event händer
*******************************************************************/
  void sendMail(String type)
  {
    String cmd = "AT+CIPSTART=0, \"TCP\",\""; cmd += SERVER_IP; cmd += "\""; cmd += SERVER_PORT;
    if(!sendCommand(cmd, "OK", CONTINUE))
    {
      return;
    }
    delay(2000);

    if(!sendCommand("AT+CIPSTATUS", "OK", CONTINUE))
    {
      return;
    }

    //Skicka för start för kommunikation mellan esp och AOL
    cmd = "Helo 192.168.43.144";
    if(!sendCommand("AT+CIPSEND=0,"+ String(cmd.length()), ">", CONTINUE))
    {
      sendCommand("AT+CIPCLOSE", "", CONTINUE);
      Serial.println("Connection timeout");
      return;
    }
    sendCommand(cmd, "OK", CONTINUE);

    //Skicka AUTH LOGON för att kunna logga in.
    cmd = "AUTH LOGIN";
    if(!sendCommand("AT+CIPSEND=0,"+ String(cmd.length()), ">", CONTINUE))
    {
      sendCommand("AT+CIPCLOSE", "", CONTINUE);
      Serial.println("Connection timeout");
      return;
    }
    sendCommand(cmd, "OK", CONTINUE);

    // Skickar min mail i 64bit kod
    cmd = "YWxhcm0uc2VuZGVyQGFvbC5jb20=";
    if(!sendCommand("AT+CIPSEND=0,"+ String(cmd.length()), ">", CONTINUE))
    {
      sendCommand("AT+CIPCLOSE", "", CONTINUE);
      Serial.println("Connection timeout");
      return;
    }
    sendCommand(cmd, "OK", CONTINUE);

    // Skickar mitt password till mailen i 64bit kod
    cmd = "Rm9taXQxOTk2IQ==";
    if(!sendCommand("AT+CIPSEND=0,"+ String(cmd.length()), ">", CONTINUE))
    {
      sendCommand("AT+CIPCLOSE", "", CONTINUE);
      Serial.println("Connection timeout");
      return;
    }
    sendCommand(cmd, "OK", CONTINUE);

    // Skickar vem som skickar mail
    cmd = "MAIL FROM:<alarm.sender@aol.com>";
    if(!sendCommand("AT+CIPSEND=0,"+ String(cmd.length()), ">", CONTINUE))
    {
      sendCommand("AT+CIPCLOSE", "", CONTINUE);
      Serial.println("Connection timeout");
      return;
    }
    sendCommand(cmd, "OK", CONTINUE);

    // Skickar till vem som ska ta emot
    cmd = "RCPT TO:<alarm.receivers@gmail.com>";
    if(!sendCommand("AT+CIPSEND=0,"+ String(cmd.length()), ">", CONTINUE))
    {
      sendCommand("AT+CIPCLOSE", "", CONTINUE);
      Serial.println("Connection timeout");
      return;
    }
    sendCommand(cmd, "OK", CONTINUE);

    // skickar DATA, oklart atm
    cmd = "DATA";
    if(!sendCommand("AT+CIPSEND=0,"+ String(cmd.length()), ">", CONTINUE))
    {
      sendCommand("AT+CIPCLOSE", "", CONTINUE);
      Serial.println("Connection timeout");
      return;
    }
    sendCommand(cmd, "OK", CONTINUE);

    // skickar ngt
    cmd = "From: alarm.sender@aol.com <alarm.sender@aol.com>";
    if(!sendCommand("AT+CIPSEND=0,"+ String(cmd.length()), ">", CONTINUE))
    {
      sendCommand("AT+CIPCLOSE", "", CONTINUE);
      Serial.println("Connection timeout");
      return;
    }
    sendCommand(cmd, "OK", CONTINUE);

    // skickar ngt
    cmd = "To: alarm.receivers@gmail.com <alarm.receivers@gmail.com>";
    if(!sendCommand("AT+CIPSEND=0,"+ String(cmd.length()), ">", CONTINUE))
    {
      sendCommand("AT+CIPCLOSE", "", CONTINUE);
      Serial.println("Connection timeout");
      return;
    }
    sendCommand(cmd, "OK", CONTINUE);

    //Skriver vilket subject på mejlet, blir tydligt för mottagaren vad eventet handlar om.
    cmd = "Subject: ";
    cmd += type;
    if(!sendCommand("AT+CIPSEND=0,"+ String(cmd.length()), ">", CONTINUE))
    {
      sendCommand("AT+CIPCLOSE", "", CONTINUE);
      Serial.println("Connection timeout");
      return;
    }
    sendCommand(cmd, "OK", CONTINUE);

    // POINT"." betyder att det är slutet på ett meddelande
    cmd = ".";
    if(!sendCommand("AT+CIPSEND=0,"+ String(cmd.length()), ">", CONTINUE))
    {
      sendCommand("AT+CIPCLOSE", "", CONTINUE);
      Serial.println("Connection timeout");
      return;
    }
    sendCommand(cmd, "OK", CONTINUE);

    // Skickar att man ska stänga kommunikationen
    cmd = "QUIT";
    if(!sendCommand("AT+CIPSEND=0,"+ String(cmd.length()), ">", CONTINUE))
    {
      sendCommand("AT+CIPCLOSE", "", CONTINUE);
      Serial.println("Connection timeout");
      return;
    }
    sendCommand(cmd, "OK", CONTINUE);

    readResponseData("");
    exception("ONLY ONCE");
  }
