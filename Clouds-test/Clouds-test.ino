
enum winSearchValues {CHILL, SEARCHING, FOUND_OFF, NO_FOUND_OFF};
byte winSearchValue;

bool isSearchingForWin;
bool isOn = true;
bool foundWin = false;
bool flashOn = false;


bool isWaitingOnNeighbor;
byte neighborSearchingForWin;    //changing index
byte indexOfNeighborToReportTo;  // use this for where we started our search


byte faceValues[6] = {CHILL, CHILL, CHILL, CHILL, CHILL, CHILL};


Timer slowTimer;
#define FRAME_DELAY 500

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
        neighborSearchingForWin = 0;
        //        indexOfNeighborToReportTo = 6;  //special for master blink

      }
    }

    if (isSearchingForWin) {
      if (isValueReceivedOnFaceExpired(neighborSearchingForWin)) { // no neighbor!
        neighborSearchingForWin = (neighborSearchingForWin + 1) % 6;

      }
      else { //found neighbor!

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

          if (neighborValue == SEARCHING) { //if neighbor face value is searching

            if (!isOn) { //if off
              faceValues[f] = FOUND_OFF;
              setValueSentOnFace(faceValues[f], f);
            }
            else { //if on
              // I've been asked to search
              indexOfNeighborToReportTo = f;
             // faceValues[f] = NO_FOUND_OFF;
              setValueSentOnFace(faceValues[f], f);
              neighborSearchingForWin = (f + 1) % 6; //move clockwise on faces
              isSearchingForWin = true;




            }

          }
          else { //when a face value is no longer searching
            if (neighborValue == NO_FOUND_OFF) {
              faceValues[f] = NO_FOUND_OFF;
              indexOfNeighborToReportTo = f;
              setValueSentOnFace(faceValues[f], f);
              isSearchingForWin = false;
              foundWin = true;
            }
            else {
             
            }





          }


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
  if (flashOn) {
    setColorOnFace(OFF, neighborSearchingForWin);
  }
}
