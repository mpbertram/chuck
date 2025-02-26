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
// name: digiio_webaudio.h
// desc: digitalio over Web Audio API
//
// author: Arve Knudsen (arve.knudsen@gmail.com)
// date: 2014
//-----------------------------------------------------------------------------
#ifndef __DIGIIO_WEBAUDIO_H__
#define __DIGIIO_WEBAUDIO_H__

#include "chuck_def.h"
#include <string>

// defaults
#define BUFFER_SIZE_DEFAULT          4096
#define NUM_BUFFERS_DEFAULT          1
#define NUM_CHANNELS_DEFAULT         1       // number of channels
#define SAMPLING_RATE_DEFAULT        44100   // sampling rate
#define USE_CB_DEFAULT               TRUE    // callback
#define BITS_PER_SAMPLE_DEFAULT      32      // sample size
#define DEVICE_NUM_OUT_DEFAULT       0
#define DEVICE_NUM_IN_DEFAULT        0

// types
#define BOOL__                 DWORD__
#define DWORD__                unsigned long
#define SINT__                 long
#define UINT__                 DWORD__
#define BYTE__                 unsigned char

#ifndef FALSE
  #define FALSE                0
#endif
#ifndef TRUE
  #define TRUE                 1
#endif

struct Chuck_VM;

//-----------------------------------------------------------------------------
// name: Digitalio
// desc: ...
//-----------------------------------------------------------------------------
class Digitalio
{
public:
    static BOOL__ initialize( DWORD__ num_dac_channels,
                              DWORD__ num_adc_channels,
                              DWORD__ sampling_rate,
                              DWORD__ bps,
                              DWORD__ buffer_size,
                              DWORD__ num_buffers,
                              DWORD__ block,
                              Chuck_VM * vm_ref,
                              BOOL__ enable_audio,
                              void * callback,
                              void * data,
                              // force_srate added 1.3.1.2
                              BOOL__ force_srate );
    static void shutdown();
    static void probe();

    static DWORD__ device_named( const std::string & name, t_CKBOOL needs_dac = FALSE, t_CKBOOL needs_adc = FALSE );

public:
    static DWORD__ sampling_rate( ) { return m_sampling_rate; }
    static DWORD__ num_channels_out( ) { return m_num_channels_out; }
    static DWORD__ num_channels_in( ) { return m_num_channels_in; }
    static DWORD__ dac_num( ) { return m_dac_n; }
    static DWORD__ adc_num( ) { return m_adc_n; }
    static DWORD__ bps( ) { return m_bps; }
    static DWORD__ buffer_size( ) { return m_buffer_size; }
    static DWORD__ num_buffers( ) { return m_num_buffers; }
    static BOOL__  start( );
    static BOOL__  stop( );
    //static BOOL__  tick( );
    /*
    static void    set_extern( SAMPLE * in, SAMPLE * out )
                   { m_extern_in = in; m_extern_out = out; }
    static int cb( void *output_buffer, void *input_buffer,
                   unsigned int buffer_size,
                   double streamTime,
                   RtAudioStreamStatus status,
                   void *user_data );
    static int cb2( void *output_buffer, void *input_buffer,
                    unsigned int buffer_size,
                    double streamTime,
                    RtAudioStreamStatus status,
                    void *user_data );
*/

public: // data
    static BOOL__ m_init;
    static DWORD__ m_start;
    static DWORD__ m_tick_count;
    static DWORD__ m_num_channels_out;
    static DWORD__ m_num_channels_in;
    static DWORD__ m_sampling_rate;
    static DWORD__ m_bps;
    static DWORD__ m_buffer_size;
    static DWORD__ m_num_buffers;
    static float* m_samples_left;
    static float* m_samples_right;
    static t_CKINT m_out_pos;
    static BOOL__ m_out_ready;
    static BOOL__ m_in_ready;
    static BOOL__ m_use_cb;
    static DWORD__ m_go;
    static DWORD__ m_end;
    static DWORD__ m_block;
    static DWORD__ m_xrun;
    static SAMPLE* m_buffer_out;
    static SAMPLE* m_buffer_in;
    static SAMPLE** m_write_ptr;
    static SAMPLE** m_read_ptr;
    static Chuck_VM* m_vm;

    static DWORD__ m_dac_n;
    static DWORD__ m_adc_n;
};

