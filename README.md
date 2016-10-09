# Bonzomatic

[![Build status](https://ci.appveyor.com/api/projects/status/ix6fwi6nym1tu4e7?svg=true)](https://ci.appveyor.com/project/Gargaj/bonzomatic)
[![Build Status](https://travis-ci.org/Gargaj/Bonzomatic.svg)](https://travis-ci.org/Gargaj/Bonzomatic)
[![Beerpay](https://beerpay.io/Gargaj/Bonzomatic/badge.svg)](https://beerpay.io/Gargaj/Bonzomatic)

## What's this?
This is a live-coding tool, where you can write a 2D fragment/pixel shader while it is running in the background.

![Screenshot](http://i.imgur.com/8K8IztLl.jpg)

The tool was originally conceived and implemented after the Revision 2014 demoscene party's live coding competition where two contestants improv-code an effect in 25 minutes head-to-head: https://www.youtube.com/watch?v=-5P9rRUXaR0

## Keys
- F2: toggle texture preview
- F5 or Ctrl-R: recompile shader
- F11: hide shader overlay
- Alt-F4: exbobolate your planet

## Configuration
Create a ```config.json``` with e.g. the following contents: (all fields are optional)
``` javascript
{
  "font":{
    "file":"Input-Regular_(InputMono-Medium).ttf",
    "size":16,
  },
  "rendering":{
    "fftSmoothFactor": 0.9, // 0.0 means there's no smoothing at all, 1.0 means the FFT is completely smoothed flat
  },
  "textures":{ /* the keys below will become the shader variable names */
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
  },
  "midi":{ /* the keys below will become the shader variable names, the values are the CC numbers */
    "fMidiKnob": 16, /* e.g. this would be CC#16, i.e. by default the leftmost knob on a nanoKONTROL 2 */
  }
}
```

## Building
As you can see you're gonna need [CMAKE](https://cmake.org/) for this, but don't worry, a lot of it is automated at this point.
* On Windows, use at least Visual C++ 2010, or 2013 if your 2010 install doesn't have MFC. For the DX9/DX11 builds, obviously you'll be needing a DirectX SDK, though a lot of it is already in the Windows 8.1 SDK as well.
* On Linux, you'll need ```xorg-dev```; after that ```cmake``` should take care of the rest.
* On OSX, ```cmake``` should take care of everything.

## Credits and acknowledgements
### Original / parent project authors
- "ScintillaGL" project by Mykhailo Parfeniuk (https://github.com/sopyer/ScintillaGL)
- Riverwash LiveCoding Tool by Michał Staniszewski and Michal Szymczyk (http://www.plastic-demo.org/)

### Libraries and other included software
- Scintilla editing component by the Scintilla Dev Team (http://www.scintilla.org/)
- OpenGL Extension Wrangler Library by Nigel Stewart (http://glew.sourceforge.net/)
- BASS.DLL by Ian Luck (http://www.un4seen.com/)
- STB Image and Truetype libraries by Sean Barrett (http://nothings.org/)
- GLFW by whoever made GLFW (http://www.glfw.org/faq.html)
- JSON++ by Hong Jiang (https://github.com/hjiang/jsonxx)
 
These software are available under their respective licenses.

The remainder of this project code was (mostly, I guess) written by Gargaj / Conspiracy and is public domain.

## Contact / discussion forum
If you have anything to say, do it at http://www.pouet.net/topic.php?which=9881 or [![Join the chat at https://gitter.im/Gargaj/Bonzomatic](https://badges.gitter.im/Gargaj/Bonzomatic.svg)](https://gitter.im/Gargaj/Bonzomatic?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
