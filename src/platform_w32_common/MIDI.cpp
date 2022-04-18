#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#include <tchar.h>

namespace MIDI
{
  int nCCValues[256];
  void CALLBACK MyMidiInProc( HMIDIIN hMidiIn, UINT wMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2) 
  {
    switch (wMsg) {
    case MM_MIM_OPEN:
      {
        // unused
      } break;
    case MM_MIM_CLOSE:
      {
        // unused
      } break;
    case MM_MIM_DATA:
      {
        unsigned char nMIDIMessage = dwParam1 & 0xFF;
        unsigned char nMIDIMessageType = nMIDIMessage >> 4;
        unsigned char nMIDIChannel = nMIDIMessage & 0x0F;
        unsigned char nMIDIControl = (dwParam1 >> 8) & 0xFF;
        unsigned char nMIDIValue = (dwParam1 >> 16) & 0xFF;

        //printf("[%08X] MM_MIM_DATA - TYPE: %X CHAN: %3d CTRL: %3d VAL: %3d\n",hMidiIn,nMIDIMessageType,nMIDIChannel,nMIDIControl,nMIDIValue);
        nCCValues[ nMIDIControl ] = nMIDIValue;
      } break;
    }
  }

  HMIDIIN hMIDIInput[32];
  int nMIDIDeviceCount = 0;
  bool Open()
  {
    ZeroMemory( &nCCValues, sizeof(int) * 256 );
    nMIDIDeviceCount = midiInGetNumDevs();

    for (int i = 0; i < nMIDIDeviceCount; i++) 
    {
      MIDIINCAPS caps;
      midiInGetDevCaps(i,&caps,sizeof(MIDIINCAPS));
      //_tprintf(_T("%d - %d - %d - %08X - %s\n"),i,caps.wMid,caps.wPid,caps.vDriverVersion,caps.szPname);

      midiInOpen(&hMIDIInput[i],i,(DWORD_PTR)MyMidiInProc,NULL,CALLBACK_FUNCTION);

      midiInStart(hMIDIInput[i]);
    }

    return true;
  }
  bool Close()
  {
    for (int i = 0; i < nMIDIDeviceCount; i++) {
      midiInStop(hMIDIInput[i]);
      midiInClose(hMIDIInput[i]);
    }
    return true;
  }

  float GetCCValue( unsigned char cc )
  {
    return nCCValues[cc] / 127.0f;
  }

};