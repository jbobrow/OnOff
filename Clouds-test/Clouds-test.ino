
enum winSearchValues {CHILL, SEARCHING, FOUND_OFF, NO_FOUND_OFF};
byte winSearchValue;

bool isSearchingForWin;
bool isOn = false;
bool isWaitingOnNeighbor;
byte neighborSearchingForWin;
byte indexOfNeighborToReportTo;  // use this for where we started our search

byte faceValues[6] = {CHILL, CHILL, CHILL, CHILL, CHILL, CHILL};

Timer slowTimer;
#define FRAME_DELAY 500

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
        neighborSearchingForWin = 0;
      }
    }

    if (isSearchingForWin) {
      if (isValueReceivedOnFaceExpired(neighborSearchingForWin)) {
        neighborSearchingForWin++;
      }
      else { //found neighbor

        //if neighbor has been asked to search
        if (!isWaitingOnNeighbor) {

          byte neighborValue = getLastValueReceivedOnFace(neighborSearchingForWin);

          if (neighborValue == CHILL) {
            faceValues[neighborSearchingForWin] = SEARCHING;
            setValueSentOnFace(faceValues[neighborSearchingForWin], neighborSearchingForWin);
          }
          else if (neighborValue == SEARCHING ) {
            //do nothing, waiting for them to search
          }
          else if (neighborValue == FOUND_OFF) {
            isSearchingForWin = false;
            isWaitingOnNeighbor = false;
            neighborSearchingForWin = 0;
            faceValues[indexOfNeighborToReportTo] = FOUND_OFF;
            setValueSentOnFace(faceValues[indexOfNeighborToReportTo], indexOfNeighborToReportTo);
          }
          else if (neighborValue == NO_FOUND_OFF) {
            //Ask neighbor to search for off blinks
            faceValues[neighborSearchingForWin] = NO_FOUND_OFF;
            setValueSentOnFace(faceValues[neighborSearchingForWin], neighborSearchingForWin);
            isWaitingOnNeighbor = true;

          }

        }
      }
    }
    else // not yet searching, listening for the opportunity to participate
    {
      // look to present neighbors
      FOREACH_FACE(f) {
        if (!isValueReceivedOnFaceExpired(f)) {
          byte neighborValue = getLastValueReceivedOnFace(f);

          if (neighborValue == SEARCHING) {
            
            if(!isOn) {
              faceValues[f] = FOUND_OFF;
              setValueSentOnFace(faceValues[f], f);
            }
            else {
              // I've been asked to search
              indexOfNeighborToReportTo = f;
              isSearchingForWin = true;
              neighborSearchingForWin = f+1;
            }
          }
        }
      }




      // Listen for message from neighbor to search for any off blinks
      // if I am off,
      // return message to neighbor asking me to search; message should say "i am an off blink"
      // else if any of my neighbors are not yet searched blinks
      // then ask them to search for any off blinks

      //setValueSentOnAllFaces(neighborSearchingForWin);

    }
    if (isOn) {
      setColor(WHITE);
    }
    else {
      setColor(dim(BLUE, 64));
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
          setColorOnFace(YELLOW, f);
          break;
        case FOUND_OFF:
          setColorOnFace(RED, f);
          break;
        case NO_FOUND_OFF:
          setColorOnFace(GREEN, f);
          break;

      }
    }
  }
  setColorOnFace(BLUE,0);
}
