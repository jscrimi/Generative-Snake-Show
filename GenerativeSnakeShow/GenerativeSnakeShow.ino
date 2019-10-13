#include <RGBmatrixPanel.h>

//Generative Snakes made by Joe Crimi
//For CS252 Comp C&P Content Generation
//Uses connection to Adafruit 32x32 multicolor LED board
//
//Due to physical board space, I was not able to actually install a sensor that detected motion
// I've instead included functionality that will allow the user to type a number to simulate
// Holding the board at different angles

//Board is 32x32. A white border around the edge limits it to 30x30
//Board is split into 4 quadrants, making one quadrant 15x15
//Board is mirrored in both directions. (1,1) = (30,1) = (30,30) = (1,30)
//Quadrants are split into 25 macro-pixels (0,0) = ([1,3],[1,3])
// Each macro-pixel has 9 LEDs that can be changed by different snakes

#define CLK  8   // USE THIS ON ARDUINO UNO, ADAFRUIT METRO M0, etc.
#define OE   9
#define LAT 10
#define A   A0
#define B   A1
#define C   A2
#define D   A3

RGBmatrixPanel matrix(A, B, C, D, CLK, LAT, OE, false);

typedef struct{//Snake Structure
  uint16_t color;
  uint8_t x;
  uint8_t y;
  uint8_t moveTowards;
  uint8_t turnsLeft;
  bool pattern[3][3];
} snake;

snake snakes[4];
bool trails = true; //true or false

void setup() {
  matrix.begin();
  Serial.begin(9600);
  randomSeed(analogRead(4));
  // put your setup code here, to run once:

  matrix.drawRect(0,0,32,32,matrix.Color333(7,7,7));
  
  for(uint8_t i = 0; i < 4; i++){
    snakes[i].color = matrix.Color444(random(16),random(16),random(16));
    uint8_t xOrY = random(2);
    if(xOrY == 0){//Choose X, along top or bottom row
      snakes[i].x = random(5);
      xOrY = random(2);
      if(xOrY == 0){//Along Top Row
        snakes[i].y = 0;
        snakes[i].moveTowards = 3;//Move Down
      } else {//Else Along Bottom Row
        snakes[i].y = 4;
        snakes[i].moveTowards = 1;//Move Up
      }
    } else {//Choose Y, in left or right column
      snakes[i].y = random(5);
      xOrY = random(2);
      if(xOrY == 0){//in left column
        snakes[i].x = 0;
        snakes[i].moveTowards = 0;//Move Right
      } else {//else in right column
        snakes[i].x = 4;
        snakes[i].moveTowards = 2;//Move Left
      }
    }
    transformSnakes(i);
  } 
}

void drawSnakes(uint8_t i, uint8_t copyMode){
  for(uint8_t a = 0; a < 3; a++){
      for(uint8_t b = 0; b < 3; b++){
        if(snakes[i].pattern[b][a]){
          if(copyMode == 0){//Normal Mirroring
            //Top Left Quad
            matrix.drawPixel((snakes[i].x * 3)+a+1,(snakes[i].y*3)+1+b,snakes[i].color);
            //Top Right Quad (Flip X)
            matrix.drawPixel(30-(snakes[i].x*3)-a,(snakes[i].y*3)+1+b,snakes[i].color);
            //Bottom Left Quad (Flip Y)
            matrix.drawPixel((snakes[i].x * 3)+a+1, 30-(snakes[i].y*3)-b,snakes[i].color);
            //Bottom Right Quad (Flip Both)
            matrix.drawPixel(30-(snakes[i].x*3)-a,30-(snakes[i].y*3)-b,snakes[i].color);
            
          } else if(copyMode == 1){//Rotational Symmetry
            //Top Left Quad
            matrix.drawPixel((snakes[i].x * 3)+a+1,(snakes[i].y*3)+1+b,snakes[i].color);
            //Top Right Quad (Flip X)
            matrix.drawPixel((snakes[i].y*3)+1+b,30-(snakes[i].x*3)-a,snakes[i].color);
            //Bottom Left Quad (Flip Y)
            matrix.drawPixel(30-(snakes[i].y*3)-b,(snakes[i].x * 3)+a+1, snakes[i].color);
            //Bottom Right Quad (Flip Both)
            matrix.drawPixel(30-(snakes[i].x*3)-a,30-(snakes[i].y*3)-b,snakes[i].color);
          } else if(copyMode == 2){//No copying, single snakes
            matrix.drawPixel((snakes[i].x * 3)+a+1,(snakes[i].y*3)+1+b,snakes[i].color);
          } else if(copyMode == 3){//Translation
            matrix.drawPixel((snakes[i].x * 3)+a+1,(snakes[i].y*3)+1+b,snakes[i].color);
            matrix.drawPixel(((snakes[i].x * 3)+a+1 + 16) % 32,(snakes[i].y*3)+1+b,snakes[i].color);
            matrix.drawPixel((snakes[i].x * 3)+a+1,((snakes[i].y*3)+1+b + 16) % 32,snakes[i].color);
            matrix.drawPixel(((snakes[i].x * 3)+a+1 + 16) % 32,((snakes[i].y*3)+1+b + 16) % 32,snakes[i].color);
            //matrix.drawRect(0,0,32,32,matrix.Color333(7,7,7));
          }
        }
      }
    }
}

