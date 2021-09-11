/*
   On-Off Clouds Game

   Basic shared game state for play, win, and reset
*/

bool isOn = false;
bool wasPressed = false;
bool b_buttonSingleClicked;
bool b_buttonLongPressed;

enum pressStates {INERT, PRESS, FLIP, RESOLVE, RESET, RESET_RESOLVE};
byte myPressState = INERT;

byte searchState[6] = {0, 0, 0, 0, 0, 0}; // place holder

Timer resetTimer;
#define RESET_DURATION 500

Timer slowTimer;
#define SLOW_STEP_DURATION 500

/*
   WIN CONDITION ANIMATION VARIABLES
*/
//Silver Lining Constants for delta
#define SPARKLE_DURATION        800
#define SPARKLE_CYCLE_DURATION  1600


//Sun Spot Values
byte sunSpot_hue = 30;
byte setupFadeFace;
Timer setupFadeTimer;
word backgroundTime;
#define SETUP_FADE_UP_INTERVAL 1000
#define SETUP_RED_INTERVAL 500
#define SETUP_FADE_DELAY 4000


//Timers

Timer fadeToBright;
Timer sunSpotFade;
Timer fadeToCloud;
Timer fadeToDark;

#define FADE_TO_BRIGHT_DELAY 2000
#define SUN_SPOT_FADE 1000
#define FADE_TO_CLOUD_DELAY 3000
#define FADE_TO_DARK_DELAY 1000

//Shared states
enum modeStates {LOSE, WIN};
byte currentMode;


void setup() {
  randomize();
}

void loop() {

  //  if (slowTimer.isExpired()) {
  //    slowTimer.set(SLOW_STEP_DURATION);
  b_buttonSingleClicked = buttonSingleClicked();
  b_buttonLongPressed = buttonLongPressed();

  // game logic
  switch (myPressState) {
    case INERT:
      inertLoop();
      break;
    case PRESS:
      pressLoop();
      break;
    case FLIP:
      flipLoop();
      break;
    case RESOLVE:
      resolveLoop();
      break;
    case RESET:
      resetLoop();
      break;
    case RESET_RESOLVE:
      resetResolveLoop();
      break;
  }

  // display
  if (isOn) {
    setColor(WHITE);
  }
  else {
    setColor(OFF);
    setColorOnFace(dim(BLUE, 128), 0);
    setColorOnFace(dim(BLUE, 128), 2);
    setColorOnFace(dim(BLUE, 128), 4);
  }

  if (!resetTimer.isExpired()) {
    setColor(dim(WHITE, resetTimer.getRemaining() / 2));
  }

  // debug visuals

  //    switch (myPressState) {
  //      case INERT:  setColorOnFace(GREEN, 0); break;
  //      case PRESS:  setColorOnFace(ORANGE, 0); break;
  //      case FLIP:  setColorOnFace(YELLOW, 0); break;
  //      case RESOLVE:  setColorOnFace(BLUE, 0); break;
  //      case RESET:  setColorOnFace(RED, 0); break;
  //      case RESET_RESOLVE:  setColorOnFace(MAGENTA, 0); break;
  //    }

  if (searchState[0] == 5) { // VICTORY
    setColor(WHITE);
    fadeToNoLight();
    if (fadeToDark.isExpired()) {
      silverLiningDisplay();
    }
  }

  // communicate with neighbors
  // share both signalState (i.e. when to change) and the game mode
  FOREACH_FACE(f) {
    byte sendData = (searchState[f] << 3) + (myPressState);
    setValueSentOnFace(sendData, f);
  }
  //  }
}


/*
   Win Loop
   do this when the board has determined it is a win
*/
void winLoop() {

}


/*
   This loop looks for a GO signalState
   Also gets the new gameMode
*/
void inertLoop() {

  if (b_buttonSingleClicked) {
    myPressState = PRESS;
    wasPressed = true;
  }

  if (b_buttonLongPressed) {
    myPressState = RESET;
  }

  //listen for neighbors
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//a neighbor!
      if (getPressState(getLastValueReceivedOnFace(f)) == PRESS) {//a neighbor saying PRESS!
        myPressState = FLIP;
      }
      if (getPressState(getLastValueReceivedOnFace(f)) == RESET) {//a neighbor saying RESET!
        myPressState = RESET;
      }
    }
  }

  // general win search
  checkForWin();
}

/*
   Flip Loop
*/
void flipLoop() {

  myPressState = RESOLVE; // if I don't see a press

  //listen for neighbors
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//a neighbor!
      if (getPressState(getLastValueReceivedOnFace(f)) == PRESS) {//a neighbor saying PRESS!
        myPressState = FLIP;  // remain in flip
      }
    }
  }
}

/*
   If all of my neighbors are in GO or RESOLVE, then I can RESOLVE
*/
void pressLoop() {
  myPressState = RESOLVE;//I default to this at the start of the loop. Only if I see a problem does this not happen

  //look for neighbors who have not heard the PRESS news
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//a neighbor!
      if (getPressState(getLastValueReceivedOnFace(f)) != FLIP) {
        myPressState = PRESS;
      }
    }
  }
}

/*
   This loop returns me to inert once everyone around me has RESOLVED
   Now receive the game mode
*/
void resolveLoop() {
  myPressState = INERT;//I default to this at the start of the loop. Only if I see a problem does this not happen

  //look for neighbors who have not moved to RESOLVE
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//a neighbor!
      if (getPressState(getLastValueReceivedOnFace(f)) != RESOLVE && getPressState(getLastValueReceivedOnFace(f)) != INERT) {//This neighbor isn't in RESOLVE. Stay in RESOLVE
        myPressState = RESOLVE;
      }
    }
  }

  if (myPressState == INERT) {
    // toggle on/off
    isOn = !isOn;

    if (wasPressed) {
      // TODO: check for win condition
      wasPressed = false;
      beginCheck();
    }

  }
}


