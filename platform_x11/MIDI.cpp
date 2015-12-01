// Dummy midi implementation, just to make sure bonzomatic compiles.
#include "../MIDI.h"

bool MIDI::Open() {
	return false;
}

bool MIDI::Close() {
	return false;
}

float MIDI::GetCCValue( unsigned char cc) {
	return 0.f;
}