void moveSnakes(uint8_t i){
  switch(snakes[i].moveTowards){
      case 0://Move Right
        snakes[i].x = (snakes[i].x + 1) % 10;
        break;
      case 1://Move Up
        snakes[i].y = (snakes[i].y + 9) % 10;
        break;
      case 2://Move Left
        snakes[i].x = (snakes[i].x + 9) % 10;
        break;
      case 3://Move Down
        snakes[i].y = (snakes[i].y + 1) % 10;
        break;
    }
    if(snakes[i].turnsLeft > 0){
      uint8_t cOCOS = random(3);
      if(cOCOS == 0){//Turn CounterClockwise (+1 moveTowards)
        snakes[i].moveTowards = (snakes[i].moveTowards + 1) % 4;
        snakes[i].turnsLeft--;
      } else if(cOCOS == 1){//Don't Turn
        snakes[i].moveTowards = snakes[i].moveTowards;
      } else { //Turn Clockwise (+3 moveTowards)
        snakes[i].moveTowards = (snakes[i].moveTowards + 3) % 4;
        snakes[i].turnsLeft--;
      }
    }
}

void transformSnakes(uint8_t i){
  snakes[i].color = matrix.Color444(random(16),random(16),random(16));
    snakes[i].turnsLeft = random(1,255);
    if (i == 3 && snakes[i].turnsLeft < 100){
      snakes[i].color = 0;
    }
      for(uint8_t j = 0; j < 3; j++){
        for(uint8_t k = 0; k < 3; k++){
          uint8_t pixelOn = random(2);
          if(pixelOn == 1){
            snakes[i].pattern[j][k] = true;
            //Serial.print(F("O"));
          } else {
            snakes[i].pattern[j][k] = false;
            //Serial.print(F("_"));
          }
        }
        //Serial.println();
      }
      //Serial.println();
}


void loop() {
  static uint8_t frameDelay = 1;
  static byte numSnakes = 4;
  static uint8_t copyMode = 0;
  // put your main code here, to run repeatedly:
  delay(frameDelay*50);
  if(!(trails)){
    matrix.fillRect(1,1,30,30,matrix.Color333(0,0,0));
  }
  for(uint8_t i = 0; i < numSnakes; i++){
    drawSnakes(i,copyMode);
    //Move Snakes for next Frame
    moveSnakes(i);
    //Check to see if snake lost all available moves
    if(snakes[i].turnsLeft == 0){
      transformSnakes(i);
      //If they did, transform into a new snake
    }
  }

  //take in user input. 
  //Pressing T toggles Trails, 
  //Entering a Number sets the framerate (higher number is less FPS)
  if(Serial.available() > 0){
    uint8_t incomingByte = Serial.read();
    if(incomingByte == 116){ //t input, toggles Trails
      //Serial.println(F("TrailsToggled"));
      if(trails){
        trails = false;
      } else {
        trails = true;
      }
    }
    if(incomingByte == 114){ //r, switches through copy modes
      copyMode = (copyMode + 1) % 4;
    }
    if(incomingByte == 49){ //1
      frameDelay = 1;
    }
    if(incomingByte == 50){ //2
      frameDelay = 2;
    }
    if(incomingByte == 51){ //3
      frameDelay = 3;
    }
    if(incomingByte == 52){ //4
      frameDelay = 4;
    }
    if(incomingByte == 53){ //5
      frameDelay = 5;
    }
    if(incomingByte == 54){ //6
      frameDelay = 7;
    }
    if(incomingByte == 55){ //7
      frameDelay = 10;
    }
    if(incomingByte == 56){ //8
      frameDelay = 15;
    }
    if(incomingByte == 57){ //9
      frameDelay = 20;
    }
    if(incomingByte == 48){ //0
      frameDelay = 255;
    }
    if(incomingByte == 97){ //a
      numSnakes = 1;
    }
    if(incomingByte == 115){ //s
      numSnakes = 2;
    }
    if(incomingByte == 100){ //d
      numSnakes = 3;
    }
    if(incomingByte == 102){ //f
      numSnakes = 4;
    }
    if(incomingByte == 110){ //n
      numSnakes = 0;
    }
    if(incomingByte == 99){ //c
      for(uint8_t i = 0; i < numSnakes; i++){
        transformSnakes(i);
      }
    }
  }
  
}//loop()
