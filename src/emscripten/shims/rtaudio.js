// "use strict";

var LibraryRtAudio = {
    $RTAUDIO: {
        rtaudio_initialize: function (numChannels) {
            console.log('Initializing RtAudio with ' + numChannels + ' channels')
        }
    }
};

autoAddDeps(LibraryRtAudio, '$RTAUDIO')
mergeInto(LibraryManager.library, LibraryRtAudio)
