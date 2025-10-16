#include "library.h"
#include <Arduboy2.h>
//calculation for finding position on line given x, -4 is for wheel radius
#define WHEELPOS(x) (lineY((x - lineTimer), linePos[0][0][0], linePos[0][1][0], linePos[0][0][1], linePos[0][1][1]) - 4)
#define WHEEL2POS(x) (lineY((17 - lineTimer), linePos[1][0][0], linePos[1][1][0], linePos[1][0][1], linePos[1][1][1]) - 4)
#define BETWEEN(y, y1) ((y + y1) / 2)
#define MAXSTAM 660
#define MAIN_GAME 0
#define TUTORIAL 1
#define SPLASH_SCREEN 2
#define MENU 3

BeepPin1 beep;
BeepPin2 beep2;
Arduboy2 arduboy;

int tutStage = 0;
float screenTimer = -20;
int gamestate = SPLASH_SCREEN;
int gear = 1;
int stam = MAXSTAM;
bool pedalRight = true;
bool done = false;
int pedalPos = -40;
float wheelie = 0;
float pedalDif;
float pedalSpeed;
float px, py; //hold pedal position result
//tell the ground where to draw
float lineTimer = 0;
//position of the ground
int linePos[][2][2] = {
   { {0,128}, {24,24} }, { {128,256}, {24,0} }
};//[line][x/y][point1/2]

//set gear and PedalDif
void setGear() {
  //set the pedalDif, sets once when changing elevation
  if (secondLine(lineTimer)){
    pedalDif = ((WHEELPOS(5) - WHEEL2POS(17)) * gear);
  }
  //add or subtract to the gear int
  if(arduboy.justPressed(DOWN_BUTTON)&&(gear>1)) {
    gear--;
    beep2.tone(beep2.freq(1000), 1);
  }
  if(arduboy.justPressed(RIGHT_BUTTON)&&(gear<6)) {
    gear++;
    beep2.tone(beep2.freq(1000), 1);
  }
}

//pedalling logic
void pedal() {
  //if left pedal OR needs to switch AND not too far down
  if (arduboy.pressed(A_BUTTON) && (!pedalRight || pedalPos > 20) && (pedalPos > -40) && (!done)){
     //pedal left
     //set sound in range of 400, or turn off sound
     if (600 * bell(pedalPos) > 15.26){
       beep.tone(beep.freq(600 * bell(pedalPos)));
     } else {
       beep.noTone();
     }
     //tell program peddling left
     pedalRight = false;
     pedalPos --; //move left pedal down
     //cant just jump to highest gear
     pedalSpeed += (gear * simularity(gear, pedalSpeed));
     //if uphill, change pedalDif, lower stam
     if (pedalDif > 0) { 
       pedalSpeed = (pedalSpeed / (pedalDif / 5));
       stam -= ((gear * pedalDif) / 10);
     }
     //if downhill, subtract pedalDif to speed, add stam
     if (pedalDif < 0) {
       pedalSpeed -= (pedalDif / 5);
       stam ++;
     }
    }
  else if (arduboy.pressed(B_BUTTON) && (pedalRight || pedalPos < -20) && (pedalPos < 40) && (!done)) {
     //pedal right
     //set rachet sound off
     //set sound in range of 400, or turn off sound
     if (600 * bell(pedalPos) > 15.26){
       beep.tone(beep.freq(600 * bell(pedalPos)));
     } else {
       beep.noTone();
     }
     //tell program peddling right
     pedalRight = true;
     pedalPos ++; //move left pedal down
     //cant just jump to highest gear
     pedalSpeed += (gear * simularity(gear, pedalSpeed));
     //if uphill, change pedalDif, lower stam
     if (pedalDif > 0) { 
       pedalSpeed = (pedalSpeed / (pedalDif / 5));
       stam -= ((gear * pedalDif) / 5);
     }
     //if downhill, subtract pedalDif to speed, add stam
     if (pedalDif < 0) {
       pedalSpeed -= (pedalDif / 10);
       stam ++;
     }
  } else {
    //set idle pedal speed
    pedalSpeed += ((pedalDif * -1) * .01 );
    pedalSpeed -= .02;
    //play racheting sound if moving, turn it off if not
    if (pedalSpeed > 0) {
      beep.tone(beep.freq(gear + 15));
    } else {
      beep.noTone();
    }
    //increase stam if not pedalling
    stam ++;
    //increase stam faster if done
    if (done) {
      stam ++;
      //play beeping sound
      if ((stam / 6) % 2 == 0) {
        beep2.tone(beep2.freq(800));
      } else {
        beep2.noTone();
      }
    }
    //set done to false if healed
    if ((stam == MAXSTAM) && done) {
      done = false;
      beep2.noTone();
    }
  }
}

