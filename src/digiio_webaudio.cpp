/*----------------------------------------------------------------------------
  ChucK Concurrent, On-the-fly Audio Programming Language
    Compiler and Virtual Machine

  Copyright (c) 2004 Ge Wang and Perry R. Cook.  All rights reserved.
    http://chuck.stanford.edu/
    http://chuck.cs.princeton.edu/

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  U.S.A.
-----------------------------------------------------------------------------*/

//-----------------------------------------------------------------------------
// name: digiio_webaudio.cpp
// desc: digitalio over Web Audio API
//
// author: Arve Knudsen (arve.knudsen@gmail.com)
// date: 2014
//-----------------------------------------------------------------------------
#include "digiio_webaudio.h"
#include "chuck_vm.h"
#include "chuck_errmsg.h"
#include "chuck_globals.h"
#include <limits.h>

extern "C" {
    BOOL__ waudio_initialize(void (*callback)(float*, float*, float*, float*, unsigned
        long));
    BOOL__ waudio_start();
    BOOL__ waudio_stop();
    BOOL__ waudio_shutdown();
}

namespace {
/** Callback to Web Audio library.
*/
extern "C" void callback(float* input_left, float* input_right, float* output_left,
    float* output_right, unsigned long buffer_size)
{
    // EM_log(CK_LOG_FINE, "Audio callback invoked for %d samples", buffer_size);
    Digitalio::m_samples_left = output_left;
    Digitalio::m_samples_right = output_right;
    Digitalio::m_out_pos = 0;
    Digitalio::m_vm->run(buffer_size);
}
}

// static
BOOL__ Digitalio::m_init = FALSE;
DWORD__ Digitalio::m_start = 0;
DWORD__ Digitalio::m_tick_count = 0;
DWORD__ Digitalio::m_num_channels_out = NUM_CHANNELS_DEFAULT;
DWORD__ Digitalio::m_num_channels_in = NUM_CHANNELS_DEFAULT;
DWORD__ Digitalio::m_sampling_rate = SAMPLING_RATE_DEFAULT;
DWORD__ Digitalio::m_bps = BITS_PER_SAMPLE_DEFAULT;
DWORD__ Digitalio::m_buffer_size = BUFFER_SIZE_DEFAULT;
DWORD__ Digitalio::m_num_buffers = NUM_BUFFERS_DEFAULT;
float* Digitalio::m_samples_left = NULL;
float* Digitalio::m_samples_right = NULL;
BOOL__ Digitalio::m_out_ready = FALSE;
BOOL__ Digitalio::m_in_ready = FALSE;
BOOL__ Digitalio::m_use_cb = USE_CB_DEFAULT;
DWORD__ Digitalio::m_go = 0;
DWORD__ Digitalio::m_dac_n = 0;
DWORD__ Digitalio::m_adc_n = 0;
DWORD__ Digitalio::m_end = 0;
DWORD__ Digitalio::m_block = TRUE;
DWORD__ Digitalio::m_xrun = 0;
Chuck_VM* Digitalio::m_vm = NULL;

//-----------------------------------------------------------------------------
// name: probe()
// desc: ...
//-----------------------------------------------------------------------------
void Digitalio::probe()
{
}

//-----------------------------------------------------------------------------
// name: device_named()
// desc: ...
//-----------------------------------------------------------------------------
DWORD__ Digitalio::device_named( const std::string & name, t_CKBOOL needs_dac, t_CKBOOL needs_adc )
{
    return -1;
}

//-----------------------------------------------------------------------------
// name: initialize()
// desc: ...
//-----------------------------------------------------------------------------
BOOL__ Digitalio::initialize( DWORD__ num_dac_channels,
                              DWORD__ num_adc_channels,
                              DWORD__ sampling_rate,
                              DWORD__ bps, DWORD__ buffer_size,
                              DWORD__ num_buffers, DWORD__ block,
                              Chuck_VM * vm_ref, BOOL__ enable_audio,
                              void*, void*, BOOL__ force_srate )
{
    Digitalio::m_vm = vm_ref;
    Digitalio::m_num_channels_out = num_dac_channels;
    return waudio_initialize(&callback);
}

//-----------------------------------------------------------------------------
// name: start()
// desc: ...
//-----------------------------------------------------------------------------
BOOL__ Digitalio::start( )
{
    return waudio_start();
}

