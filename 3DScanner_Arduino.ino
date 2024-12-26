
#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include <SD.h>
#include "memorysaver.h"
#define ARDUCHIP_FRAMES 		0x01 
//This demo can only work on OV5640_MINI_5MP_PLUS or OV5642_MINI_5MP_PLUS platform.
#if !(defined (OV2640_MINI_2MP_PLUS))
#error Please select the hardware platform and camera module in the ../libraries/ArduCAM/memorysaver.h file
#endif
#define   FRAMES_NUM    0x00
//#define GPIO_RESET_MASK 		0x01
// set pin 7 as the slave select for the digital pot:
const int CS = 7;
#define SD_CS 10          // SD card CS pin

const int stepPin = 2;
const int dirPin = 3;
int steps_per_picture = 20;   // Number of steps to move after each picture
int total_steps = 200;        // Total steps for full movement

#define BUTTON_PIN 5       // Button to trigger capturing
#define LED_PIN 4          // Blue LED to indicate capturing

bool is_header = false;
int total_time = 0;
int picture_count = 0;
const int max_pictures = 10;
int total_movement = 0;
#if defined (OV2640_MINI_2MP_PLUS)
ArduCAM myCAM( OV2640, CS );
#endif
uint8_t read_fifo_burst(ArduCAM myCAM);

void setup() {
  // put your setup code here, to run once:
  uint8_t vid, pid;
  uint8_t temp;
  
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  
  // Ensure the motor is idle at the start
  digitalWrite(stepPin, LOW);


#if defined(__SAM3X8E__)
  Wire1.begin();
#else
  Wire.begin();
#endif
  Serial.begin(115200);
  Serial.println(F("ArduCAM Start!"));
  // set the CS as an output:
  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH);
  // initialize SPI:
  SPI.begin();
  //Reset the CPLD
myCAM.write_reg(0x07, 0x80);
delay(100);
myCAM.write_reg(0x07, 0x00);
delay(100);
  while (1) {
    //Check if the ArduCAM SPI bus is OK
    myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
    temp = myCAM.read_reg(ARDUCHIP_TEST1);
    if (temp != 0x55)
    {
      Serial.println(F("SPI interface Error!"));
      delay(1000); continue;
    } else {
      Serial.println(F("SPI interface OK.")); break;
    }
  }
#if defined (OV2640_MINI_2MP_PLUS)
  while (1) {
    //Check if the camera module type is OV2640
    myCAM.wrSensorReg8_8(0xff, 0x01);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
    if ((vid != 0x26 ) && (( pid != 0x41 ) || ( pid != 0x42 ))) {
      Serial.println(F("ACK CMD Can't find OV2640 module!"));
      delay(1000); continue;
    }
    else {
      Serial.println(F("ACK CMD OV2640 detected.")); break;
    }
  }
#endif
  //Initialize SD Card
  while (!SD.begin(SD_CS))
  {
    Serial.println(F("SD Card Error!")); delay(1000);
  }
  Serial.println(F("SD Card detected."));
  //Change to JPEG capture mode and initialize the OV5640 module
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myCAM.clear_fifo_flag();
  myCAM.write_reg(ARDUCHIP_FRAMES, FRAMES_NUM);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (digitalRead(BUTTON_PIN) == HIGH) {  // Assuming active LOW
    Serial.println(F("Button pressed. Starting capture..."));

    // Turn on the LED to indicate capturing
    digitalWrite(LED_PIN, HIGH);

    // Take 'max_pictures' pictures
    for (picture_count = 0; picture_count < max_pictures; picture_count++) {
      take_picture(picture_count + 1);
      total_movement += steps_per_picture;
      Serial.print(F("total movement: "));
      Serial.print(total_movement);
      delay(1000);  // Optional: delay between pictures
    }
   /*  
     if (total_movement != total_steps) {
      int correction_steps = total_steps - total_movement;
      move_stepper(correction_steps);  // Move the stepper back to its original position
      Serial.println(F("Final position correction done."));
    }
    */
    // Turn off the LED after capturing is done
    digitalWrite(LED_PIN, LOW);
    Serial.println(F("Finished taking 10 pictures."));

    // Debounce delay to prevent multiple triggers
    delay(2000);
  }
}

