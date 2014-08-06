/* -*- c-basic-offset: 4 -*-  vi:set ts=8 sts=4 sw=4: */
/*
This file is part of the PyDAW project, Copyright PyDAW Team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef _PYDAW_MAIN_H
#define _PYDAW_MAIN_H

//#include "../include/pydaw_plugin.h"
#include <lo/lo.h>

#define D3H_MAX_CHANNELS   16  /* MIDI limit */
#define D3H_MAX_INSTANCES  (D3H_MAX_CHANNELS)

/* character used to seperate DSO names from plugin labels on command line */
#define LABEL_SEP ':'

typedef struct _d3h_dll_t d3h_dll_t;

struct _d3h_dll_t {
    d3h_dll_t               *next;
    char                    *name;
    int                      is_PYINST_dll;
    PYFX_Descriptor_Function descfn;
};

typedef struct _d3h_plugin_t d3h_plugin_t;

struct _d3h_plugin_t {
    d3h_dll_t             *dll;
    char                  *label;
    int                    is_first_in_dll;
    PYFX_Descriptor *descriptor;
    int                    ins;
    int                    outs;
    int                    controlIns;
    int                    controlOuts;
};

typedef struct _d3h_instance_t d3h_instance_t;

#define MIDI_CONTROLLER_COUNT 128

struct _d3h_instance_t
{
    d3h_plugin_t    *plugin;
    char            *friendly_name;
    char            *ui_osc_configure_path;
};

#endif /* _PYDAW_MAIN_H */

