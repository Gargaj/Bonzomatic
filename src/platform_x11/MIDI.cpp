// Dummy midi implementation, just to make sure bonzomatic compiles.
#include "../MIDI.h"

#include <alsa/asoundlib.h>
#include <stdio.h>

static struct {
	snd_rawmidi_t *midi;
	unsigned char nCCValues[256];
} g;

bool MIDI::Open() {
	// TODO enumerate
	const char *dev_name = getenv("MIDI_DEV");
	if (!dev_name)
		return false;

	int err = snd_rawmidi_open(&g.midi, NULL, dev_name, SND_RAWMIDI_NONBLOCK);
	if (err < 0) {
		fprintf(stderr, "rawmidi_open(%s): %s\n", dev_name, snd_strerror(err));
		return false;
	}
	fprintf(stderr, "Opened midi device %s\n", dev_name);

	return true;
}

bool MIDI::Close() {
	if (g.midi)
		snd_rawmidi_close(g.midi);
	return true;
}

float MIDI::GetCCValue(unsigned char cc) {
	if (!g.midi)
		return 0;

	unsigned char midi_buf[1024];
	const int err = snd_rawmidi_read(g.midi, midi_buf, sizeof(midi_buf));
	if (err < 0 && err != -EAGAIN) {
		fprintf(stderr, "midi_read: %s\n", snd_strerror(err));
	}
	if (err > 0) {
		for (int i = 0; i < err; ++i) {
			const unsigned char b = midi_buf[i];
			if (!(b & 0x80))
				continue; /* skip unsync data bytes */

			if (err - i < 3)
				break; /* expect at least 2 args */

			const int channel = b & 0x0f;
			if ((b & 0xf0) == 0xb0) { /* control change */
				const int controller = midi_buf[i + 1];
				const int value = midi_buf[i + 2];
				//fprintf(stderr, "MidiControl ch=%d p=%d v=%d\n", channel, controller, value);
				g.nCCValues[controller] = value;
				i += 2;
			}
		}
	}

	return g.nCCValues[cc] / 127.0f;
}