//-----------------------------------------------------------------------------
// name: TickOut
// desc: ...
//-----------------------------------------------------------------------------
class TickOut
{
public:
    virtual ~TickOut() {}

public:
    virtual BOOL__ reset() { return true; }
    virtual BOOL__ tick_out( SAMPLE s ) = 0;
    virtual BOOL__ tick_out( SAMPLE l, SAMPLE r ) = 0;
    virtual BOOL__ tick_out( const SAMPLE * out, DWORD__ n ) = 0;
};

//-----------------------------------------------------------------------------
// name: TickIn
// desc: ...
//-----------------------------------------------------------------------------
class TickIn
{
public:
    virtual ~TickIn() { }

public:
    virtual BOOL__ reset() { return true; }

    virtual BOOL__ tick_in( SAMPLE * in ) = 0;
    virtual BOOL__ tick_in( SAMPLE * l, SAMPLE * r ) = 0;
    virtual BOOL__ tick_in( SAMPLE * in, DWORD__ n ) = 0;

    virtual SAMPLE tick( )
    { SAMPLE in; return ( tick_in( &in ) ? in : (SAMPLE)0.0f ); }
};

//-----------------------------------------------------------------------------
// name: class DigitalOut
// desc: ...
//-----------------------------------------------------------------------------
class DigitalOut : public TickOut
{
public:
    DigitalOut();
    ~DigitalOut();

public:
    BOOL__ initialize();
    void cleanup();

    BOOL__ start();
    BOOL__ stop();

public:
    virtual BOOL__ tick_out( SAMPLE s );
    virtual BOOL__ tick_out( SAMPLE l, SAMPLE r );
    virtual BOOL__ tick_out( const SAMPLE * samples, DWORD__ n );

public:
    DWORD__ render();

protected:
    SAMPLE * m_data_ptr_out;
    SAMPLE * m_data_max_out;

    inline BOOL__ prepare_tick_out();
};

//-----------------------------------------------------------------------------
// name: class DigitalIn
// desc: ...
//-----------------------------------------------------------------------------
class DigitalIn : public TickIn
{
public:
    DigitalIn();
    ~DigitalIn();

public: // in
    BOOL__ initialize();
    void cleanup();

    BOOL__ start();
    BOOL__ stop();

public:
    virtual BOOL__ tick_in( SAMPLE * s );
    virtual BOOL__ tick_in( SAMPLE * l, SAMPLE * r );
    virtual BOOL__ tick_in( SAMPLE * samples, DWORD__ n );

public:
    DWORD__ capture( );

protected:
    SAMPLE * m_data;
    SAMPLE * m_data_ptr_in;
    SAMPLE * m_data_max_in;

    inline BOOL__ prepare_tick_in();
};

//-----------------------------------------------------------------------------
// name: AudioBufferX
// desc: ...
//-----------------------------------------------------------------------------
class AudioBufferX
{
public:
    AudioBufferX( DWORD__ size = 0 );

public:
    BOOL__ initialize( DWORD__ size );
    void cleanup();
    SAMPLE * data();
    DWORD__ size();

protected:
    SAMPLE * m_data;
    DWORD__ m_size;

    SAMPLE * m_ptr_curr;
    SAMPLE * m_ptr_end;
};

//-----------------------------------------------------------------------------
// name: AudioBufferIn
// desc: ...
//-----------------------------------------------------------------------------
class AudioBufferIn : public TickIn, public AudioBufferX
{
public:
    AudioBufferIn( DWORD__ size = 0 );
    virtual ~AudioBufferIn();

public:
    virtual BOOL__ reset();
    virtual BOOL__ tick_in( SAMPLE * in );
    virtual BOOL__ tick_in( SAMPLE * l, SAMPLE * r );
    virtual BOOL__ tick_in( SAMPLE * in, DWORD__ n );
};

//-----------------------------------------------------------------------------
// name: AudioBufferOut
// desc: ...
//-----------------------------------------------------------------------------
class AudioBufferOut : public TickOut, public AudioBufferX
{
public:
    AudioBufferOut( DWORD__ size = 0 );
    virtual ~AudioBufferOut();

public:
    virtual BOOL__ reset();
    virtual BOOL__ tick_out( SAMPLE s );
    virtual BOOL__ tick_out( SAMPLE l, SAMPLE r );
    virtual BOOL__ tick_out( const SAMPLE * out, DWORD__ n );
};

#endif
