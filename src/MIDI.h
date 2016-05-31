namespace MIDI
{
  enum
  {
    MIDIMSG_NOTE_OFF         = 8,
    MIDIMSG_NOTE_ON          = 9,
    MIDIMSG_POLYPHONIC_KEY   = 10,
    MIDIMSG_CONTROL_CHANGE   = 11,
    MIDIMSG_PROGRAM_CHANGE   = 12,
    MIDIMSG_CHANNEL_PRESSURE = 13,
    MIDIMSG_PITCH_BEND       = 14,
    MIDIMSG_SYSTEM           = 15,
  } MIDI_MESSAGE_TYPE;

  bool Open();
  bool Close();
  float GetCCValue( unsigned char cc );
};