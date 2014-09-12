#include "RtAudio/RtAudio.h"

RtAudio m_rtaudio = null;

extern "C"
{
bool rtaudio_initialize(int num_channels)
{
    // allocate RtAudio
    try { m_rtaudio = new RtAudio( ); }
    catch( RtError err )
    {
        // problem finding audio devices, most likely
        EM_error2( 0, "%s", err.getMessage().c_str() );
        return false;
    }

    // convert 1-based ordinal to 0-based ordinal (added 1.3.0.0)
    // note: this is to preserve previous devices numbering after RtAudio change
    if( m_num_channels_out > 0 )
    {
        // check output device number; 0 used to mean "default"
        bool useDefault = ( m_dac_n == 0 );

        // default (refactor 1.3.1.2)
        if( useDefault )
        {
            // get the default
            m_dac_n = m_rtaudio->getDefaultOutputDevice();
        }
        else
        {
            m_dac_n -= 1;
        }

        // get device info
        RtAudio::DeviceInfo device_info = m_rtaudio->getDeviceInfo(m_dac_n);

        // ensure correct channel count if default device is requested
        if( useDefault )
        {
            // check
            if( device_info.outputChannels < m_num_channels_out )
            {
                // find first device with at least the requested channel count
                m_dac_n = -1;
                int num_devices = m_rtaudio->getDeviceCount();
                for( int i = 0; i < num_devices; i++ )
                {
                    device_info = m_rtaudio->getDeviceInfo(i);
                    if(device_info.outputChannels >= m_num_channels_out)
                    {
                        m_dac_n = i;
                        break;
                    }
                }

                // check for error
                if( m_dac_n == -1 )
                {
                    EM_error2( 0, "no audio output device with requested channel count (%i)...",
                        m_num_channels_out );
                    return m_init = FALSE;
                }
            }
        }

        // index of closest sample rate
        long closestIndex = -1;
        // difference of closest so far
        long closestDiff = LONG_MAX;
        // the next highest
        long nextHighest = -1;
        // diff to next highest so far
        long diffToNextHighest = LONG_MAX;
        // check if request sample rate in support rates (added 1.3.1.2)
        for( long i = 0; i < device_info.sampleRates.size(); i++ )
        {
            // difference
            long diff = device_info.sampleRates[i] - sampling_rate;
            // check
            if( ::abs(diff) < closestDiff )
            {
                // remember index
                closestIndex = i;
                // update diff
                closestDiff = ::abs(diff);
            }

            // for next highest
            if( diff > 0 && diff < diffToNextHighest )
            {
                // remember index
                nextHighest = i;
                // update diff
                diffToNextHighest = diff;
            }
        }

        // see if we found exact match (added 1.3.1.2)
        if( closestDiff != 0 )
        {
            // check
            if( force_srate )
            {
                // request sample rate not found, error out
                EM_error2( 0, "unsupported sample rate (%d) requested...", sampling_rate );
                EM_error2( 0, "| (try --probe to enumerate available sample rates)" );
                return m_init = FALSE;
            }

            // use next highest if available
            if( nextHighest >= 0 )
            {
                // log
                EM_log( CK_LOG_SEVERE, "new sample rate (next highest): %d -> %d",
                        sampling_rate, device_info.sampleRates[nextHighest] );
                // update sampling rate
                m_sampling_rate = sampling_rate = device_info.sampleRates[nextHighest];
            }
            else if( closestIndex >= 0 ) // nothing higher
            {
                // log
                EM_log( CK_LOG_SEVERE, "new sample rate (closest): %d -> %d",
                        sampling_rate, device_info.sampleRates[closestIndex] );
                // update sampling rate
                m_sampling_rate = sampling_rate = device_info.sampleRates[closestIndex];
            }
            else
            {
                // nothing to do (will fail and throw error message when opening)
            }
        }
    }

    // convert 1-based ordinal to 0-based ordinal
    if( m_num_channels_in > 0 )
    {
        if( m_adc_n == 0 )
        {
            m_adc_n = m_rtaudio->getDefaultInputDevice();

            // ensure correct channel count if default device is requested
            RtAudio::DeviceInfo device_info = m_rtaudio->getDeviceInfo(m_adc_n);

            // check if input channels > 0
            if( device_info.inputChannels < m_num_channels_in )
            {
                // find first device with at least the requested channel count
                m_adc_n = -1;
                int num_devices = m_rtaudio->getDeviceCount();
                for(int i = 0; i < num_devices; i++)
                {
                    device_info = m_rtaudio->getDeviceInfo(i);
                    if(device_info.inputChannels >= m_num_channels_in)
                    {
                        m_adc_n = i;
                        break;
                    }
                }

                // changed 1.3.1.2 (ge): for input, if nothing found, we just gonna try to open half-duplex
                if( m_adc_n == -1 )
                {
                    // set to 0
                    m_num_channels_in = 0;
                    // problem finding audio devices, most likely
                    // EM_error2( 0, "unable to find audio input device with requested channel count (%i)...", m_num_channels_in);
                    // return m_init = FALSE;
                }
            }
        }
        else
        {
            m_adc_n -= 1;
        }
    }

    // open device
    try {
        // log
        EM_log( CK_LOG_FINE, "trying %d input %d output...",
                m_num_channels_in, m_num_channels_out );

        RtAudio::StreamParameters output_parameters;
        output_parameters.deviceId = m_dac_n;
        output_parameters.nChannels = m_num_channels_out;
        output_parameters.firstChannel = 0;

        RtAudio::StreamParameters input_parameters;
        input_parameters.deviceId = m_adc_n;
        input_parameters.nChannels = m_num_channels_in;
        input_parameters.firstChannel = 0;

        RtAudio::StreamOptions stream_options;
        stream_options.flags = 0;
        stream_options.numberOfBuffers = num_buffers;
        stream_options.streamName = "ChucK";
        stream_options.priority = 0;

        // open RtAudio
        m_rtaudio->openStream(
            m_num_channels_out > 0 ? &output_parameters : NULL,
            m_num_channels_in > 0 ? &input_parameters : NULL,
            CK_RTAUDIO_FORMAT, sampling_rate, &bufsize,
            m_use_cb ? ( block ? &cb : &cb2 ) : NULL, vm_ref,
            &stream_options );
    } catch( RtError err ) {
        // log
        EM_log( CK_LOG_INFO, "exception caught: '%s'...", err.getMessage().c_str() );
        EM_error2( 0, "%s", err.getMessage().c_str() );
        SAFE_DELETE( m_rtaudio );
        return m_init = FALSE;
    }

    // check bufsize
    if( bufsize != (int)m_buffer_size )
    {
        EM_log( CK_LOG_SEVERE, "new buffer size: %d -> %i", m_buffer_size, bufsize );
        m_buffer_size = bufsize;
    }

    // pop indent
    EM_poplog();
}
}