//-----------------------------------------------------------------------------
// name: stop()
// desc: ...
//-----------------------------------------------------------------------------
BOOL__ Digitalio::stop( )
{
    return waudio_stop();
}

//-----------------------------------------------------------------------------
// name: shutdown()
// desc: ...
//-----------------------------------------------------------------------------
void Digitalio::shutdown()
{
    waudio_shutdown();
}

//-----------------------------------------------------------------------------
// name: DigitalOut()
// desc: ...
//-----------------------------------------------------------------------------
DigitalOut::DigitalOut()
{
    m_data_ptr_out = NULL;
    m_data_max_out = NULL;
}

//-----------------------------------------------------------------------------
// name: ~DigitalOut()
// desc: ...
//-----------------------------------------------------------------------------
DigitalOut::~DigitalOut()
{
    this->cleanup();
}

//-----------------------------------------------------------------------------
// name: initialize()
// desc: initialize audio out
//-----------------------------------------------------------------------------
BOOL__ DigitalOut::initialize( )
{
    return TRUE;
}

//-----------------------------------------------------------------------------
// name: start()
// desc: render
//-----------------------------------------------------------------------------
BOOL__ DigitalOut::start()
{
    // start stream
    return ( Digitalio::start() != 0 );
}

//-----------------------------------------------------------------------------
// name: stop()
// desc: render
//-----------------------------------------------------------------------------
BOOL__ DigitalOut::stop()
{
    // well
    Digitalio::stop();

    return TRUE;
}

//-----------------------------------------------------------------------------
// name: cleanup()
// desc: ...
//-----------------------------------------------------------------------------
void DigitalOut::cleanup()
{
}

//-----------------------------------------------------------------------------
// name: tick_out()
// desc: 1 channel
//-----------------------------------------------------------------------------
BOOL__ DigitalOut::tick_out( SAMPLE sample )
{
    return TRUE;
}

//-----------------------------------------------------------------------------
// name: tick_out()
// desc: 2 channel
//-----------------------------------------------------------------------------
BOOL__ DigitalOut::tick_out( SAMPLE sample_l, SAMPLE sample_r )
{
    return TRUE;
}

//-----------------------------------------------------------------------------
// name: tick_out()
// desc: all channels
//-----------------------------------------------------------------------------
BOOL__ DigitalOut::tick_out( const SAMPLE * samples, DWORD__ n )
{
    Digitalio::m_samples_left[Digitalio::m_out_pos] = samples[0];
    Digitalio::m_samples_right[Digitalio::m_out_pos] = samples[1];
    ++Digitalio::m_out_pos;
    return TRUE;
}

//-----------------------------------------------------------------------------
// name: render()
// desc: the render
//-----------------------------------------------------------------------------
DWORD__ DigitalOut::render()
{
    return TRUE;
}

//-----------------------------------------------------------------------------
// name: DigitalIn()
// desc: ...
//-----------------------------------------------------------------------------
DigitalIn::DigitalIn()
{
    m_data_ptr_in = NULL;
    m_data_max_in = NULL;
}

//-----------------------------------------------------------------------------
// name: ~DigitalIn()
// desc: ...
//-----------------------------------------------------------------------------
DigitalIn::~DigitalIn()
{
    this->cleanup();
}

//-----------------------------------------------------------------------------
// name: initialize()
// desc: initialize audio in
//-----------------------------------------------------------------------------
BOOL__ DigitalIn::initialize( )
{
    return TRUE;
}

//-----------------------------------------------------------------------------
// name: start()
// desc: ...
//-----------------------------------------------------------------------------
BOOL__ DigitalIn::start()
{
    return ( Digitalio::start() != 0 );
}

//-----------------------------------------------------------------------------
// name: capture_stop()
// desc: ...
//-----------------------------------------------------------------------------
BOOL__ DigitalIn::stop()
{
    Digitalio::stop();
    return TRUE;
}

//-----------------------------------------------------------------------------
// name: cleanup()
// desc: ...
//-----------------------------------------------------------------------------
void DigitalIn::cleanup()
{
}

//-----------------------------------------------------------------------------
// name: tick_in()
// desc: ...
//-----------------------------------------------------------------------------
BOOL__ DigitalIn::tick_in( SAMPLE * s )
{
    return TRUE;
}

//-----------------------------------------------------------------------------
// name: tick_in()
// desc: ...
//-----------------------------------------------------------------------------
BOOL__ DigitalIn::tick_in( SAMPLE * l, SAMPLE * r )
{
    return TRUE;
}

