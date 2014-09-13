// "use strict";

var LibraryWebAudio = {
    // Internal functions
    $WEBAUDIO: {
        started: false,
    },
    waudio_initialize: function () {
        console.log('Initializing Web Audio')
        return true
    },
    waudio_start: function () {
        if (WEBAUDIO._started) {
            return
        }
        console.log('Starting Web Audio')
        WEBAUDIO._started = true
        return true
    },
    waudio_stop: function () {
        if (!WEBAUDIO._started) {
            return
        }
        console.log('Stopping Web Audio')
        WEBAUDIO._started = false
        return true
    },
    waudio_shutdown: function () {
        console.log('Shutting down Web Audio')
        return true
    },
    // Not implemented in Emscripten yet
    srandom: function (seed) {
    }
}

autoAddDeps(LibraryWebAudio, '$WEBAUDIO')
mergeInto(LibraryManager.library, LibraryWebAudio)