//brake, wheelie, and stam logic
void brakeEct() {
  //brakes
  if (arduboy.pressed(LEFT_BUTTON)) {
      pedalSpeed = pedalSpeed * .95;
  }
  //wheelie, for fun
  if (arduboy.pressed(UP_BUTTON)) {
    if(wheelie < 5) { wheelie += .5; }
  } else
  if(wheelie > 0) { wheelie -= .5; }

  //stop stam from being negative, set done to true
  if (stam < 0) { 
    stam = 0;
    done = true;
  }

  //stop stam from going over max
  if (stam > MAXSTAM) {
    stam = MAXSTAM;
  }
}

//speed and ground logic
void movement() {
  //set max speed = to gear
  if (pedalSpeed > gear) { pedalSpeed = gear; }
  //stop ground from going backwards
  if (pedalSpeed < 0) { pedalSpeed = 0; }
  //ground moves based on speed
  lineTimer -= pedalSpeed;
}

//Stamina, gear, pedals
void drawMisc() {
  //fil the stamina bar
  arduboy.fillRect((128)-(stam/6), 0, (stam/6), 6);//screen width-stam //x,y,w,h
  //fill the fraction of the stamina bar
  if (stam > 6) { arduboy.drawFastVLine((127)-(stam/6), 0, stam % 6); }//screen width-stam, remainder of stam / 6
  //print gear number, cursor is set
  arduboy.print(gear);
  //draw drive train
  arduboy.drawCircle(11, 3, 3);//x, y, radius
  //call the function, based on pedalPos, flips the pedalling direction
  // input(* -1 to reverse travel direction), radius, centerX, centerY, outX, outY, bool which pedal
  if (pedalRight) {
    getCirclePoint(pedalPos, 3, 11, 3, &px, &py, pedalRight);
  } else {
    getCirclePoint(pedalPos * -1, 3, 11, 3, &px, &py, pedalRight);
  }
  //draw pedal
  arduboy.drawFastHLine((int)roundf(px - 1.25), (int)roundf(py), 5); //x, y, length, color
}

//Terrain
void drawTerrain() {
  //l is for line  
  for (int l = 0; l < (sizeof(linePos) / sizeof(linePos[0])); l++){
    arduboy.drawLine(((linePos[l][0][0]) + lineTimer), (linePos[l][1][0]),
                     ((linePos[l][0][1]) + lineTimer), (linePos[l][1][1]));
  //pt1 x, y, pt2 x, y
  }
}

