// Include VirtualWire library
#include <VirtualWire.h>

int led_pin = 13;
int transmit_pin = 12;
int pir_pin = 2;
int val = 0; 
int pir_state = LOW;

void setup()
{
   Serial.begin(9600);
   vw_set_tx_pin(transmit_pin);
   vw_setup(4000); // Transmission rate
   pinMode(led_pin, OUTPUT);
   pinMode(pir_pin,INPUT);
}
 
void loop()
{
  // Message to send
  char msg[1] = {'0'};
  // Get sensor value
  pirValue = digitalRead(pir_pin);
  magnetValue = digitalRead(/*magnet_pin*/);
  
  // Change message if motion is detected
  if (pirValue == 1)
  {
    msg[0] = '1';
    digitalWrite(led_pin, HIGH); // Flash a light to show transmitting
    vw_send((uint8_t *)msg, 1);
    vw_wait_tx(); // Wait until the whole message is gone
  }

  if(magnetValue == 1)
  {
    msg[0] = '2';
    digitalWrite(led_pin, HIGH);
    vw_send((uint8_t *)msg, 1);
    vw_wait_tx();
  }
}
