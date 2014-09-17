// "use strict";

var LibraryWebAudio = {
  // Internal functions
  $WEBAUDIO: {
    started: false,
    stop: function () {
      if (!WEBAUDIO.started) {
          return false
      }
      console.log('Stopping Web Audio')
      WEBAUDIO.scriptProcessor.disconnect(0)
      WEBAUDIO.started = false
      return true
    }
  },
  waudio_initialize: function (callback) {
    callback = Runtime.getFuncWrapper(callback, 'viiiii')
    console.log('Initializing Web Audio, callback: ' + callback)
    if (!callback) {
      throw new Error('callback must be provided')
    }
    WEBAUDIO.audioContext = new AudioContext()
    var bufLen = 4096
    WEBAUDIO.scriptProcessor = WEBAUDIO.audioContext.createScriptProcessor(bufLen, 0, 2)
    WEBAUDIO.scriptProcessor.onaudioprocess = function (event) {
      var samplesLeft = event.outputBuffer.getChannelData(0)
      var samplesRight = event.outputBuffer.getChannelData(1)
      var numBytes = bufLen * Float32Array.BYTES_PER_ELEMENT
      // Allocate buffers for the callback to fill
      var pointerLeft = Module._malloc(numBytes)
      var pointerRight = Module._malloc(numBytes)

      try {
        try {
          callback(0, 0, pointerLeft, pointerRight, bufLen)
        }
        catch (err) {
          console.error('Exception caught in audio callback: ' + err)
          // TODO: Terminate processing
        }

        var i, shiftage = Float32Array.BYTES_PER_ELEMENT / 2
        for (i = 0; i < bufLen; ++i) {
          samplesLeft[i] = HEAPF32[(pointerLeft >> shiftage) + i]
          samplesRight[i] = HEAPF32[(pointerRight >> shiftage) + i]
        }
      }
      finally {
        Module._free(pointerLeft)
        Module._free(pointerRight)
      }
    }

    return true
  },
  waudio_start: function () {
    if (WEBAUDIO.started) {
        return
    }
    console.log('Starting Web Audio')
    WEBAUDIO.scriptProcessor.connect(WEBAUDIO.audioContext.destination)
    WEBAUDIO.started = true
    return true
  },
  waudio_stop: function () {
    return WEBAUDIO.stop()
  },
  waudio_shutdown: function () {
    if (WEBAUDIO.started) {
      WEBAUDIO.stop();
    }
    else {
      throw new Error('Not started')
    }

    console.log('Shutting down Web Audio')
    WEBAUDIO.audioContext = null
    return true
  },
  // Not implemented in Emscripten yet
  srandom: function (seed) {
  }
}

autoAddDeps(LibraryWebAudio, '$WEBAUDIO')
mergeInto(LibraryManager.library, LibraryWebAudio)
