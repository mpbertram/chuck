// "use strict";

var LibraryWebAudio = {
    // Internal functions
    $WEBAUDIO: {
    },
    waudio_initialize: function () {
        console.log('Initializing Web Audio')
    }
}

autoAddDeps(LibraryWebAudio, '$WEBAUDIO')
mergeInto(LibraryManager.library, LibraryWebAudio)
