// Dummy midi implementation, just to make sure bonzomatic compiles.
#include "../MIDI.h"

#include <alsa/asoundlib.h>
#include <stdio.h>

#define MAX_MIDI_DEVICES 8

struct AlsaMidiDevice {
	snd_rawmidi_t *handle;
	char name[32];
	int errors;
};

static struct {
	int num_devices;
	AlsaMidiDevice devices[MAX_MIDI_DEVICES];
	unsigned char nCCValues[256];
	int verbose;
} g;

#define MSG_ERR(msg, ...) fprintf(stderr, "[MIDI] " msg "\n", ## __VA_ARGS__)
#define MSG(msg, ...) fprintf(stderr, "[MIDI] " msg "\n", ## __VA_ARGS__)

static int enumerateDevices() {
	int card = -1, found = 0;
	snd_rawmidi_info_t *info;
	snd_rawmidi_info_alloca(&info);

	for (;;) {
		int err = snd_card_next(&card);
		if (err < 0) {
			MSG_ERR("Failed to get next ALSA card index: %s", snd_strerror(err));
			break;
		}
		if (card < 0)
			break;

		snd_ctl_t *ctl = NULL;
		char card_name[16];
		sprintf(card_name, "hw:%d", card);
		err = snd_ctl_open(&ctl, card_name, 0);
		if (err < 0) {
			MSG_ERR("Failed to open control for card %s: %s", card_name, snd_strerror(err));
			continue;
		}

		int device = -1;
		for (;;) {
			err = snd_ctl_rawmidi_next_device(ctl, &device);
			if (err < 0) {
				MSG_ERR("Failed to get device number for %s: %s", card_name, snd_strerror(err));
				break;
			}
			if (device < 0)
				break;

			snd_rawmidi_info_set_device(info, device);

			snd_rawmidi_info_set_stream(info, SND_RAWMIDI_STREAM_INPUT);
			err = snd_ctl_rawmidi_info(ctl, info);
			const int inputs = err < 0 ? 0 : snd_rawmidi_info_get_subdevices_count(info);
			for (int i = 0; i < inputs; ++i) {
				snd_rawmidi_info_set_subdevice(info, i);
				err = snd_ctl_rawmidi_info(ctl, info);
				if (err < 0) {
					MSG_ERR("Failed to get rawmidi info on device hw:%d,%d,%d: %s", card, device, i, snd_strerror(err));
					continue;
				}
				const char *name = snd_rawmidi_info_get_name(info);
				const char *subdevice_name = snd_rawmidi_info_get_subdevice_name(info);

				if (g.num_devices == MAX_MIDI_DEVICES) {
					MSG_ERR("Cannot add device hw:%d,%d,%d (%s, %s): maximum number of MIDI devices (%d) reached",
						card, device, i, name, subdevice_name, MAX_MIDI_DEVICES);
					continue;
				}

				AlsaMidiDevice *dev = g.devices + g.num_devices;
				snprintf(dev->name, sizeof(dev->name), "hw:%d,%d,%d", card, device, i);
				err = snd_rawmidi_open(&dev->handle, NULL, dev->name, SND_RAWMIDI_NONBLOCK);
				if (err < 0) {
					MSG_ERR("Failed to open device %s (%s, %s): %s", dev->name, name, subdevice_name, snd_strerror(err));
					continue;
				}

				MSG("Opened device %s (%s, %s)", dev->name, name, subdevice_name);
				g.num_devices++;
			}
		}
		snd_ctl_close(ctl);
	}
	return g.num_devices;
}

bool MIDI::Open() {
	g.num_devices = 0;
	g.verbose = !!getenv("MIDI_VERBOSE");
	const char *dev_name = getenv("MIDI_DEV");
	if (dev_name) {
		const int err = snd_rawmidi_open(&g.devices[0].handle, NULL, dev_name, SND_RAWMIDI_NONBLOCK);
		if (err < 0) {
			MSG_ERR("Failed to open device %s: %s", dev_name, snd_strerror(err));
			return false;
		}
		MSG("Opened device %s", dev_name);
		strncpy(g.devices[0].name, dev_name, sizeof(g.devices[0].name)-1);
		g.devices[0].name[sizeof(g.devices[0].name)-1] = '\0';
		g.num_devices = 1;

		return true;
	}

	if (enumerateDevices())
		return true;

	MSG("No devices were connected");
	return false;
}

bool MIDI::Close() {
	for (int i = 0; i < g.num_devices; ++i) {
		AlsaMidiDevice *dev = g.devices + i;
		if (dev->handle) {
			snd_rawmidi_close(dev->handle);
			dev->handle = NULL;
		}
	}
	return true;
}

float MIDI::GetCCValue(unsigned char cc) {
	for (int d = 0; d < g.num_devices; ++d) {
		AlsaMidiDevice *dev = g.devices + d;
		if (!dev->handle)
			continue;
		unsigned char midi_buf[1024];
		const int err = snd_rawmidi_read(dev->handle, midi_buf, sizeof(midi_buf));
		if (err < 0 && err != -EAGAIN) {
			MSG_ERR("Failed to read stream from device %s: %s", dev->name, snd_strerror(err));
			++dev->errors;
			if (dev->errors == 10) {
				MSG_ERR("Failed to read device %s 10 times in a row, disabling", dev->name);
				snd_rawmidi_close(dev->handle);
				dev->handle = NULL;
			}
			continue;
		}

		dev->errors = 0;

		if (err <= 0)
			continue;

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
				if (g.verbose)
					MSG("%s: control change ch=%d p=%d v=%d", dev->name, channel, controller, value);
				g.nCCValues[controller] = value;
				i += 2;
			}
		}
	}

	return g.nCCValues[cc] / 127.0f;
}
