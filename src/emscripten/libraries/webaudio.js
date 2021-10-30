// "use strict";

var LibraryWebAudio = {
  waudio_initialize__deps: ['$getFuncWrapper'],
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
    callback = getFuncWrapper(callback, 'viiiii')
    console.log('Initializing Web Audio')
    if (!callback) {
      throw new Error('callback must be provided')
    }
    WEBAUDIO.audioContext = new (window.AudioContext || window.webkitAudioContext)()
    var bufLen = 4096
    WEBAUDIO.scriptProcessor = WEBAUDIO.audioContext.createScriptProcessor(bufLen, 0, 1)
    WEBAUDIO.scriptProcessor.onaudioprocess = function (event) {
      var samples = event.outputBuffer.getChannelData(0)
      var buf = Module._malloc(bufLen * Float32Array.BYTES_PER_ELEMENT)

      try {
        try {
          callback(0, buf, bufLen)
        }
        catch (err) {
          console.error('Exception caught in audio callback: ' + err)
          // TODO: Terminate processing
        }

        for (i = 0; i < bufLen; ++i) {
          var f = buf + (i * Float32Array.BYTES_PER_ELEMENT)
          samples[i] = getValue(f, 'float')
        }
      }
      finally {
        Module._free(buf)
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
    _srand(seed)
  },
  random: function () {
    return _rand()
  },
}

autoAddDeps(LibraryWebAudio, '$WEBAUDIO')
mergeInto(LibraryManager.library, LibraryWebAudio)
