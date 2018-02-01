// Include VirtualWire library
#include <VirtualWire.h>

int led_pin = 13;
int transmit_pin = 12;
int pir_pin = 2;
int val = 0; 
int pir_state = LOW;

/*******************************************************************
 * SETUP
 * Nödvändiga funktioner för att man ska kunna skicka till receiver
*******************************************************************/
void setup()
{
   Serial.begin(9600);
   vw_set_tx_pin(transmit_pin);
   vw_setup(4000); // Transmission rate
   pinMode(led_pin, OUTPUT);
   pinMode(pir_pin,INPUT);
}

/*******************************************************************
 * LOOP
 * Tar in data från rörelsedetektorn och skickar det till receivern
*******************************************************************/
void loop()
{
  // Message to send
  char msg[1] = {'0'};
  // Get sensor value
  magnetValue = digitalRead(/*magnet_pin*/);
  
  if(magnetValue == 1)
  {
    msg[0] = '2';
    digitalWrite(led_pin, HIGH);
    vw_send((uint8_t *)msg, 1);
    vw_wait_tx();
  }
}
