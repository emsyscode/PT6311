/****************************************************/
/* This is only one example of code structure       */
/* OFFCOURSE this code can be optimized, but        */
/* the idea is let it so simple to be easy catch    */
/* where we can do changes and look to the results  */
/****************************************************/
//set your clock speed
#define F_CPU 16000000UL
//these are the include files. They are outside the project folder
#include <avr/io.h>
//#include <iom1284p.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define VFD_data 7// If 0 write LCD, if 1 read of LCD
#define VFD_clk 8 // if 0 is a command, if 1 is a data0
#define VFD_stb 9 // Must be pulsed to LCD fetch data of bus

#define AdjustPins    PIND // before is C, but I'm use port C to VFC Controle signals

unsigned char DigitTo7SegEncoder(unsigned char digit, unsigned char common);

/*Global Variables Declarations*/
bool flagSet = false;
bool flagReached = false;
byte myByte= 0x01;   // this variable is only related with swapLed1.

unsigned char secs;
unsigned char hours = 0;
unsigned char minutes = 0;
unsigned char seconds = 0;
unsigned char milisec = 0;

unsigned char digit;
unsigned char number;

unsigned char numberA;
unsigned char numberB;
unsigned char numberC;
unsigned char numberD;
unsigned char numberE;
unsigned char numberF;

unsigned char wakeSecondsUnits=0;
unsigned char wakeSecondsDozens=0;
unsigned char wakeMinutesUnits=0;
unsigned char wakeMinutesDozens=0;
unsigned char wakeHoursUnits=0;
unsigned char wakeHoursDozens=0;

unsigned char wakeSeconds=0;
unsigned char wakeMinutes=0;
unsigned char wakeHours=0;
 
unsigned char grid;
unsigned char wordA = 0;
unsigned char wordB = 0;
unsigned int k=0;

unsigned int segments[] ={
  //This not respect the normal table for 7segm like "abcdefgh"  // 
      0b01110111, //0  // 
      0b00010010, //1  // 
      0b01101011, //2  // 
      0b01011011, //3  // 
      0b00011110, //4  // 
      0b01011101, //5  // 
      0b01111101, //6  // 
      0b00010011, //7  // 
      0b01111111, //8  // 
      0b00011111, //9  // 
      0b00000000, //10 // empty display
  };
