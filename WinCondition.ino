enum winSearchValues {CHILL, SEARCHING, WAITING, FOUND_OFF, NO_FOUND_OFF, VICTORY, DEFEAT, WIN_RESOLVE};

bool isSearchingForWin;
bool isWaitingOnNeighbor;
bool foundWin = false;
byte neighborSearchingForWin;    //changing index
byte indexOfNeighborToReportTo = 6;  // use this for where we started our search

void beginCheck() {
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

void checkForWin() {
  if (isSearchingForWin) {

    //check if done
    if (isDoneSearching()) {
      isSearchingForWin = false;
      if (indexOfNeighborToReportTo == 6) { // if I am the origin
        if (!foundWin) {
          initWin();
        }
        foundWin = true;
        setAllTo(VICTORY);
      }
    }

    if (isValueReceivedOnFaceExpired(neighborSearchingForWin)) { // no neighbor!
      searchState[neighborSearchingForWin] = NO_FOUND_OFF;
      neighborSearchingForWin = (neighborSearchingForWin + 1) % 6;
    }
    else { //found neighbor!
      if (neighborSearchingForWin == indexOfNeighborToReportTo) {
        searchState[neighborSearchingForWin] = NO_FOUND_OFF;
        searchState[indexOfNeighborToReportTo] = NO_FOUND_OFF;
        isSearchingForWin = false;
      }
      else {

        byte neighborValue = getSearchState(getLastValueReceivedOnFace(neighborSearchingForWin));

        if (!isWaitingOnNeighbor) { // if neighbor has not yet been asked to search

          if (neighborValue == CHILL) {
            searchState[neighborSearchingForWin] = WAITING;
            isWaitingOnNeighbor = true;
          }
          else if (neighborValue == SEARCHING ) {
            //ignore and move on to the next one
            searchState[neighborSearchingForWin] = NO_FOUND_OFF;
            neighborSearchingForWin = (neighborSearchingForWin + 1) % 6;  // continue looking to the next one
          }
          else if (neighborValue == FOUND_OFF) {
            isSearchingForWin = false;
            neighborSearchingForWin = 0;
            searchState[indexOfNeighborToReportTo] = FOUND_OFF;
          }
          else if (neighborValue == NO_FOUND_OFF) {
            //Ask neighbor to search for off blinks
            searchState[neighborSearchingForWin] = NO_FOUND_OFF;
            neighborSearchingForWin = (neighborSearchingForWin + 1) % 6;  // continue looking to the next one
          }

        } // end not waiting on neighbor
        else {  // am waiting on neighor

          // let the one I am reporting to know I am waiting on a neighbor
          if (indexOfNeighborToReportTo != 6) { // if I have someone to report to
            searchState[indexOfNeighborToReportTo] = WAITING;
          }

          if (neighborValue == FOUND_OFF) { // just heard back from neighbor that they FOUND AN OFF
            searchState[neighborSearchingForWin] = FOUND_OFF;
            isSearchingForWin = false;
            isWaitingOnNeighbor = false;
            neighborSearchingForWin = 0;
            searchState[indexOfNeighborToReportTo] = FOUND_OFF;
            // if I am the starting point..
            // signal no victory, stop the search, cuz we found an off
            if (indexOfNeighborToReportTo == 6) {
              setAllTo(DEFEAT);
            }
          }
          else if (neighborValue == NO_FOUND_OFF) { // just heard back from neighbor that they haven't found and OFF
            // Acknowledge and move on to the next one
            searchState[neighborSearchingForWin] = NO_FOUND_OFF;
            isWaitingOnNeighbor = false;
            // if I am the starting point..
            // signal victory, cuz we searched all and didn't find an OFF

            if (indexOfNeighborToReportTo != 6) { // I'm not the starting point
              neighborSearchingForWin = (neighborSearchingForWin + 1) % 6;
            }
            else { // I'm the starting point
              if (neighborSearchingForWin == 5) { // last side I will search
                // signal victory, cuz we searched all and didn't find an OFF
                isSearchingForWin = false;
                if (!foundWin) {
                  initWin();
                }
                foundWin = true;
                setAllTo(VICTORY);
              }
              else {
                // advance to the next side, if we have search all sides, stop searching
                neighborSearchingForWin = (neighborSearchingForWin + 1) % 6;
              }
            }
          }
        } //end waiting on neighbor
      }
    }
  }
  else // not yet searching, listening for the opportunity to participate
  {

    //      if(searchState[0] == WIN_RESOLVE) {
    //        byte value = CHILL;
    //        FOREACH_FACE(f) {
    //        if (!isValueReceivedOnFaceExpired(f)) {
    //          byte neighborValue = getLastValueReceivedOnFace(f);
    //          if(neighborValue == VICTORY) {
    //            value = WIN_RESOLVE;
    //          }
    //        }
    //      }

    // look to present neighbors
    if (searchState[0] == VICTORY) {
      if (b_buttonSingleClicked || b_buttonLongPressed) {
        setAllTo(DEFEAT);
      }
    }

    FOREACH_FACE(f) {
      if (!isValueReceivedOnFaceExpired(f)) {
        byte neighborValue = getSearchState(getLastValueReceivedOnFace(f));


        if (searchState[0] == DEFEAT) {
          byte value = WIN_RESOLVE;

          //listen for neighbors
          FOREACH_FACE(f) {
            if (!isValueReceivedOnFaceExpired(f)) {//a neighbor!
              if (getSearchState(getLastValueReceivedOnFace(f)) != DEFEAT && getSearchState(getLastValueReceivedOnFace(f)) != WIN_RESOLVE) {//a neighbor saying
                value = DEFEAT;  // remain in DEFEAT
              }
            }
          }

          setAllTo(value);
        }
        else if (searchState[0] == WIN_RESOLVE) {
          byte value = CHILL;

          //listen for neighbors
          FOREACH_FACE(f) {
            if (!isValueReceivedOnFaceExpired(f)) {//a neighbor!
              if (getSearchState(getLastValueReceivedOnFace(f)) != WIN_RESOLVE && getSearchState(getLastValueReceivedOnFace(f)) != CHILL) {//a neighbor saying
                value = WIN_RESOLVE;  // remain in VICTORY
              }
            }
          }

          setAllTo(value);
          if (value == CHILL) {
            resetPiece();
          }
        }
        else {
          if (neighborValue == VICTORY) {
            if (!foundWin) {
              initWin();
              foundWin = true;
            }
            setAllTo(VICTORY);  // spread the victory
          }
          else if (neighborValue == DEFEAT) {
            setAllTo(DEFEAT);  // spread the defeat
          }

          if (neighborValue == WAITING && f != indexOfNeighborToReportTo) { //if neighbor face value is searching

            // set all to searching
            setAllTo(SEARCHING);

            if (!isOn) { //if off
              searchState[f] = FOUND_OFF;
              setAllTo(FOUND_OFF);
            }
            else { //if on
              // I've been asked to search
              indexOfNeighborToReportTo = f;
              searchState[f] = WAITING;
              neighborSearchingForWin = (f + 1) % 6; //move clockwise on faces
              isSearchingForWin = true;
            }

          }

        } // end if neighbor is searching

      }// end if neighbor is present


    } // end for loop
  } // end not yet searching

  buttonPressed();
}

void resetPiece() {
  setAllTo(CHILL);
  isSearchingForWin = false;
  isWaitingOnNeighbor = false;
  foundWin = false;
  neighborSearchingForWin = 0;
  indexOfNeighborToReportTo = 6;
}

void setAllTo(byte state) {
  FOREACH_FACE(f) {
    searchState[f] = state;
  }
}

bool isDoneSearching() {
  FOREACH_FACE(f) {
    if (searchState[f] == CHILL || searchState[f] == WAITING || searchState[f] == SEARCHING) {
      return false;
    }
  }
  return true;
}