//-----------------------------------------------------------------------------
// name: tick_in()
// desc: ...
//-----------------------------------------------------------------------------
BOOL__ DigitalIn::tick_in( SAMPLE * sample, DWORD__ n )
{
    return TRUE;
}

//-----------------------------------------------------------------------------
// name: prepare_tick_in()
// desc: data ptr ok
//-----------------------------------------------------------------------------
inline BOOL__ DigitalIn::prepare_tick_in()
{
    return TRUE;
}

//-----------------------------------------------------------------------------
// name: cb_capture()
// desc: ...
//-----------------------------------------------------------------------------
DWORD__ DigitalIn::capture( )
{
    return TRUE;
}

//-----------------------------------------------------------------------------
// name: AudioBufferX()
// desc: ...
//-----------------------------------------------------------------------------
AudioBufferX::AudioBufferX( DWORD__ size )
{
}

//-----------------------------------------------------------------------------
// name: initialize()
// desc: ...
//-----------------------------------------------------------------------------
BOOL__ AudioBufferX::initialize( DWORD__ size )
{
    return TRUE;
}

//-----------------------------------------------------------------------------
// name: cleanup()
// desc: ...
//-----------------------------------------------------------------------------
void AudioBufferX::cleanup()
{
}

//-----------------------------------------------------------------------------
// name: size()
// desc: ...
//-----------------------------------------------------------------------------
DWORD__ AudioBufferX::size()
{
    return m_size;
}

//-----------------------------------------------------------------------------
// name: data()
// desc: ...
//-----------------------------------------------------------------------------
SAMPLE * AudioBufferX::data()
{
    return m_data;
}

//-----------------------------------------------------------------------------
// name: AudioBufferIn()
// desc: ...
//-----------------------------------------------------------------------------
AudioBufferIn::AudioBufferIn( DWORD__ size )
    : AudioBufferX( size )
{ }

//-----------------------------------------------------------------------------
// name: ~AudioBufferIn()
// desc: ...
//-----------------------------------------------------------------------------
AudioBufferIn::~AudioBufferIn()
{
    this->cleanup();
}

//-----------------------------------------------------------------------------
// name: reset()
// desc: ...
//-----------------------------------------------------------------------------
BOOL__ AudioBufferIn::reset()
{
    return TickIn::reset();
}

//-----------------------------------------------------------------------------
// name: tick_in()
// desc: ...
//-----------------------------------------------------------------------------
BOOL__ AudioBufferIn::tick_in( SAMPLE * s )
{
    return TRUE;
}

//-----------------------------------------------------------------------------
// name: tick_in()
// desc: ...
//-----------------------------------------------------------------------------
BOOL__ AudioBufferIn::tick_in( SAMPLE * l, SAMPLE * r )
{
    return TRUE;
}

//-----------------------------------------------------------------------------
// name: tick_in()
// desc: ...
//-----------------------------------------------------------------------------
BOOL__ AudioBufferIn::tick_in( SAMPLE * in, DWORD__ n )
{
    return TRUE;
}

//-----------------------------------------------------------------------------
// name: AudioBufferOut()
// desc: ...
//-----------------------------------------------------------------------------
AudioBufferOut::AudioBufferOut( DWORD__ size )
    : AudioBufferX( size )
{ }

//-----------------------------------------------------------------------------
// name: ~AudioBufferOut()
// desc: ...
//-----------------------------------------------------------------------------
AudioBufferOut::~AudioBufferOut()
{
    this->cleanup();
}

//-----------------------------------------------------------------------------
// name: reset()
// desc: ...
//-----------------------------------------------------------------------------
BOOL__ AudioBufferOut::reset()
{
    return TickOut::reset();
}

//-----------------------------------------------------------------------------
// name: tick_out()
// desc: ...
//-----------------------------------------------------------------------------
BOOL__ AudioBufferOut::tick_out( SAMPLE s )
{
    return TRUE;
}

//-----------------------------------------------------------------------------
// name: tick_out()
// desc: ...
//-----------------------------------------------------------------------------
BOOL__ AudioBufferOut::tick_out( SAMPLE l, SAMPLE r )
{
    return TRUE;
}

//-----------------------------------------------------------------------------
// name: tick_out()
// desc: ...
//-----------------------------------------------------------------------------
BOOL__ AudioBufferOut::tick_out( const SAMPLE * out, DWORD__ n )
{
    return TRUE;
}