/************************** Initialize of driver PT6311***************************************/
void pt6311_init(void)
{
  delayMicroseconds(200); //power_up delay
  // Note: Allways the first byte in the input data after the STB go to LOW is interpret as command!!!

  // Configure VFD display (grids)
  cmd_with_stb(0b00000000);//  (0b01000000)    cmd1 8 grids 20 segm in 6311
  delayMicroseconds(1);
  // turn vfd on, stop key scannig
   cmd_with_stb(0b10001000);//(BIN(01100110)); 
  delayMicroseconds(1);
  // Write to memory display, increment address, normal operation
  cmd_with_stb(0b01000000);//(BIN(01000000));
  delayMicroseconds(1);
  // Address 00H - 15H ( total of 11*2Bytes=176 Bits)
  cmd_with_stb(0b11000000);//(BIN(01100110)); 
  delayMicroseconds(1);
  // set DIMM/PWM to value
  cmd_with_stb((0b10001000) | 7);//0 min - 7 max  )(0b01010000)
  delayMicroseconds(1);
}
/********************** Send a command with the strobe/chip select only to 4 bits*******************************************/
void cmd_4bitsWithout_stb(unsigned char a)
{
  // send without stb
  unsigned char transmit = 3; //define our transmit pin
  unsigned char data = 170; //value to transmit, binary 10101010
  unsigned char mask = 1; //our bitmask
  
  data=a;
  //This don't send the strobe signal, to be used in burst data send
   for (mask = 00000001; mask>0; mask <<= 1) { //iterate through bit mask
     digitalWrite(VFD_clk, LOW);
     if (data & mask){ // if bitwise AND resolves to true
        digitalWrite(VFD_data, HIGH);
     }
     else{ //if bitwise and resolves to false
       digitalWrite(VFD_data, LOW);
     }
    delayMicroseconds(5);
    digitalWrite(VFD_clk, HIGH);
    delayMicroseconds(5);
   }
   //digitalWrite(VFD_clk, LOW);
}
/*********************** Send command without strobe/chip select******************************************/
void cmd_without_stb(unsigned char a)
{
  // send without stb
  unsigned char transmit = 7; //define our transmit pin
  unsigned char data = 170; //value to transmit, binary 10101010
  unsigned char mask = 1; //our bitmask
  
  data=a;
  //This don't send the strobe signal, to be used in burst data send
   for (mask = 00000001; mask>0; mask <<= 1) { //iterate through bit mask
     digitalWrite(VFD_clk, LOW);
     if (data & mask){ // if bitwise AND resolves to true
        digitalWrite(VFD_data, HIGH);
     }
     else{ //if bitwise and resolves to false
       digitalWrite(VFD_data, LOW);
     }
    delayMicroseconds(5);
    digitalWrite(VFD_clk, HIGH);
    delayMicroseconds(5);
   }
   //digitalWrite(VFD_clk, LOW);
}
/************************ Send a command with the strobe/chip select only to 4 bits *****************************/
void cmd_4bitsWith_stb(unsigned char a)
{
  // send with stb
  unsigned char transmit = 3; //define our transmit pin
  unsigned char data = 170; //value to transmit, binary 10101010
  unsigned char mask = 1; //our bitmask
  
  data=a;
  
  //This send the strobe signal
  //Note: The first byte input at in after the STB go LOW is interpreted as a command!!!
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(1);
   for (mask = 00000001; mask>0; mask <<= 1) { //iterate through bit mask
     digitalWrite(VFD_clk, LOW);
     delayMicroseconds(1);
     if (data & mask){ // if bitwise AND resolves to true
        digitalWrite(VFD_data, HIGH);
     }
     else{ //if bitwise and resolves to false
       digitalWrite(VFD_data, LOW);
     }
    digitalWrite(VFD_clk, HIGH);
    delayMicroseconds(1);
   }
   digitalWrite(VFD_stb, HIGH);
   delayMicroseconds(1);
}
/********************** Send a command with the strobe/chip select *******************************************/
void cmd_with_stb(unsigned char a)
{
  // send with stb
  unsigned char transmit = 7; //define our transmit pin
  unsigned char data = 170; //value to transmit, binary 10101010
  unsigned char mask = 1; //our bitmask
  
  data=a;
  
  //This send the strobe signal
  //Note: The first byte input at in after the STB go LOW is interpreted as a command!!!
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(1);
   for (mask = 00000001; mask>0; mask <<= 1) { //iterate through bit mask
     digitalWrite(VFD_clk, LOW);
     delayMicroseconds(1);
     if (data & mask){ // if bitwise AND resolves to true
        digitalWrite(VFD_data, HIGH);
     }
     else{ //if bitwise and resolves to false
       digitalWrite(VFD_data, LOW);
     }
    digitalWrite(VFD_clk, HIGH);
    delayMicroseconds(1);
   }
   digitalWrite(VFD_stb, HIGH);
   delayMicroseconds(1);
}
/*********************** Run a test to VFD, let it all bright******************************************/
void test_VFD(void)
{
  clear_VFD();
      
      digitalWrite(VFD_stb, LOW);
      delayMicroseconds(1);
      cmd_with_stb(0b00001000); // cmd 1 // 5 Grids & 16 Segments
      cmd_with_stb(0b01000000); // cmd 2 //Write VFD, Normal operation; Set pulse as 1/16, Auto increment
      cmd_with_stb(0b10001000 | 0x07); // cmd 2 //set on, dimmer to max
        digitalWrite(VFD_stb, LOW);
        delayMicroseconds(1);
        cmd_without_stb((0b11000000)); //cmd 3 wich define the start address (00H to 15H)
        // Only here must change the bit to test, first 2 bytes and 1/2 byte of third.
         for (int i = 0; i < 8 ; i++){ // test base to 16 segm and 5 grids
          // Zone of test, if write 1 on any position of 3 bytes below position, will bright segment corresponding it.
         cmd_without_stb(0b00000000); // Data to fill table 5*16 = 80 bits
         cmd_without_stb(0b00000000); // Data to fill table 5*16 = 80 bits
         cmd_4bitsWithout_stb(0b00000000); // Data to fill table 5*16 = 80 bits
         }
    
      //cmd_without_stb(0b00000001); // cmd1 Here I define the 5 grids and 16 Segments
      //cmd_with_stb((0b10001000) | 7); //cmd 4
      digitalWrite(VFD_stb, HIGH);
      delay(1);
      delay(3000);  
}
/*********************** Clear of VFD display ******************************************/
void clear_VFD(void)
{
  /*
  Here I clean all registers 
  Could be done only on the number of grid
  to be more fast. The 12 * 3 bytes = 36 registers
  */
      for (int n=0; n < 8; n++){  //
        cmd_with_stb(0b00001000); //       cmd 1 // 8 Grids & 20 Segments
        cmd_with_stb(0b01000000); //       cmd 2 //Normal operation; Set pulse as 1/16
        digitalWrite(VFD_stb, LOW);
        delayMicroseconds(1);
            cmd_without_stb((0b11000000) | n); // cmd 3 //wich define the start address (00H to 15H)
            cmd_without_stb(0b00000000); // Data to fill table of 5 grids * 16 segm = 80 bits on the table
            cmd_without_stb(0b00000000); // Data to fill table of 5 grids * 16 segm = 80 bits on the table
            cmd_4bitsWithout_stb(0b00000000); // only half byte of third byte.
            //
            //cmd_with_stb((0b10001000) | 7); //cmd 4
            digitalWrite(VFD_stb, HIGH);
            delayMicroseconds(100);
     }
}
/********************* Run the effect of wheels********************************************/
void AD16311_RunWheels(){
  int j, n;
  char x;
  short v = 0b0000000000000001;  // The short have a size of 16 bits(2 bytes)
 
        for (n=15; n < 16; n++){  //Note: only want write the position 9 & 10 of memory map (5 grids X 2 bytes)
          //clear_VFD();
            for(j = 0; j < 5; j++) {  // execute 8 times the for cycle
              //cmd1 Configure VFD display (grids) 
              cmd_with_stb(0b00000000);//  5 grids
              delayMicroseconds(1);  // 
              
              //cmd2 Write to memory display, increment address, normal operation 
              cmd_with_stb(0b01000000);//Teste mode setting to normal, Address increment Fixed, Write data to display memory...
              
              digitalWrite(VFD_stb, LOW);
              delayMicroseconds(1);
              //cmd3 Address 15H Start 3 position of memory allocated to the 6ª grid(total of 2bytes and half).
              cmd_without_stb((0b11000000) | n);//Increment active, then test all segments from positions 15 to 17, grid 6!
              delayMicroseconds(1);
               x=0b00000001;
               cmd_without_stb((x << j) & 0xFF); // Block the 8º bit wich brigth the CD symbol.
              delayMicroseconds(1);
              digitalWrite(VFD_stb, HIGH);
              //cmd4 set DIMM/PWM to value
              cmd_with_stb((0b10001000) | 7);//0 min - 7 max  )(0b01010000)//0 min - 7 max  )(0b01010000)
              delay(30);
            }
        }
}
/*********************** Assigne values to variable of clock*******************************************/
void send_update_clock(void)
{
  if (secs >=60){
    secs =0;
    minutes++;
  }
  if (minutes >=60){
    minutes =0;
    hours++;
  }
  if (hours >=24){
    hours =0;
  }
    //------------------------------------------------------------
    DigitTo7SegEncoder(secs%10);
    //Serial.println(secs, DEC);
    numberA=segments[number];
    DigitTo7SegEncoder(secs/10);
    //Serial.println(secs, DEC);
    numberB=segments[number];
    SegTo32Bits();
    //------------------------------------------------------------
    DigitTo7SegEncoder(minutes%10);
    numberC=segments[number];
    DigitTo7SegEncoder(minutes/10);
    numberD=segments[number];
    SegTo32Bits();
    //------------------------------------------------------------
    DigitTo7SegEncoder(hours%10);
    numberE=segments[number];
    DigitTo7SegEncoder(hours/10);
    numberF=segments[number];
    SegTo32Bits();
    //------------------------------------------------------------
    //
}
/********************** Assigne values to Alarm variables********************************************/
void send_update_Alarm(void)
{
  if (secs >=60){
    secs =0;
    minutes++;
    
  }
  if (minutes >=60){
    minutes =0;
    hours++;
  }
  if (hours >=24){
    hours =0;
  }
    //------------------------------------------------------------
    DigitTo7SegEncoder(wakeSeconds%10);
    //Serial.println(secs, DEC);
    wakeSecondsUnits=segments[number];
    DigitTo7SegEncoder(wakeSeconds/10);
    //Serial.println(secs, DEC);
    wakeSecondsDozens=segments[number];
    wakeUpSet();
    //------------------------------------------------------------
    DigitTo7SegEncoder(wakeMinutes%10);
    wakeMinutesUnits=segments[number];
    DigitTo7SegEncoder(wakeMinutes/10);
    wakeMinutesDozens=segments[number];
    wakeUpSet();
    //------------------------------------------------------------
    DigitTo7SegEncoder(wakeHours%10);
    wakeHoursUnits=segments[number];
    DigitTo7SegEncoder(wakeHours/10);
    wakeHoursDozens=segments[number];
    wakeUpSet();
    //------------------------------------------------------------
    //
}
/************************* Show values presents on variables of Clock *****************************************/
void SegTo32Bits(){
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(10);
      cmd_with_stb(0b00001000); // cmd 1 // 8 Grids 
      cmd_with_stb(0b01000000); // cmd 2 //Normal operation; Set pulse as 1/16
      
        digitalWrite(VFD_stb, LOW);
        delayMicroseconds(10);
        cmd_without_stb((0b11000000) | grid); //cmd 3 wich define the start address (00H to 15H)
          // Grid 0
          cmd_without_stb(0x00);// //----------------------------0
          cmd_without_stb(numberA);// seconds unit //------------1
          cmd_4bitsWithout_stb(0x00);//--------------------------2
          // Grid 1
          cmd_without_stb(0x00);// //----------------------------3
          cmd_without_stb(numberB);// seconds dozens  //---------4
          cmd_4bitsWithout_stb(0x00);//--------------------------5
          // Grid 2
          cmd_without_stb(numberD); // Minuts dozens //----------6
          cmd_without_stb(numberC); // Minuts unit //------------7
          cmd_4bitsWithout_stb(0x00);//--------------------------8
          // Grid 3
          cmd_without_stb(numberF); // Hours dozens //-----------A
          cmd_without_stb(numberE); // Hours unit //-------------9
          cmd_4bitsWithout_stb(0x00);//
          // Grid 4
          cmd_without_stb(0x00);//
          cmd_without_stb(0x00);//
          cmd_4bitsWithout_stb(0x00);// 
          // Grid 5
          cmd_without_stb(0x00);//
          cmd_without_stb(0x00);//
          cmd_4bitsWithout_stb(0x00);// 
          // Grid number 6 is hwere is the wheel
          //
      digitalWrite(VFD_stb, HIGH);
      delayMicroseconds(10);
      cmd_with_stb((0b10001000) | 7); //cmd 4
      delay(5);
       
}
/*************************** Show values presents on the Wake up Set variables **************************************/
void wakeUpSet(){
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(10);
      cmd_with_stb(0b00001000); // cmd 1 // 8 Grids 
      cmd_with_stb(0b01000000); // cmd 2 //Normal operation; Set pulse as 1/16
      
        digitalWrite(VFD_stb, LOW);
        delayMicroseconds(10);
        cmd_without_stb((0b11000000) | grid); //cmd 3 wich define the start address (00H to 15H)
          // Grid 0
          cmd_without_stb(0x00);// //---------------------------------0
          cmd_without_stb(wakeSecondsUnits);// seconds unit //--------1
          cmd_4bitsWithout_stb(0x00);//-------------------------------2
          // Grid 1
          cmd_without_stb(0x00);// //---------------------------------3
          cmd_without_stb(wakeSecondsDozens);// seconds dozens //-----4
          cmd_4bitsWithout_stb(0x00);//-------------------------------5
          // Grid 2
          cmd_without_stb(wakeMinutesDozens); // Minuts dozens //-----6
          cmd_without_stb(wakeMinutesUnits); // Minuts unit //--------7
          cmd_4bitsWithout_stb(0x00);//-------------------------------8
          // Grid 3
          cmd_without_stb(wakeHoursDozens); // Hours dozens //--------A
          cmd_without_stb(wakeHoursUnits); // Hours unit //-----------9
          cmd_4bitsWithout_stb(0x00);//
          // Grid 4
          cmd_without_stb(0x00);//
          cmd_without_stb(0x00);//
          cmd_4bitsWithout_stb(0x00);// 
          // Grid 5
          cmd_without_stb(0x00);//
          cmd_without_stb(0x00);//
          cmd_4bitsWithout_stb(0x00);// 
          // Grid number 6 is hwere is the wheel
          //
      digitalWrite(VFD_stb, HIGH);
      delayMicroseconds(10);
      cmd_with_stb((0b10001000) | 7); //cmd 4
      delay(5);
       
}
/********************** To adapt the 7 seg if use VFD with different 7 seg digits *****************************/
void DigitTo7SegEncoder( unsigned char digit)
{
  switch(digit)
  {
    case 0:   number=0;     break;  // if remove the LongX, need put here the segments[x]
    case 1:   number=1;     break;
    case 2:   number=2;     break;
    case 3:   number=3;     break;
    case 4:   number=4;     break;
    case 5:   number=5;     break;
    case 6:   number=6;     break;
    case 7:   number=7;     break;
    case 8:   number=8;     break;
    case 9:   number=9;     break;
  }
} 
/*********************** When use buttons supporte by pins of Arduino uno ***********************************/ 
void adjustHMS(){
 // This function will be not used if you use the buttons of the board with 6311
 // but in case of not use it, then you can read direct from the pins of arduino.
 // Important is necessary put a pull-up resistor to the VCC(+5VDC) to this pins (3, 4, 5)
 // if dont want adjust of the time comment or remove the call of function on the loop
  /* Reset Seconds to 00 Pin number 3 Switch to GND*/
    if((AdjustPins & 0x08) == 0 )
    {
      _delay_ms(200);
      secs=00;
    }
    
    /* Set Minutes when SegCntrl Pin 4 Switch is Pressed*/
    if((AdjustPins & 0x10) == 0 )
    {
      _delay_ms(200);
      if(minutes < 59)
      minutes++;
      else
      minutes = 0;
    }
    /* Set Hours when SegCntrl Pin 5 Switch is Pressed*/
    if((AdjustPins & 0x20) == 0 )
    {
      _delay_ms(200);
      if(hours < 23)
      hours++;
      else
      hours = 0;
    }
}
/************************* Send segments to help determine pinout of VFD *****************************/
void send7segm(){
      cmd_with_stb(0b00001000); // cmd 1 // 8 Grids & 20 Segments
      cmd_with_stb(0b01000000); // cmd 2 //Normal operation; Set pulse as 1/16
        //
        digitalWrite(VFD_stb, LOW);
        delayMicroseconds(1);
        cmd_without_stb((0b11000000)); //cmd 3 wich define the start address (00H to 15H)
        //
          cmd_without_stb(wordB); // seconds unit
          cmd_without_stb(segments[k]); // 
          cmd_without_stb(segments[k]); // minuts units
          cmd_without_stb(segments[k]); // minuts dozens
          cmd_without_stb(segments[k]); // hours units
          cmd_without_stb(segments[k]); // hours dozens
          cmd_without_stb(segments[k]); // hours third digit not used
      digitalWrite(VFD_stb, HIGH);
      cmd_with_stb((0b10001000) | 7); //cmd 4
      delay(1);
      delay(1000);  
}
/************************** Read buttons normal mode (fixed VFD) ********************************/
void readButtons(){
//Take special attention to the initialize digital pin LED_BUILTIN as an output.
//
int ledPin = 13;   // LED connected to digital pin 13
int inPin = 7;     // pushbutton connected to digital pin 7
int val = 0;       // variable to store the read value
int dataIn=0;

byte array[8] = {0,0,0,0,0,0,0,0};
byte together = 0;

unsigned char receive = 7; //define our transmit pin
unsigned char data = 0; //value to transmit, binary 10101010
unsigned char mask = 1; //our bitmask

array[0] = 1;

unsigned char btn1 = 0x41;

      digitalWrite(VFD_stb, LOW);
        delayMicroseconds(2);
      cmd_without_stb(0b01000010); // cmd 2 //Read Keys;Normal operation; Set pulse as 1/16
       // cmd_without_stb((0b11000000)); //cmd 3 wich define the start address (00H to 15H)
     // send without stb
  
  pinMode(7, INPUT);  // Important this point! Here I'm changing the direction of the pin to INPUT data.
  delayMicroseconds(2);
  //PORTD != B01010100; // this will set only the pins you want and leave the rest alone at
  //their current value (0 or 1), be careful setting an input pin though as you may turn 
  //on or off the pull up resistor  
  //This don't send the strobe signal, to be used in burst data send
  //This variable z on cycle for define quantity of bytes is read (6311 have a  matrix of 4*12=48 bits, 6bytes)
         for (int z = 0; z < 6; z++){ 
             //for (mask=00000001; mask > 0; mask <<= 1) { //iterate through bit mask
                   for (int h =8; h > 0; h--) {
                      digitalWrite(VFD_clk, HIGH);  // Remember wich the read data happen when the clk go from LOW to HIGH! Reverse from write data to out.
                      delayMicroseconds(2);
                     val = digitalRead(inPin);
                      //digitalWrite(ledPin, val);    // sets the LED to the button's value
                           if (val & mask){ // if bitwise AND resolves to true
                             //Serial.print(val);
                            //data =data | (1 << mask);
                            array[h] = 1;
                           }
                           else{ //if bitwise and resolves to false
                            //Serial.print(val);
                           // data = data | (1 << mask);
                           array[h] = 0;
                           }
                    digitalWrite(VFD_clk, LOW);
                    delayMicroseconds(2);
                    
                   } 
             
              Serial.print(z);
              Serial.print(" - " );
                        
                                  for (int bits = 7 ; bits > -1; bits--) {
                                      Serial.print(array[bits]);
                                   }
                       
                        if (z==0){
                          if(array[6] == 1){
                           hours++;
                          }
                        }
                          if (z==0){
                          if(array[7] == 1){
                           hours--;
                          }
                          }
                          if (z==0){
                          if(array[4] == 1){
                           minutes++;
                          }
                        }
                        if (z==0){
                          if(array[5] == 1){
                           minutes--;
                          }
                        }
                        if (z==1){
                          if(array[7] == 1){
                             if(flagSet == false)
                             flagSet=true;
                             else
                             flagSet=false;
                          }
                        }
                          if (z==1){
                            if(array[4] == 1){
                              hours = 0;
                              minutes = 0;
                             secs=0;  // Set count of secs to zero to be more easy to adjust with other clock.
                            }
                          }

                          if (z==1){
                            if(array[5] == 1){
                              swapLed1();
                              delay(1);
                              flagReached = false;
                              delay(1);
                            }
                          }
                         
                  Serial.println();
          }  // End of "for" of "z"
      Serial.println();

 digitalWrite(VFD_stb, HIGH);
 delayMicroseconds(2);
 cmd_with_stb((0b10001000) | 7); //cmd 4
 delayMicroseconds(2);
 pinMode(7, OUTPUT);  // Important this point!  // Important this point! Here I'm changing the direction of the pin to OUTPUT data.
 delay(1); 
 
}
/************************* Read buttons when wake up mode (Blinkiing VFD)**********************************/
void readButtonsWake(){
//Take special attention to the initialize digital pin LED_BUILTIN as an output.
//
int ledPin = 13;   // LED connected to digital pin 13
int inPin = 7;     // pushbutton connected to digital pin 7
int val = 0;       // variable to store the read value
int dataIn=0;

byte array[8] = {0,0,0,0,0,0,0,0};
byte together = 0;

unsigned char receive = 7; //define our transmit pin
unsigned char data = 0; //value to transmit, binary 10101010
unsigned char mask = 1; //our bitmask

array[0] = 1;

unsigned char btn1 = 0x41;

      digitalWrite(VFD_stb, LOW);
      delayMicroseconds(2);
      cmd_without_stb(0b01000010); // cmd 2 //Read Keys;Normal operation; Set pulse as 1/16
       // cmd_without_stb((0b11000000)); //cmd 3 wich define the start address (00H to 15H)
     // send without stb
  
  pinMode(7, INPUT);  // Important this point! Here I'm changing the direction of the pin to INPUT data.
  delayMicroseconds(2);
  //PORTD != B01010100; // this will set only the pins you want and leave the rest alone at
  //their current value (0 or 1), be careful setting an input pin though as you may turn 
  //on or off the pull up resistor  
  //This don't send the strobe signal, to be used in burst data send
  //This variable z on cycle for define quantity of bytes is read (6311 have a  matrix of 4*12=48 bits, 6bytes)
         for (int z = 0; z < 6; z++){
             //for (mask=00000001; mask > 0; mask <<= 1) { //iterate through bit mask
                   for (int h =8; h > 0; h--) {
                      digitalWrite(VFD_clk, HIGH);  // Remember wich the read data happen when the clk go from LOW to HIGH! Reverse from write data to out.
                      delayMicroseconds(2);
                     val = digitalRead(inPin);
                      //digitalWrite(ledPin, val);    // sets the LED to the button's value
                           if (val & mask){ // if bitwise AND resolves to true
                             //Serial.print(val);
                            //data =data | (1 << mask);
                            array[h] = 1;
                           }
                           else{ //if bitwise and resolves to false
                            //Serial.print(val);
                           // data = data | (1 << mask);
                           array[h] = 0;
                           }
                    digitalWrite(VFD_clk, LOW);
                    delayMicroseconds(2);
                    
                   } 
             
              Serial.print(z);
              Serial.print(" - " );
                        
                                  for (int bits = 7 ; bits > -1; bits--) {
                                     Serial.print(array[bits]);
                                  }
                       
                        if (z==0){
                          if(array[6] == 1){
                           wakeHours++;
                          }
                        }
                          if (z==0){
                          if(array[7] == 1){
                           wakeHours--;
                          }
                          }
                          if (z==0){
                          if(array[4] == 1){
                           wakeMinutes++;
                          }
                        }
                        if (z==0){
                          if(array[5] == 1){
                           wakeMinutes--;
                          }
                        }
                        if (z==0){
                          if(array[3] == 1){
                            flagReached = false;  // to start the compare the hours and wakeUp.
                             if(flagSet == false)
                             flagSet=true;
                             else
                             flagSet=false;
                          }
                        }
                          if (z==1){
                            if(array[4] == 1){
                              wakeHours = 0;
                              wakeMinutes = 0;
                             wakeSeconds=0;  // Set count of secs to zero to be more easy to adjust with other clocl.
                            }
                          }
                          if (z==1){
                          if(array[5] == 1){
                           myByte=0x01;
                          }
                        }
                         
                  Serial.println();
          }  // End of "for" of "z"
      Serial.println();

 digitalWrite(VFD_stb, HIGH);
 delayMicroseconds(2);
 cmd_with_stb((0b10001000) | 7); //cmd 4
 delayMicroseconds(2);
 pinMode(7, OUTPUT);  // Important this point!  // Important this point! Here I'm changing the direction of the pin to OUTPUT data.
 delay(1); 
}
/********************** Compare between WakeUp and Clock ****************************************/
void comparTime(){
    if (flagReached == false){
           if (( hours == wakeHours) and (minutes == wakeMinutes)){
              flagReached=true;
              digitalWrite(VFD_stb, LOW);
              delayMicroseconds(20);
              cmd_without_stb(0b01000001);
              delayMicroseconds(20);
              myByte ^=(0b00000001);  // Here is only to invert bit of led 1.
              cmd_without_stb(myByte);
              delayMicroseconds(20);
              digitalWrite(VFD_stb, HIGH);
              delayMicroseconds(20);
            }
            else{
            swapLed1();
           }
     }
     
}
/************************* Swap the led Function ***************************************/
void swapLed1(){
    digitalWrite(VFD_stb, LOW);
    delayMicroseconds(20);
    cmd_without_stb(0b01000001);
    delayMicroseconds(20);
    myByte ^=(0b00000001);  //Here is only to invert bit of led 1, repeat this function if you want use other of 3 remaining leds.
    cmd_without_stb(myByte);
    delayMicroseconds(20);
    digitalWrite(VFD_stb, HIGH);
    delayMicroseconds(20);
}
/************************ Setup Zone******************************************/
void setup() {
// put your setup code here, to run once:
// initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  seconds = 0x00;
  minutes =0x00;
  hours = 0x00;

  /*CS12  CS11 CS10 DESCRIPTION
  0        0     0  Timer/Counter1 Disabled 
  0        0     1  No Prescaling
  0        1     0  Clock / 8
  0        1     1  Clock / 64
  1        0     0  Clock / 256
  1        0     1  Clock / 1024
  1        1     0  External clock source on T1 pin, Clock on Falling edge
  1        1     1  External clock source on T1 pin, Clock on rising edge
 */
  // initialize timer1 
  cli();           // disable all interrupts
  // initialize timer1 
  //noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;// This initialisations is very important, to have sure the trigger take place!!!
  TCNT1  = 0;
  // Use 62499 to generate a cycle of 1 sex 2 X 0.5 Secs (16MHz / (2*256*(1+62449) = 0.5
  //ATT: Depending of the arduino you use, maybe is necessary redo this calcul. Check your Xtall
  OCR1A = 62499;            // compare match register 16MHz/256/2Hz
  TCCR1B |= (1 << WGM12);   // CTC mode
  TCCR1B |= ((1 << CS12) | (0 << CS11) | (0 << CS10));    // 256 prescaler 
  TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
  
// Note: this counts is done to a Arduino 1 with Atmega 328... Is possible you need adjust
// a little the value 62499 upper or lower if the clock have a delay or advnce on hours.
   
//  a=0x33;
//  b=0x01;

CLKPR=(0x80);
//Set PORT
DDRD = 0xFF;  // IMPORTANT: from pin 0 to 7 is port D, from pin 8 to 13 is port B
PORTD=0x00;
DDRB =0xFF;
PORTB =0x00;

pt6311_init();

test_VFD();

clear_VFD();

//only here I active the enable of interrupts to allow run the test of VFD
//interrupts();             // enable all interrupts
sei();
}
/********************** Loop Zone *********************************/
void loop() {
  // You can comment untill while cycle to avoid the test running.

   // Can use this cycle to teste all segments of VFD
   /*
       for(int h=0; h < 20; h++){
       k=h;
       send7segm();
       }
   */
  clear_VFD();
  while(1){
          if (flagSet==false){  // This flag define wich is show on  VFD, if wakeUpTime or clock values.
            comparTime();
            delay(10);
            send_update_clock(); // show the values of clock on VFD
            delay(10);
            readButtons();
            delay(10);
            AD16311_RunWheels();
            delay(10);
            Serial.print(" If : ");
            Serial.println(flagSet);
          }
          else{
          cmd_with_stb(0b10001000); // cmd 2 //display on
          delayMicroseconds(10);
          send_update_Alarm();  // show the values of wake up on VFD
          delay(250);
          cmd_with_stb(0b10000000); // cmd 2 //display off
          delay(250);
          readButtonsWake();
          Serial.print(" else : ");
          Serial.println(flagSet);
          }   
   }  
}
/******************** Interrupt Zone *****************************/
ISR(TIMER1_COMPA_vect)   { 
//This is the interrupt request
//https://sites.google.com/site/qeewiki/books/avr-guide/timers-on-the-atmega328
//
      secs++;
//
} 