//bike, pos to control x
void drawBike(int(pos)) {
  //the back wheel
  arduboy.drawCircle(pos +5, WHEELPOS(5), 4); //x,y,radius
  //if first wheel is on second line
  if (secondLine(lineTimer)){
    arduboy.drawCircle(pos + 17, (WHEEL2POS(17) - wheelie), 4);
    //x, defined calculation, radius
    //body of bike
    //triangle in the back
    arduboy.drawTriangle(pos + 5, WHEELPOS(5),
                         pos + 9, BETWEEN(WHEELPOS(5), (WHEEL2POS(17) - wheelie)),
                         pos + 10, BETWEEN(WHEELPOS(5), (WHEEL2POS(17) - wheelie)) - 6); //x y, x1 y1, x2 y2
    //line on bottom
    arduboy.drawLine(pos + 9, BETWEEN(WHEELPOS(5), (WHEEL2POS(17) - wheelie)), pos + 16, (WHEEL2POS(17) - wheelie) - 7); // x, y, x1, y1
    //line on top
    arduboy.drawLine(pos + 10, BETWEEN(WHEELPOS(5), (WHEEL2POS(17) - wheelie)) - 6, pos + 16, (WHEEL2POS(17) - wheelie) - 7);
    //line to right wheel
    arduboy.drawLine(pos + 16, (WHEEL2POS(17) - wheelie) - 7, pos + 17, (WHEEL2POS(17) - wheelie));
  }
  else {   
    //the wheel
    arduboy.drawCircle(pos + 17, (WHEELPOS(17) - wheelie), 4); //x,y,radius
    //body of bike
    //triangle in the back
    arduboy.drawTriangle(pos + 5, WHEELPOS(5),
                         pos + 9, BETWEEN(WHEELPOS(5), (WHEELPOS(17) - wheelie)),
                         pos + 10, BETWEEN(WHEELPOS(5), (WHEELPOS(17) - wheelie)) - 6); //x y, x1 y1, x2 y2
    //line on bottom
    arduboy.drawLine(pos + 9, BETWEEN(WHEELPOS(5), (WHEELPOS(17) - wheelie)), pos + 16, (WHEELPOS(17) - wheelie) - 7); // x, y, x1, y1
    //line on top
    arduboy.drawLine(pos + 10, BETWEEN(WHEELPOS(5), (WHEELPOS(17) - wheelie)) - 6, pos + 16, (WHEELPOS(17) - wheelie) - 7);
    //line to right wheel
    arduboy.drawLine(pos + 16, (WHEELPOS(17) - wheelie) - 7, pos + 17, (WHEELPOS(17) - wheelie));
  }
}

//terrainLogic
void terrainLogic() {
  //lineTimer --;//use for constant speed
  if (lineTimer < -128){
    lineTimer = 0;
    //set all line y positions to shift
    linePos[0][1][0] = linePos[0][1][1];
    //intersecting point of two lines
    linePos[0][1][1] = linePos[1][1][1];
    linePos[1][1][0] = linePos[1][1][1];
    //the the random height, lower for menu
    if (gamestate == MENU || gamestate == TUTORIAL) {
      linePos[1][1][1] = random(42, 64);
    } else {
      linePos[1][1][1] = random(22, 64);
    }
  }
}

//Tutorial loop
void tutorialLoop(int(stage)) {
  switch(stage) {
    case(0):
      drawTerrain();
      drawBike(0);
      arduboy.setCursor(0,0);
      arduboy.print("Hello There!");
      screenTimer ++;
      if (screenTimer > 179) {tutStage = 1; screenTimer = 0;}
      break;
    case(1):
      drawMisc();
      drawTerrain();
      drawBike(0);
      pedal();
      brakeEct();
      movement();
      terrainLogic();
      arduboy.setCursor(0,8);
      arduboy.print("Hold B to pedal");
      if (pedalPos > 29) {
        tutStage = 2;
        break;
      } else {
        tutStage = 1;
        break;
      }
    case(2):
      drawMisc();
      drawTerrain();
      drawBike(0);
      pedal();
      brakeEct();
      movement();
      terrainLogic();
      arduboy.setCursor(0,8);
      arduboy.print("Now hold A");
      arduboy.display();
      if (pedalPos < -29) {
        tutStage = 3;
        break;
      } else {
        tutStage = 2;
        break;
      }
    case(3):
      drawMisc();
      drawTerrain();
      drawBike(0);
      pedal();
      brakeEct();
      movement();
      terrainLogic();
      arduboy.setCursor(3,9);
      arduboy.print(" ^ let's you know");
      arduboy.setCursor(0,17);
      arduboy.print("which button to press");
      //arduboy.display();
      screenTimer ++;
      if (screenTimer > 179) {tutStage = 4; screenTimer = 0;}
      break;
    case(4):
      drawMisc();
      drawTerrain();
      drawBike(0);
      setGear();
      pedal();
      brakeEct();
      movement();
      terrainLogic();
      screenTimer ++;
      if (gear < 2) {
        arduboy.setCursor(0,9);
        arduboy.print("use down and right");
        arduboy.setCursor(0,18);
        arduboy.print("to switch gears");
      } else {
        arduboy.setCursor(0,9);
        arduboy.print("Try 6th gear");
        if (gear > 5) {
          tutStage = 5;
          screenTimer = 0;
        }
      }
      break;
    case(5):
      drawMisc();
      drawTerrain();
      drawBike(0);
      setGear();
      pedal();
      brakeEct();
      movement();
      terrainLogic();
      screenTimer ++;
      if (screenTimer > 59) {tutStage = 6; screenTimer = 0;}
      break;
    case(6):
      drawTerrain();
      drawBike(0);
      setGear();
      pedal();
      brakeEct();
      movement();
      terrainLogic();
      const char *msgs[] = {
        "The higher the gear,",
        "the faster you'll go",
        "It will also",
        "use more stamina",
        "If you run out,",
        "you'll need a break",
        "That's all from me,",
        "enjoy the game!"
      };
      int step0 = screenTimer / 180;//60 frame per second
      int i = step0 * 2;//for text selection    
      arduboy.setCursor(0,0); arduboy.print(msgs[i]);
      arduboy.setCursor(0,8); arduboy.print(msgs[i + 1]);
      screenTimer ++;
      if (i > 4) {
        gear = 1;
        gamestate = MAIN_GAME;
      }
      break;
    }
}

