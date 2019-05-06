# Bonzomatic

[![Build status](https://ci.appveyor.com/api/projects/status/ix6fwi6nym1tu4e7?svg=true)](https://ci.appveyor.com/project/Gargaj/bonzomatic)
[![Build Status](https://travis-ci.org/Gargaj/Bonzomatic.svg)](https://travis-ci.org/Gargaj/Bonzomatic)

## What's this?
This is a live-coding tool, where you can write a 2D fragment/pixel shader while it is running in the background.

![Screenshot](https://i.imgur.com/8K8IztLl.jpg)

The tool was originally conceived and implemented after the Revision 2014 demoscene party's live coding competition where two contestants improv-code an effect in 25 minutes head-to-head. Wanna see how it looks in action? Check https://www.youtube.com/watch?v=KG_2q4OEhKc

## Keys
- F2: toggle texture preview
- F5 or Ctrl-R: recompile shader
- F11 or Ctrl/Cmd-f: hide shader overlay
- Alt-F4 or Shift+Escape: exbobolate your planet

## Requirements
On Windows, both DirectX 9 and 11 are supported.

For the OpenGL version (for any platform), at least OpenGL 4.1 is required.

On recent macOS, to allow sound input to be captured (for FFT textures to be generated), you need to: Open up System Preferences, click on Security & Privacy, click on the Privacy tab then click on the Microphone menu item. Make sure Bonzomatic.app is in the list and ticked.

## Configuration
Create a `config.json` with e.g. the following contents: (all fields are optional)
``` javascript
{
  "window":{ // default window size / state, if there's a setup dialog, it will override it
    "width":1920,
    "height":1080,
    "fullscreen":true,
  },
  "font":{
    "file":"Input-Regular_(InputMono-Medium).ttf",
    "size":16,
  },
  "rendering":{
    "fftSmoothFactor": 0.9, // 0.0 means there's no smoothing at all, 1.0 means the FFT is completely smoothed flat
    "fftAmplification": 1.0, // 1.0 means no change, larger values will result in brighter/stronger bands, smaller values in darker/weaker ones
  },
  "textures":{ // the keys below will become the shader variable names
    "texChecker":"textures/checker.png",
    "texNoise":"textures/noise.png",
    "texTex1":"textures/tex1.jpg",
  },
  "gui":{
    "outputHeight": 200,
    "opacity": 192, // 255 means the editor occludes the effect completely, 0 means the editor is fully transparent
    "texturePreviewWidth": 64,
    "spacesForTabs": false,
    "tabSize": 8,
    "visibleWhitespace": true,
    "autoIndent": "smart", // can be "none", "preserve" or "smart"
  },
  "midi":{ // the keys below will become the shader variable names, the values are the CC numbers
    "fMidiKnob": 16, // e.g. this would be CC#16, i.e. by default the leftmost knob on a nanoKONTROL 2
  },
  // this section is if you want to enable NDI streaming; otherwise just ignore it
  "ndi":{
    "enabled": true,
    "connectionString": "<ndi_product something=\"123\"/>", // metadata sent to the receiver; completely optional
    "identifier": "hello!", // additional string to the device name; helps source discovery/identification in the receiver if there are multiple sources on the network
    "frameRate": 60.0, // frames per second
    "progressive": true, // progressive or interleaved?
  },
  "postExitCmd":"copy_to_dropbox.bat" // this command gets ran when you quit Bonzomatic, and the shader filename gets passed to it as first parameter. Use this to take regular backups.
}
```
### Automatic shader backup
If you want the shader to be backed up once you quit Bonzomatic, you can use the above `postExitCmd` parameter in the config, and use a batch file like this:
```
@echo off
REM ### cf. https://stackoverflow.com/a/23476347
for /f "tokens=2 delims==" %%a in ('wmic OS Get localdatetime /value') do set "dt=%%a"
set "YY=%dt:~2,2%" & set "YYYY=%dt:~0,4%" & set "MM=%dt:~4,2%" & set "DD=%dt:~6,2%"
set "HH=%dt:~8,2%" & set "Min=%dt:~10,2%" & set "Sec=%dt:~12,2%"
copy %1 X:\MyShaderBackups\%YYYY%%MM%%DD%-%HH%%Min%%Sec%.glsl
```
This will copy the shader timestamped into a specified folder.

## Building
As you can see you're gonna need [CMAKE](https://cmake.org/) for this, but don't worry, a lot of it is automated at this point.

### Windows
Use at least Visual C++ 2010. For the DX9/DX11 builds, obviously you'll be needing a DirectX SDK, though a lot of it is already in the Windows 8.1 SDK as well.

### OSX/macOS
```cmake``` should take care of everything:
```
cmake .
make
make install
```

### Linux
You'll need ```xorg-dev``` and ```libglu1-mesa-dev```; after that ```cmake``` should take care of the rest:
```
apt install xorg-dev libglu1-mesa-dev cmake
cd Bonzomatic
cmake .
make
make install
```

### OpenBSD
[Xenocara](https://xenocara.org) contains all required components.  Hack away with
```
cmake .
make
```
or use the port
```
cd /usr/ports/graphics/bonzomatic
make install
```


## Organizing a competition
If you want to organize a competition using Bonzomatic at your party, here's a handy-dandy guide on how to get started:
https://github.com/Gargaj/Bonzomatic/wiki/How-to-set-up-a-Live-Coding-compo

## Credits and acknowledgements
### Original / parent project authors
- "ScintillaGL" project by Mykhailo Parfeniuk (https://github.com/sopyer/ScintillaGL)
- Riverwash LiveCoding Tool by Michał Staniszewski and Michal Szymczyk (http://www.plastic-demo.org)

### Libraries and other included software
- Scintilla editing component by the Scintilla Dev Team (https://www.scintilla.org)
- OpenGL Extension Wrangler Library by Nigel Stewart (http://glew.sourceforge.net)
- mini_al by David Reid (https://github.com/dr-soft/mini_al)
- KISSFFT by Mark Borgerding (https://github.com/mborgerding/kissfft)
- STB Image and Truetype libraries by Sean Barrett (https://nothings.org)
- GLFW by whoever made GLFW (https://www.glfw.org/faq.html)
- JSON++ by Hong Jiang (https://github.com/hjiang/jsonxx)
- NDI(tm) SDK by NewTek(tm) (https://www.newtek.com/ndi.html)

These software are available under their respective licenses.

The remainder of this project code was (mostly, I guess) written by Gargaj / Conspiracy and is public domain.
OSX / macOS maintenance and ports by Alkama / Tpolm + Calodox; Linux maintenance by PoroCYon / K2.

## Contact / discussion forum
If you have anything to say, do it at https://www.pouet.net/topic.php?which=9881 or [![Join the chat at https://gitter.im/Gargaj/Bonzomatic](https://badges.gitter.im/Gargaj/Bonzomatic.svg)](https://gitter.im/Gargaj/Bonzomatic?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
