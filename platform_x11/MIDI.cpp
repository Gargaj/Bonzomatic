#include <stdio.h>

// A blank midi class that needs some work on X11 and Apple :S

namespace MIDI
{
  
  void MyMidiInProc( ) {
  }

  bool Open() {
    return false;
  }

  bool Close() {
    return true;
  }

  float GetCCValue( unsigned char cc )
  {
    return 0.0;
  }

};
