
enum winSearchValues {CHILL, SEARCHING, FOUND_OFF, NO_FOUND_OFF};
byte winSearchValue;

bool isSearchingForWin;
bool isOn = false;
bool isWaitingOnNeighbor;
byte neighborSearchingForWin;

void setup() {


}

void loop() {

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
          neighborSearchingForWin++;
        }
        else if (neighborValue == SEARCHING ) {
          //do nothing, waiting for them to search
        }
        else if (neighborValue == FOUND_OFF) {
          isSearchingForWin = false;
          isWaitingOnNeighbor = false;
          neighborSearchingForWin = 0;
        }
        else {
          //Ask neighbor to search for off blinks
          setValueSentOnFace(NO_FOUND_OFF, neighborSearchingForWin);
          isWaitingOnNeighbor = true;

        }

      }
    }




    // Listen for message from neighbor to search for any off blinks
    // if I am off,
    // return message to neighbor asking me to search; message should say "i am an off blink"
    // else if any of my neighbors are not yet searched blinks
    // then ask them to search for any off blinks

    setValueSentOnAllFaces(neighborSearchingForWin);

  }
  if (isOn) {
    setColor(WHITE);
  }
  else {
    setColor(dim(BLUE, 64));
  }

FOREACH_FACE(f){
  switch (f) {
    case CHILL:
      setColorOnFace(OFF, neighborSearchingForWin);
      continue;
    case SEARCHING:
      setColorOnFace(YELLOW, neighborSearchingForWin);
      continue;
    case FOUND_OFF:
      setColorOnFace(RED, neighborSearchingForWin);
      break;
    case NO_FOUND_OFF:
      setColorOnFace(GREEN, neighborSearchingForWin);
      break;

  }
}

}
