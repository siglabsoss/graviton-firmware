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

#define INTERRUPT 2

/*
 * Messages from Copper Suicide
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
#define CS_OP_CODE_63             0x3F

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT); 
  pinMode(INTERRUPT, OUTPUT);
  digitalWrite(INTERRUPT, LOW);


  Serial1.begin(115200);
}

// the loop function runs over and over again forever
void loop() {
  uint8_t buf[2] = { 0 };
  digitalWrite(INTERRUPT, LOW);
  digitalWrite(LED_BUILTIN, LOW);
  buf[0] = CS_AFE_A | CS_RX;
  buf[1] = CS_AFE_B | CS_OP_CODE_3;
  Serial1.write(buf, 2);
//  Serial1.write(CS_AFE_B | CS_OP_CODE_3);
  delay(50);

  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(INTERRUPT, HIGH);

  delay(50);

  digitalWrite(INTERRUPT, LOW);
  digitalWrite(LED_BUILTIN, LOW);
  buf[0] = CS_OP_CODE_0;
  buf[1] = CS_OP_CODE_0;
  Serial1.write(buf, 2);
  delay(50);

  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(INTERRUPT, HIGH);

  delay(50);
//  
//  digitalWrite(INTERRUPT, HIGH);
//  delay(1000);
//
//  digitalWrite(INTERRUPT, LOW);
//  message[0] = CS_AFE_B | CS_OP_CODE_3;
//  Serial1.println(message);
//  delay(1000);
//
//    digitalWrite(INTERRUPT, HIGH);
//  delay(1000);
//
//  digitalWrite(INTERRUPT, LOW);
//  message[0] = CS_AFE_B | CS_RX;
//  Serial1.println(message);
//  delay(1000);

}




