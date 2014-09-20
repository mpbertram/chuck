/*----------------------------------------------------------------------------
  ChucK Concurrent, On-the-fly Audio Programming Language
    Compiler and Virtual Machine

  Copyright (c) 2003 Ge Wang and Perry R. Cook.  All rights reserved.
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
// file: chuck_js.cpp
// desc: chuck JavaScript logic
//
// author: Arve Knudsen (arve.knudsen@gmail.com)
//-----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>

#include "chuck_compile.h"
#include "chuck_vm.h"
#include "chuck_bbq.h"
#include "chuck_lang.h"
#include "chuck_globals.h"

#include "util_math.h"
#include "util_string.h"
#include "hidio_sdl.h"

using namespace std;

namespace {
void load_chugins(Chuck_Compiler *compiler, Chuck_VM *vm)
{
    Chuck_VM_Shred *shred;
    // log
    EM_log(CK_LOG_SEVERE, "pre-loading ChucK libs...");
    EM_pushlog();

    // iterate over list of ck files that the compiler found
    for (std::list<std::string>::iterator j = compiler->m_cklibs_to_preload.begin();
         j != compiler->m_cklibs_to_preload.end(); j++)
    {
        // the filename
        std::string filename = *j;

        // log
        EM_log(CK_LOG_SEVERE, "preloading '%s'...", filename.c_str());
        // push indent
        EM_pushlog();

        // SPENCERTODO: what to do for full path
        std::string full_path = filename;

        // parse, type-check, and emit
        if (compiler->go(filename, NULL, NULL, full_path))
        {
            // TODO: how to compilation handle?
            //return 1;

            // get the code
            Chuck_VM_Code* code = compiler->output();
            // name it - TODO?
            // code->name += string(argv[i]);

            // spork it
            shred = vm->spork(code, NULL);
        }

        // pop indent
        EM_poplog();
    }

    // clear the list of chuck files to preload
    compiler->m_cklibs_to_preload.clear();

    // pop log
    EM_poplog();
}

t_CKBOOL sporkCode(Chuck_Compiler* compiler, Chuck_VM* vm, const char* filename, const char* sourceCode)
{
    compiler->env->load_user_namespace();

    // log
    EM_log(CK_LOG_FINE, "compiling code...");
    // push indent
    EM_pushlog();

    // parse, type-check, and emit (full_path added 1.3.0.0)
    if (!compiler->go(filename, NULL, sourceCode, NULL))
    {
        EM_log(CK_LOG_SYSTEM, "compilation failed");
        return FALSE;
    }

    EM_log(CK_LOG_FINE, "compiled successfully");
    // get the code
    Chuck_VM_Code* code = compiler->output();
    // name it
    code->name += string(filename);

    // spork
    vm->spork(code, NULL);
    // add args
    // (*shred)->args = args;

    // pop indent
    EM_poplog();

    // reset the parser
    reset_parse();

    return TRUE;
}
}

extern "C" {
/** Execute ChucK code.

The purpose of this function is to play back a piece of code, starting
the audio engine before execution starts and shutting it down after.
*/
void executeCode(const char* fileName, const char* code)
{
    Chuck_Compiler *compiler = NULL;
    Chuck_VM *vm = NULL;

    t_CKBOOL vm_halt = TRUE;
    t_CKUINT srate = SAMPLING_RATE_DEFAULT;
    t_CKBOOL force_srate = FALSE; // added 1.3.1.2
    t_CKUINT buffer_size = BUFFER_SIZE_DEFAULT;
    t_CKUINT num_buffers = NUM_BUFFERS_DEFAULT;
    t_CKUINT dac = 0;
    t_CKUINT adc = 0;
    t_CKUINT dac_chans = 2;
    t_CKUINT adc_chans = 2;
    t_CKBOOL dump = FALSE;
    t_CKBOOL auto_depend = FALSE;
    t_CKBOOL block = FALSE;
    t_CKINT adaptive_size = 0;
    t_CKINT log_level = CK_LOG_FINE;
    t_CKINT deprecate_level = 1; // 1 == warn
    string filename = "";
    vector <string> args;

    // list of search pathes (added 1.3.0.0)
    std::list <std::string> dl_search_path;
    // initial chug-in path (added 1.3.0.0)
    std::string initial_chugin_path;
    // if set as environment variable (added 1.3.0.0)
    if (getenv(g_chugin_path_envvar))
    {
        // get it from the env var
        initial_chugin_path = getenv(g_chugin_path_envvar);
    }
    else
    {
        // default it
        initial_chugin_path = g_default_chugin_path;
    }
    // parse the colon list into STL list (added 1.3.0.0)
    parse_path_list(initial_chugin_path, dl_search_path);
    // list of individually named chug-ins (added 1.3.0.0)
    std::list <std::string> named_dls;

    t_CKUINT count = 1;

    // set log level
    EM_setlog(log_level);

    // check buffer size
    buffer_size = ensurepow2(buffer_size);

    // allocate the vm - needs the type system
    vm = g_vm = new Chuck_VM;
    if (!vm->initialize(TRUE, vm_halt, srate, buffer_size,
            num_buffers, dac, adc, dac_chans, adc_chans,
            block, adaptive_size, force_srate))
    {
        fprintf(stderr, "[chuck]: %s\n", vm->last_error());
        exit(1);
    }

    // allocate the compiler
    compiler = g_compiler = new Chuck_Compiler;
    // initialize the compiler (search_apth and named_dls added 1.3.0.0 -- TODO: refactor)
    if (!compiler->initialize(vm, dl_search_path, named_dls))
    {
        fprintf(stderr, "[chuck]: error initializing compiler...\n");
        exit(1);
    }
    // enable dump
    compiler->emitter->dump = dump;
    // set auto depend
    compiler->set_auto_depend(auto_depend);

    // vm synthesis subsystem - needs the type system
    if (!vm->initialize_synthesis())
    {
        fprintf(stderr, "[chuck]: %s\n", vm->last_error());
        exit(1);
    }

    // set deprecate
    compiler->env->deprecate_level = deprecate_level;

    // reset count
    count = 1;

    load_chugins(compiler, vm);

    if (!sporkCode(compiler, vm, fileName, code))
    {
        exit(1);
    }

    // run the vm
    vm->run();
    //
    // // free vm
    // vm = NULL;
    // SAFE_DELETE(g_vm);
    // // free the compiler
    // compiler = NULL;
    // SAFE_DELETE(g_compiler);
}
}
