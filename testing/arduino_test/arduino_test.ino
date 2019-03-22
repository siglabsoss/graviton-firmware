/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://www.arduino.cc/en/Main/Products

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/Blink
*/

#include "CurieTimerOne.h"



#define INTERRUPT 2
#define TRIGGER 3

/*
   Messages from Copper Suicide
*/

#define CS_AFE_MASK               0x80
#define CS_RX_FLAG_MASK           0x40
#define CS_OP_CODE_MASK           0x3F

#define CS_AFE_A                  0x00
#define CS_AFE_B                  0x80
#define CS_RX                     0x40

#define CS_OP_CODE_0              0x00
#define CS_OP_CODE_1              0x01
#define CS_OP_CODE_2              0x02
#define CS_OP_CODE_3              0x03
#define CS_OP_CODE_4              0x04
#define CS_OP_CODE_5              0x05
#define CS_OP_CODE_6              0x06
#define CS_OP_CODE_7              0x07
#define CS_OP_CODE_8              0x08
#define CS_OP_CODE_9              0x09
#define CS_OP_CODE_10             0x0A
#define CS_OP_CODE_11             0x0B
#define CS_OP_CODE_12             0x0C
#define CS_OP_CODE_13             0x0D
#define CS_OP_CODE_14             0x0E
#define CS_OP_CODE_15             0x0F
#define CS_OP_CODE_16             0x10
#define CS_OP_CODE_17             0x11
#define CS_OP_CODE_18             0x12
#define CS_OP_CODE_19             0x13
#define CS_OP_CODE_20             0x14
#define CS_OP_CODE_21             0x15
#define CS_OP_CODE_22             0x16
#define CS_OP_CODE_23             0x17
#define CS_OP_CODE_24             0x18
#define CS_OP_CODE_25             0x19
#define CS_OP_CODE_63             0x3F

uint8_t state = 0;

void timedBlinkIsr()   // callback function when interrupt is asserted
{
  uint8_t buf[3] = { 0 };

  switch (state)
  {
    case 0:
      digitalWrite(INTERRUPT, HIGH);
      digitalWrite(LED_BUILTIN, HIGH);
      buf[0] = CS_AFE_A | CS_OP_CODE_3;
      buf[1] = CS_AFE_B | CS_OP_CODE_3;
      Serial1.write(buf, 2);
      delayMicroseconds(200);
      digitalWrite(LED_BUILTIN, LOW);
      digitalWrite(INTERRUPT, LOW);
      digitalWrite(TRIGGER, HIGH);
      state = 2;
      break;
    case 2:
      digitalWrite(INTERRUPT, HIGH);
      digitalWrite(LED_BUILTIN, HIGH);
      buf[0] = CS_AFE_A | CS_RX;
      buf[1] = CS_AFE_B | CS_RX;
      Serial1.write(buf, 2);
      delayMicroseconds(200);
      digitalWrite(LED_BUILTIN, LOW);
      digitalWrite(INTERRUPT, LOW);
      digitalWrite(TRIGGER, LOW);
      state = 0;
      break;
    default:
      state = 0;
      break;
  }
}

// the setup function runs once when you press reset or power the board
void setup() {
  uint8_t buf[1];

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(INTERRUPT, OUTPUT);
  pinMode(TRIGGER, OUTPUT);
  digitalWrite(INTERRUPT, LOW);

  buf[0] = CS_OP_CODE_0;
  Serial1.write(buf, 1);


  Serial1.begin(115200);

  CurieTimerOne.start(1000, &timedBlinkIsr);

}

// the loop function runs over and over again forever
void loop() {
  uint8_t buf[1];
  buf[0] = CS_OP_CODE_0;
  Serial1.write(buf, 1);
  delay(10000);

}
