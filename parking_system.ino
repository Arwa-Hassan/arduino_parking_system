//for the oled
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>     
#include <Adafruit_SSD1306.h> 

#include <Servo.h>
#include <RH_RF95.h> //for LORA network

// Singleton instance of the radio driver
//RH_RF95 rf96;

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

// Declaration for SSD1306 display connected using I2C
//#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
//#define SCREEN_ADDRESS 0x3C
//Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Declaration for SSD1306 display connected using software SPI:
#define OLED_MOSI 9
#define OLED_CLK 10
#define OLED_DC 11
#define OLED_CS 12
#define OLED_RESET 13
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
Servo myservo;
int ir_s1 = 5;
int ir_s2 = 6;
int ir_s3 = 7;

int S1 = 0, S2 = 0, S3 = 0;

int ir_enter = 2;
int ir_back = 4;


int slot = 3;

bool flag1 = 0;
bool flag2 = 0;


void setup() {
  Serial.begin(9600);

  while (!Serial)
    ;  // Wait for serial port to be available
  if (!rf96.init())
    Serial.println("init failed");

  pinMode(ir_s1, INPUT);
  pinMode(ir_s2, INPUT);
  pinMode(ir_s3, INPUT);
  pinMode(ir_enter, INPUT);
  pinMode(ir_back, INPUT);


  myservo.attach(3);  //attaching the servo to pin 3
  myservo.write(90);  //starting at 90 degrees

  Read_Sensor();

  int total = S1 + S2 + S3;
  slot = slot - total;
  // initialize the OLED object
  //if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
  // Serial.println(F("SSD1306 allocation failed"));
  // for(;;); // Don't proceed, loop forever
  // }

  // Uncomment this if you are using SPI
  if (!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }

  // Clear the buffer.
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("car parking ");
  display.setCursor(0, 20);
  display.println("system ");
  display.display();
  delay(4000);
  display.clearDisplay();
}

void loop() {
  //taking readings from the sensors at each parking slot
  Read_Sensor();

  //displaying the no. of available slots
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print("Have Slot: ");
  display.println(slot);
  display.println("     ");
  display.display();

  //displaying the state of slot 1 (full or empty)
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 15);    //moving the cursor to the beginnig of a seperate new line
  if (S1 == 1) {
    display.println("S1:Full ");
    display.display();
  } else {
    display.println("S1:Empty");
    display.display();
  }

  //displaying the state of slot 2 (full or empty)
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 35);    //moving the cursor to the beginnig of a seperate new line
  if (S2 == 1) {
    display.println("S2:Full ");
    display.display();
  } else {
    display.println("S2:Empty");
    display.display();
  }

  //displaying the state of slot 3 (full or empty)
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 55);    //moving the cursor to the beginnig of a seperate new line
  if (S3 == 1) {
    display.println("S3:Full ");
    display.display();
  } else {
    display.println("S3:Empty");
    display.display();
  }

  //checking when a car passes infront of the enterance sensor
  if (digitalRead(ir_enter) == 0 && flag1 == 0) {
    if (slot > 0) {
      flag1 = 1;
      if (flag2 == 0) {     //checking that the car is not exiting (this will be false when the car passes by the exit sensor first while exiting i.e. when it passes by the enterance sensor while exiting)
        myservo.write(180); //opening the door by rotating the servo 90 degrees
        slot = slot - 1;    //on entering the no. of empty slots decrease
      }
    } else {  //this will be excuted when it tries to enter when no empty slots are there
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0, 0);
      display.println(" Sorry Parking Full ");
      display.display();
      delay(2000);
    }
  }

  //checking when a car passes infront of the exiting sensor
  if (digitalRead(ir_back) == 0 && flag2 == 0) {
    flag2 = 1;
    if (flag1 == 0) {     //checking that the car is not actually entering while passing through the exiting sensor
      myservo.write(180);            //opening the door by rotating the servo 90 degrees
      if (slot < 3) slot = slot + 1; //on exiting the no. of empty slots increase
    }
  }

  //reseting the flags after passing by the two sensors (enterance & exit)
  if (flag1 == 1 && flag2 == 1) {
    delay(1000);
    myservo.write(90);
    flag1 = 0, flag2 = 0;
  }

  delay(1000);
  display.clearDisplay();
  ///////////////////////////////////////////////////////
  //we can use the following code to enhance the functionality by sending data through LORA network
  // Serial.println("Sending to rf96_server");
  // // Send a message to rf95_server
  // uint8_t data[] = "Hello I'm from lora!";
  // rf96.send(data, sizeof(data));
  // rf96.waitPacketSent();
  // // Now wait for a reply
  // uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  // uint8_t len = sizeof(buf);
  // if (rf96.waitAvailableTimeout(3000)) {
  //   // Should be a reply message for us now
  //   if (rf96.recv(buf, &len)) {
  //     Serial.print("got reply: ");
  //     Serial.println((char*)buf);
  //   } else {
  //     Serial.println("recv failed");
  //   }
  // } else {
  //   Serial.println("No reply, is rf95_server running?");
  // }
  // delay(400);
}

//function taking readings from the sensors at each parking slot
void Read_Sensor() {
  S1 = 0, S2 = 0, S3 = 0;

  if (digitalRead(ir_s1) == 0) { S1 = 1; }
  if (digitalRead(ir_s2) == 0) { S2 = 1; }
  if (digitalRead(ir_s3) == 0) { S3 = 1; }
}
