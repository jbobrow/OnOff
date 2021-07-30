
enum winSearchValues {CHILL, SEARCHING, WAITING, FOUND_OFF, NO_FOUND_OFF};
byte winSearchValue;

bool isSearchingForWin;
bool isOn = true;
bool foundWin = false;
bool flashOn = false;
bool foundSearcher = false;

bool isWaitingOnNeighbor;
byte neighborSearchingForWin;    //changing index
byte indexOfNeighborToReportTo = 6;  // use this for where we started our search


byte faceValues[6] = {CHILL, CHILL, CHILL, CHILL, CHILL, CHILL};


Timer slowTimer;
#define FRAME_DELAY 200

Timer flashTimer;
#define FLASH_DELAY 100

void setup() {


}

void loop() {

  if (slowTimer.isExpired()) {
    slowTimer.set(FRAME_DELAY);

    if (buttonSingleClicked()) {
      // I am one that was clicked
      isOn = !isOn;

    }
    if (buttonDoubleClicked()) {

      //Start Search for any off blinks
      //If I am an off blink, no need to search
      if (!isOn) {
        isSearchingForWin = false;
      }
      else {
        //Else Ask each neighbor to search for off blinks
        isSearchingForWin = true;
        setAllTo(SEARCHING);
        neighborSearchingForWin = 0;
        indexOfNeighborToReportTo = 6;  //special for master blink
      }
    }

    if (isSearchingForWin) {

      if (isValueReceivedOnFaceExpired(neighborSearchingForWin)) { // no neighbor!
        faceValues[neighborSearchingForWin] = NO_FOUND_OFF;
        neighborSearchingForWin = (neighborSearchingForWin + 1) % 6;
      }
      else { //found neighbor!
        if (neighborSearchingForWin == indexOfNeighborToReportTo) {
          faceValues[neighborSearchingForWin] = NO_FOUND_OFF;
          faceValues[indexOfNeighborToReportTo] = NO_FOUND_OFF;
          isSearchingForWin = false;
        }
        else {

          byte neighborValue = getLastValueReceivedOnFace(neighborSearchingForWin);

          if (neighborValue == SEARCHING ) {
            //do nothing, waiting for them to search
          }

          //if neighbor has been asked to search
          if (!isWaitingOnNeighbor) {


            if (neighborValue == CHILL) {
              faceValues[neighborSearchingForWin] = WAITING;
              isWaitingOnNeighbor = true;
            }
            else if (neighborValue == SEARCHING ) {
              //do nothing, waiting for them to search
              neighborSearchingForWin = (neighborSearchingForWin + 1) % 6;  // continue looking to the next one
            }
            else if (neighborValue == FOUND_OFF) {
              isSearchingForWin = false;
              neighborSearchingForWin = 0;
              faceValues[indexOfNeighborToReportTo] = FOUND_OFF;
            }
            else if (neighborValue == NO_FOUND_OFF) {
              //Ask neighbor to search for off blinks
              faceValues[neighborSearchingForWin] = NO_FOUND_OFF;
            }

          } // end not waiting on neighbor
          else {  // am waiting on neighor

            // let the one I am reporting to know I am waiting on a neighbor
            if (indexOfNeighborToReportTo != 6) {
              faceValues[indexOfNeighborToReportTo] = WAITING;
            }

            if (neighborValue == FOUND_OFF) {
              faceValues[neighborSearchingForWin] = FOUND_OFF;
              isSearchingForWin = false;
              isWaitingOnNeighbor = false;
              neighborSearchingForWin = 0;
              faceValues[indexOfNeighborToReportTo] = FOUND_OFF;
            }
            else if (neighborValue == NO_FOUND_OFF) {
              // Acknowledge and move on to the next one
              faceValues[neighborSearchingForWin] = NO_FOUND_OFF;
              isWaitingOnNeighbor = false;
              neighborSearchingForWin = (neighborSearchingForWin + 1) % 6;  //
            }
          } //end waiting on neighbor
        }
      }
    }
    else // not yet searching, listening for the opportunity to participate
    {
      // look to present neighbors
      FOREACH_FACE(f) {
        if (!isValueReceivedOnFaceExpired(f)) {
          byte neighborValue = getLastValueReceivedOnFace(f);


          if (neighborValue == WAITING && f != indexOfNeighborToReportTo) { //if neighbor face value is searching

            // set all to searching
            setAllTo(SEARCHING);

            if (!isOn) { //if off
              faceValues[f] = FOUND_OFF;
              setAllTo(FOUND_OFF);
            }
            else { //if on
              // I've been asked to search
              indexOfNeighborToReportTo = f;
              faceValues[f] = WAITING;
              neighborSearchingForWin = (f + 1) % 6; //move clockwise on faces
              isSearchingForWin = true;
            }

          } // end if neighbor is searching

        }// end if neighbor is present


      } // end for loop
    } // end not yet searching

  } // end slow timer
  FOREACH_FACE(f) {
    setValueSentOnFace(faceValues[f], f);
  }

  // Listen for message from neighbor to search for any off blinks
  // if I am off,
  // return message to neighbor asking me to search; message should say "i am an off blink"
  // else if any of my neighbors are not yet searched blinks
  // then ask them to search for any off blinks


  if (isOn) {
    setColor(WHITE);
  }
  else {
    setColor(dim(BLUE, 64));
  }


  if (foundWin) {
    FOREACH_FACE(f) {
      setColorOnFace(ORANGE, f);
    }
  }


  // DEBUG VISUALIZATION
  FOREACH_FACE(f) {
    switch (faceValues[f]) {
      case CHILL:
        {
          if (isOn) {
            setColorOnFace(WHITE, f);
          }
          else {
            setColorOnFace(dim(BLUE, 64), f);
          }

        }
        break;
      case SEARCHING:
        setColorOnFace(BLUE, f);
        break;
      case WAITING:
        setColorOnFace(YELLOW, f);
        break;
      case FOUND_OFF:
        setColorOnFace(RED, f);
        break;
      case NO_FOUND_OFF:
        setColorOnFace(GREEN, f);
        break;

    }
  } //END DEBUG VISUALIZATION
  if (flashTimer.isExpired()) {
    flashOn = !flashOn;
    flashTimer.set(FLASH_DELAY);

  }
  if (flashOn && isSearchingForWin) {
    setColorOnFace(OFF, neighborSearchingForWin);
  }

}

void setAllTo(byte state) {
  FOREACH_FACE(f) {
    faceValues[f] = state;
  }
}