//main game loop
void gameLoop() {
//LOGIC
  //set gear and PedalDif
  setGear();
  //pedalling logic
  pedal();
  //brake, wheelie, and stam logic
  brakeEct();
  //Speed and ground logic
  movement();

//RENDERING
  //Stamina, gear, pedals
  drawMisc();
  //Terrain
  drawTerrain();
  //Bike
  drawBike(0);

//END OF LOOP
  terrainLogic();
}

//splash screen
void splashScreen() { 
  if (screenTimer > 200 && screenTimer < 401) {
    linePos[0][1][0] = 64;
    linePos[0][1][1] = 50;
    linePos[1][1][0] = 50;
    arduboy.drawBitmap(0, 0, ArduBike, 128, 68);
    screenTimer ++;
  } else if (screenTimer > 400) {
      screenTimer = 0;
      gamestate = MENU;
  } else {
    arduboy.drawBitmap(20, 24, arduboy.arduboy_logo, 88, 16); //x, y, bitmap, width, height
    drawBike(screenTimer);
    screenTimer ++;
  }
}

//menu loop
void menuLoop() {
  arduboy.setCursor(0,0);
  arduboy.print("ArduBike");
  arduboy.setCursor(70,0);
  arduboy.print("A play");
  arduboy.setCursor(70,10);
  arduboy.print("B tutorial");
  drawTerrain();
  drawBike(0);
  lineTimer --;
  terrainLogic();
  if (arduboy.pressed(A_BUTTON)){
    gamestate = MAIN_GAME;
  }
  if (arduboy.pressed(B_BUTTON)){
    screenTimer = 0;
    gamestate = TUTORIAL;
  }
}
  
void setup() {
  Serial.begin(9600);
  arduboy.setFrameRate(60);
  //set up hardware for playing tones
  beep.begin();//for peddals
  beep2.begin();//for recharge and gear change
  //arduboy logo
  arduboy.begin();
  //set the text curser
  arduboy.setCursor(0,0);
  //initiate random seed
  arduboy.initRandomSeed();
  //set end of tarrain to be random
  linePos[1][1][1] = random(42, 64);
}

void loop() {
  //set 60 frame per second, polls buttons
  if (!arduboy.nextFrame()){return;}
  arduboy.pollButtons();
  beep.timer(); // once per frame to keep track of tones lengths
  beep2.timer();
  arduboy.clear();

  switch(gamestate) {
    case MAIN_GAME:
      gameLoop();
      break;
    case TUTORIAL:
      tutorialLoop(tutStage);
      break;
    case SPLASH_SCREEN:
      if (arduboy.pressed(RIGHT_BUTTON)) {
        gamestate = MENU;
      } else {
        splashScreen();
      }
      break;
    case MENU:
      menuLoop();
      break;
  }
  arduboy.display();
}
