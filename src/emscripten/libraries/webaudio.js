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
    callback = Runtime.getFuncWrapper(callback, 'viii')
    console.log('Initializing Web Audio, callback: ' + callback)
    if (!callback) {
      throw new Error('callback must be provided')
    }
    WEBAUDIO.audioContext = new AudioContext()
    var bufLen = 4096
    WEBAUDIO.scriptProcessor = WEBAUDIO.audioContext.createScriptProcessor(bufLen, 0, 2)
    WEBAUDIO.scriptProcessor.onaudioprocess = function (event) {
      samplesLeft = event.outputBuffer.getChannelData(0)
      samplesRight = event.outputBuffer.getChannelData(1)

      try {
        callback([], [samplesLeft, samplesRight], bufLen)
      }
      catch (err) {
        console.error('Exception caught in audio callback: ' + err)
        // TODO: Terminate processing
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