void take_picture(int pic_num) {
  //reset_camera(myCAM);
  // Flush and clear the FIFO
  myCAM.flush_fifo();
  myCAM.clear_fifo_flag();
  
  //clearDisplayBuffer();  // This could be an external function to clear the display

  // Add a delay to ensure everything is reset
 // delay(2000);

#if defined(OV2640_MINI_2MP_PLUS)
  myCAM.OV2640_set_JPEG_size(OV2640_1600x1200); delay(1000);
#endif

  //delay(100);
  // Start capture
  myCAM.start_capture();
  Serial.println(F("Start capture."));
  total_time = millis();

  // Wait for capture to complete
  while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK));
  Serial.println(F("CAM Capture Done."));
  total_time = millis() - total_time;
  Serial.print(F("Capture total_time used (in milliseconds): "));
  Serial.println(total_time, DEC);

  // Save the picture
  total_time = millis();
  if (read_fifo_burst(myCAM)) {
    Serial.print(F("Picture "));
    Serial.print(pic_num);
    Serial.println(F(" saved successfully."));
  } else {
    Serial.print(F("Failed to save Picture "));
    Serial.println(pic_num);
  }
  total_time = millis() - total_time;
  Serial.print(F("Save capture total_time used (in milliseconds): "));
  Serial.println(total_time, DEC);

  // Clear the capture done flag
  myCAM.clear_fifo_flag();
  delay(5000);

  // Move the stepper motor after each picture
  move_stepper(steps_per_picture);
}

void move_stepper(int steps) {
  digitalWrite(dirPin, HIGH);
  for (int i = 0; i < steps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(1000);  // Adjust speed by changing the delay
    digitalWrite(stepPin, LOW);
    delayMicroseconds(1000);
  }
}

uint8_t read_fifo_burst(ArduCAM myCAM)
{
  uint8_t temp = 0, temp_last = 0;
  uint32_t length = 0;
  static int i = 0;
  static int k = 0;
  char str[16];
  File outFile;
  byte buf[256];
  length = myCAM.read_fifo_length();
  Serial.print(F("The fifo length is :"));
  Serial.println(length, DEC);
  if (length >= MAX_FIFO_SIZE) //8M
  {
    Serial.println("Over size.");
    return 0;
  }
  if (length == 0 ) //0 kb
  {
    Serial.println(F("Size is 0."));
    return 0;
  }
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();//Set fifo burst mode
  i = 0;
  while ( length-- )
  {
    temp_last = temp;
    temp =  SPI.transfer(0x00);
    //Read JPEG data from FIFO
    if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,
    {
      buf[i++] = temp;  //save the last  0XD9
      //Write the remain bytes in the buffer
      myCAM.CS_HIGH();
      outFile.write(buf, i);
      //Close the file
      outFile.close();
      Serial.println(F("OK"));
      is_header = false;
      myCAM.CS_LOW();
      myCAM.set_fifo_burst();
      i = 0;
    }
    if (is_header == true)
    {
      //Write image data to buffer if not full
      if (i < 256)
        buf[i++] = temp;
      else
      {
        //Write 256 bytes image data to file
        myCAM.CS_HIGH();
        outFile.write(buf, 256);
        i = 0;
        buf[i++] = temp;
        myCAM.CS_LOW();
        myCAM.set_fifo_burst();
      }
    }
    else if ((temp == 0xD8) & (temp_last == 0xFF))
    {
      is_header = true;
      myCAM.CS_HIGH();
      //Create a avi file
      k = k + 1;
      itoa(k, str, 10);
      strcat(str, ".jpg");
      //Open the new file
      outFile = SD.open(str, O_WRITE | O_CREAT | O_TRUNC);
      if (! outFile)
      {
        Serial.println(F("File open failed"));
        while (1);
      }
      myCAM.CS_LOW();
      myCAM.set_fifo_burst();
      buf[i++] = temp_last;
      buf[i++] = temp;
    }
  }
  myCAM.CS_HIGH();
  return 1;
}
