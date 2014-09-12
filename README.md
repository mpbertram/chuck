# ChucK [![Build Status](https://travis-ci.org/ccrma/chuck.svg?branch=master)](https://travis-ci.org/ccrma/chuck)
## Strongly-timed, Concurrent, and On-the-fly Music Programming Language  

Welcome to ChucK!

what is it? : ChucK is a programming language for real-time sound synthesis and music creation. It is open-source and freely available on MacOS X, Windows, and Linux. ChucK presents a unique time-based, concurrent programming model that's precise and expressive (we call this strongly-timed), dynamic control rates, and the ability to add and modify code on-the-fly. In addition, ChucK supports MIDI, OpenSoundControl, HID device, and multi-channel audio. It's fun and easy to learn, and offers composers, researchers, and performers a powerful programming tool for building and experimenting with complex audio synthesis/analysis programs, and real-time interactive music.

For more information, including documentation, research publications, and community resources, please check out the ChucK website:
http://chuck.stanford.edu/

## BUILDING
### JavaScript
To build the JavaScript port of ChucK, first source the [Emscripten](http://emscripten.org/) SDK,
then enter the src directory and run `make emscripten`.

This will generate a JavaScript library with the help of Emscripten, chuck.js. You can test chuck.js
by opening the HTML document emscripten/chuck.html. For now, chuck.html does nothing, but you can look
for errors in the JavaScript console of your browser.
