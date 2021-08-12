/*
   On-Off Clouds Game

   Basic shared game state for play, win, and reset
*/

bool isOn = false;
bool wasPressed = false;

enum pressStates {INERT, PRESS, FLIP, RESOLVE, RESET, RESET_RESOLVE};
byte myPressState = INERT;

byte searchState[6] = {0, 0, 0, 0, 0, 0}; // place holder

Timer resetTimer;
#define RESET_DURATION 500

Timer slowTimer;
#define SLOW_STEP_DURATION 500

void setup() {

}

void loop() {

//  if (slowTimer.isExpired()) {
//    slowTimer.set(SLOW_STEP_DURATION);

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

    if(!resetTimer.isExpired()) {
      setColor(dim(WHITE, resetTimer.getRemaining()/2));
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

  if (buttonSingleClicked()) {
    myPressState = PRESS;
    wasPressed = true;
  }

  if (buttonDoubleClicked()) {
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
    
    if(wasPressed) {
      // TODO: check for win condition
      wasPressed = false;
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
  }
}


byte getSearchState(byte data) {
  return ((data >> 3) & 7);
}

byte getPressState(byte data) {
  return (data & 7);
}