/*
   Reset Loop
*/
void resetLoop() {
  myPressState = RESET_RESOLVE; // if I don't see a RESET

  //listen for neighbors
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//a neighbor!
      if (getPressState(getLastValueReceivedOnFace(f)) != RESET && getPressState(getLastValueReceivedOnFace(f)) != RESET_RESOLVE) {//a neighbor saying RESET!
        myPressState = RESET;  // remain in flip
      }
    }
  }

}

void resetResolveLoop() {
  myPressState = INERT;//I default to this at the start of the loop. Only if I see a problem does this not happen

  //look for neighbors who have not moved to RESOLVE
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//a neighbor!
      if (getPressState(getLastValueReceivedOnFace(f)) != RESET_RESOLVE && getPressState(getLastValueReceivedOnFace(f)) != INERT) {//This neighbor isn't in RESOLVE. Stay in RESOLVE
        myPressState = RESET_RESOLVE;
      }
    }
  }

  if (myPressState == INERT) {
    // reset by all going dark
    isOn = false;
    // animate reset
    resetTimer.set(RESET_DURATION);
    // reset win condition search
    resetPiece();
  }
}


byte getSearchState(byte data) {
  return ((data >> 3) & 7);
}

byte getPressState(byte data) {
  return (data & 7);
}

/*
   WIN CONDITION ANIMATION
*/

void initWin() {
  //Set Timers
  setupFadeTimer.set(backgroundTime + SETUP_FADE_UP_INTERVAL + random(SETUP_FADE_DELAY));
  fadeToBright.set(FADE_TO_BRIGHT_DELAY);
  fadeToDark.set(FADE_TO_DARK_DELAY);
  fadeToCloud.set(FADE_TO_CLOUD_DELAY);
}

void fadeToLight() {
  FOREACH_FACE(f) {
    // minimum of 125, maximum of 255
    byte brightness = 255 -  map(fadeToBright.getRemaining(), 0, FADE_TO_BRIGHT_DELAY, 0, 255);
    Color faceColor = makeColorHSB(0, 0, brightness);
    setColorOnFace(faceColor, f);
  }
}


void fadeToNoLight() {
  FOREACH_FACE(f) {
    byte brightness = map(fadeToDark.getRemaining(), 0, FADE_TO_DARK_DELAY, 0, 255);
    Color faceColor = makeColorHSB(0, 0, brightness);
    setColorOnFace(faceColor, f);
  }
}

void silverLiningDisplay() {
  Color faceColor_lining;
  Color faceColor_cloud;

  FOREACH_FACE(f) {
    // minimum of 125, maximum of 255
    byte phaseShift = 60 * f;
    byte amplitude = 55;
    byte midline = 185;
    byte rate = 10;
    byte brightness = 255 -  map(fadeToBright.getRemaining(), 0, FADE_TO_BRIGHT_DELAY, 0, 255);
    byte cloudBrightness = 255 -  map(fadeToCloud.getRemaining(), 0, FADE_TO_CLOUD_DELAY, 0, 255);
    if (!fadeToCloud.isExpired()) {

      faceColor_lining = makeColorHSB(0, 0, brightness);
      faceColor_cloud = makeColorHSB(160, brightness, cloudBrightness);

    }

    if (fadeToBright.isExpired()) {
      faceColor_lining = makeColorHSB(0, 0, 255);
      faceColor_cloud = makeColorHSB(160, 0, 255);

    }
    silverLining(faceColor_cloud, faceColor_lining);
  }
}




void silverLining(Color faceColor_Cloud, Color faceColor_Lining) {

  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) { //if something there aka if within the cloud

      Color fadeColor;
      byte saturation;
      byte hue;
      // have the color on the Blink raise and lower to feel more alive
      byte bri = 185 + sin8_C( (millis() / 14) % 255) * 70 / 255; // oscillate between values 185and 255

      // lets do a celebration on each face in an order
      word delta = millis() % SPARKLE_CYCLE_DURATION; // 2 second cycle

      if (delta > SPARKLE_DURATION) {
        delta = SPARKLE_DURATION;
      }


      if (setupFadeTimer.isExpired()) { //fade timer for sun spots
        setupFadeFace = f; //assign face
        backgroundTime = SETUP_RED_INTERVAL + random(SETUP_RED_INTERVAL / 2); //bit of randomness
        setupFadeTimer.set(backgroundTime + SETUP_FADE_UP_INTERVAL + random(SETUP_FADE_DELAY));
      }


      if (setupFadeTimer.getRemaining() < backgroundTime + SETUP_FADE_UP_INTERVAL) {//we are inside the animation
        if (setupFadeTimer.getRemaining() < SETUP_FADE_UP_INTERVAL) {//we are fading
          saturation = 255 - map(setupFadeTimer.getRemaining(), 0, SETUP_FADE_UP_INTERVAL, 0, 255);
          fadeColor = makeColorHSB(160, saturation, bri); //sun spot fade
        } else {
          sunSpotFade.set(SUN_SPOT_FADE);
          if (!sunSpotFade.isExpired()) {
            saturation =  map(sunSpotFade.getRemaining(), SUN_SPOT_FADE, 0, 0, 255);
            fadeColor = makeColorHSB(sunSpot_hue, saturation, bri); //sunspot burst
          }

        }

        setColorOnFace(fadeColor, setupFadeFace); //set sun spot colors
      }

      fadeColor = makeColorHSB(160, 255, bri); //cloud colour
      setColorOnFace(fadeColor, f);  //set cloud colours
    }


    else {
      setColorOnFace(faceColor_Lining, f); // if not within the cloud, display the lining color (edges)

    }

  }
}
