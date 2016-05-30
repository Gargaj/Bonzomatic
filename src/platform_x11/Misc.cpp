// Dummy Keymap functions that don't actually map anything,
// just to fix the SDL build.

#include <string.h>
#include "../Misc.h"

void Misc::InitKeymaps() {
	return;
}

void Misc::GetKeymapName(char* sz) {
	strncpy(sz,"<native>",7);
}
