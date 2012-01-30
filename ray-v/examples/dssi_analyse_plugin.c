/* dssi_analyse_plugin.c
 *
 * Written by Sean Bolton, with inspiration from Richard Furse's analyseplugin
 * from the LADSPA SDK.
 *
 * This program is in the public domain.
 *
 * This program expects the name of a DSSI plugin to be provided on the
 * command line, in the form '[<path>]<so-name>[:<label>]' (e.g.
 * '/usr/lib/dssi/xsynth-dssi.so:Xsynth-DSSI'). It then lists various
 * information about the plugin.  If <path> is omitted, then the plugin
 * shared library is searched for in the colon-separated list of directories
 * specified in the environment variable DSSI_PATH.  If <label> is omitted,
 * all plugins in the shared library are shown.
 *
 * $Id: dssi_analyse_plugin.c,v 1.1 2010/06/27 19:30:09 smbolton Exp $
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <dlfcn.h>

#include <ladspa.h>
#include "dssi.h"
#include <alsa/asoundef.h>

int verbose = 0;

static char *_strdupn(const char *s, size_t n)
{
    char *t = malloc(n + 1);
    if (t) {
        memcpy(t, s, n);
        t[n] = 0;
    }
    return t;
}

static const char *_isnonnull(void *p, int ought)
{
    return (p ? "yes" : (ought ? "NO!" : "no "));
}

static void
print_cc(int cc)
{
    const char *s = NULL;

    printf(" CC%d", cc);
    switch (cc) {
      case MIDI_CTL_MSB_BANK:             s = "Error: bank select MSB should not be mapped!"; break;
      case MIDI_CTL_MSB_MODWHEEL:         s = "modwheel";            break;
      case MIDI_CTL_MSB_BREATH:           s = "breath";              break;
      case MIDI_CTL_MSB_FOOT:             s = "foot";                break;
      case MIDI_CTL_MSB_PORTAMENTO_TIME:  s = "portamento time";     break;
      case MIDI_CTL_MSB_DATA_ENTRY:       s = "data entry";          break;
      case MIDI_CTL_MSB_MAIN_VOLUME:      s = "volume";              break;
      case MIDI_CTL_MSB_BALANCE:          s = "balance";             break;
      case MIDI_CTL_MSB_PAN:              s = "pan";                 break;
      case MIDI_CTL_MSB_EXPRESSION:       s = "expression";          break;
      case MIDI_CTL_MSB_EFFECT1:          s = "effect 1";            break;
      case MIDI_CTL_MSB_EFFECT2:          s = "effect 2";            break;
      case MIDI_CTL_MSB_GENERAL_PURPOSE1: s = "general purpose control 1"; break;
      case MIDI_CTL_MSB_GENERAL_PURPOSE2: s = "general purpose control 2"; break;
      case MIDI_CTL_MSB_GENERAL_PURPOSE3: s = "general purpose control 3"; break;
      case MIDI_CTL_MSB_GENERAL_PURPOSE4: s = "general purpose control 4"; break;
      case MIDI_CTL_LSB_BANK:             s = "Error: bank select LSB should not be mapped!"; break;
      case MIDI_CTL_SUSTAIN:              s = "sustain";             break;
      case MIDI_CTL_PORTAMENTO:           s = "portamento";          break;
      case MIDI_CTL_SUSTENUTO:            s = "sostenuto";           break;
      case MIDI_CTL_SOFT_PEDAL:           s = "soft pedal";          break;
      case MIDI_CTL_LEGATO_FOOTSWITCH:    s = "legato footswitch";   break;
      case MIDI_CTL_HOLD2:                s = "hold 2 pedal";        break;
      case MIDI_CTL_SC1_SOUND_VARIATION:  s = "sc1 sound variation"; break;
      case MIDI_CTL_SC2_TIMBRE:           s = "sc2 timbre";          break;
      case MIDI_CTL_SC3_RELEASE_TIME:     s = "sc3 release time";    break;
      case MIDI_CTL_SC4_ATTACK_TIME:      s = "sc4 attack time";     break;
      case MIDI_CTL_SC5_BRIGHTNESS:       s = "sc5 brightness";      break;
      case MIDI_CTL_SC6:                  s = "sound control 6";     break;
      case MIDI_CTL_SC7:                  s = "sound control 7";     break;
      case MIDI_CTL_SC8:                  s = "sound control 8";     break;
      case MIDI_CTL_SC9:                  s = "sound control 9";     break;
      case MIDI_CTL_SC10:                 s = "sound control 10";    break;
      case MIDI_CTL_GENERAL_PURPOSE5:     s = "general purpose button 1"; break;
      case MIDI_CTL_GENERAL_PURPOSE6:     s = "general purpose button 2"; break;
      case MIDI_CTL_GENERAL_PURPOSE7:     s = "general purpose button 3"; break;
      case MIDI_CTL_GENERAL_PURPOSE8:     s = "general purpose button 4"; break;
      case MIDI_CTL_PORTAMENTO_CONTROL:   s = "portamento control";  break;
      case MIDI_CTL_E1_REVERB_DEPTH:      s = "effects level";       break;
      case MIDI_CTL_E2_TREMOLO_DEPTH:     s = "tremelo level";       break;
      case MIDI_CTL_E3_CHORUS_DEPTH:      s = "chorus level";        break;
      case MIDI_CTL_E4_DETUNE_DEPTH:      s = "celeste level";       break;
      case MIDI_CTL_E5_PHASER_DEPTH:      s = "phaser level";        break;
    }
    if (s) printf(" (%s)", s);
}

void
describe_plugin(const DSSI_Descriptor *descriptor, const char *so_directory,
                const char *so_name, const char *label)
{
    int i;
    const DSSI_Descriptor   *dd = descriptor;
    const LADSPA_Descriptor *ld = descriptor->LADSPA_Plugin;
    LADSPA_Handle instance;

    printf("Label:     %s\n", ld->Label);
    printf("Name:      %s\n", ld->Name);
    printf("Maker:     %s\n", ld->Maker);
    printf("Copyright: %s\n", ld->Copyright);

    printf("Properties:");
    if (LADSPA_IS_REALTIME(ld->Properties))
        printf(" REALTIME");
    else
        printf(" (not REALTIME)");
    if (LADSPA_IS_INPLACE_BROKEN(ld->Properties))
        printf(" INPLACE_BROKEN");
    else
        printf(" (not INPLACE_BROKEN)");
    if (LADSPA_IS_HARD_RT_CAPABLE(ld->Properties))
        printf(" HARD_RT_CAPABLE\n");
    else
        printf(" (not HARD_RT_CAPABLE)\n");

    if (dd->DSSI_API_Version != 1)
        printf("Put on your future-proof bat-panties, Robin! This plugin reports DSSI API\n"
               "version %d! Proceeding anyway....\n", dd->DSSI_API_Version);
    else
        printf("DSSI API Version: 1\n");

    printf("Functions available:\n");
    printf("    %s  instantiate()          %s  configure()\n",
           _isnonnull(ld->instantiate, 1),
           _isnonnull(dd->configure, 0));
    printf("    %s  connect_port()         %s  get_program()\n",
           _isnonnull(ld->connect_port, 1),
           _isnonnull(dd->get_program, 0));
    printf("    %s  activate()             %s  select_program()\n",
           _isnonnull(ld->activate, 0),
           _isnonnull(dd->select_program, 0));
    printf("    %s  run()                  %s  get_midi_controller_for_port()\n",
           _isnonnull(ld->run, 0),
           _isnonnull(dd->get_midi_controller_for_port, 0));
    printf("    %s  run_adding()           %s  run_synth()\n",
           _isnonnull(ld->run_adding, (ld->set_run_adding_gain != NULL)),
           _isnonnull(dd->run_synth, 0));
    printf("    %s  set_run_adding_gain()  %s  run_synth_adding()\n",
           _isnonnull(ld->set_run_adding_gain, 
                          (ld->run_adding != NULL ||
                           dd->run_synth_adding != NULL ||
                           dd->run_multiple_synths_adding != NULL)),
           _isnonnull(dd->run_synth_adding, (ld->set_run_adding_gain != NULL)));
    printf("    %s  deactivate()           %s  run_multiple_synths()\n",
           _isnonnull(ld->deactivate, 0),
           _isnonnull(dd->run_multiple_synths, 0));
    printf("    %s  cleanup()              %s  run_multiple_synths_adding()\n",
           _isnonnull(ld->cleanup, 1),
           _isnonnull(dd->run_multiple_synths_adding, (ld->set_run_adding_gain != NULL)));
    
    if (ld->PortCount) {
        printf("Ports (%lu):\n", ld->PortCount);
             /* -------------------------------------------------------------------------------- 80 chars */
        printf("  --- name---------------- type------- min----- max----- default flags---------\n");
             /*     1 OSC1 Pitch           control in      0.25        4   max   *srate log int */
    } else
        printf("Error: plugin has no ports!\n");

    for (i = 0; i < ld->PortCount; i++) {
        LADSPA_PortDescriptor pd = ld->PortDescriptors[i];
        const LADSPA_PortRangeHint *prh = &ld->PortRangeHints[i];
        LADSPA_PortRangeHintDescriptor hd = prh->HintDescriptor;

        printf("  %3d %-20s", i, ld->PortNames[i]);
        if ((pd & (LADSPA_PORT_CONTROL | LADSPA_PORT_AUDIO)) == LADSPA_PORT_CONTROL) {
            printf(" control");
        } else if ((pd & (LADSPA_PORT_CONTROL | LADSPA_PORT_AUDIO)) == LADSPA_PORT_AUDIO) {
            printf(" audio  ");
        } else {
            printf(" 0x%02x??", (pd & (LADSPA_PORT_CONTROL | LADSPA_PORT_AUDIO)));
        }
        if ((pd & (LADSPA_PORT_INPUT | LADSPA_PORT_OUTPUT)) == LADSPA_PORT_INPUT) {
            printf(" in ");
        } else if ((pd & (LADSPA_PORT_INPUT | LADSPA_PORT_OUTPUT)) == LADSPA_PORT_OUTPUT) {
            printf(" out");
        } else {
            printf(" %02x?", (pd & (LADSPA_PORT_INPUT | LADSPA_PORT_OUTPUT)));
        }

        /* PortRangeHints */
        if (LADSPA_IS_HINT_TOGGLED(hd)) {
            printf("  toggled         ");
        } else {
            if (LADSPA_IS_HINT_BOUNDED_BELOW(hd))
                printf("%9g", prh->LowerBound);
            else
                printf("         ");
            if (LADSPA_IS_HINT_BOUNDED_ABOVE(hd))
                printf("%9g", prh->UpperBound);
            else
                printf("         ");
        }
        if      (LADSPA_IS_HINT_DEFAULT_MINIMUM(hd)) printf("   min  ");
        else if (LADSPA_IS_HINT_DEFAULT_LOW(hd))     printf("   low  ");
        else if (LADSPA_IS_HINT_DEFAULT_MIDDLE(hd))  printf("   mid  ");
        else if (LADSPA_IS_HINT_DEFAULT_HIGH(hd))    printf("   hi   ");
        else if (LADSPA_IS_HINT_DEFAULT_MAXIMUM(hd)) printf("   max  ");
        else if (LADSPA_IS_HINT_DEFAULT_0(hd))       printf("   0    ");
        else if (LADSPA_IS_HINT_DEFAULT_1(hd))       printf("   1    ");
        else if (LADSPA_IS_HINT_DEFAULT_100(hd))     printf("   100  ");
        else if (LADSPA_IS_HINT_DEFAULT_440(hd))     printf("   440  ");
        else if (!LADSPA_IS_HINT_HAS_DEFAULT(hd))    printf("        ");
        else                                         printf(" invalid?");
        if (LADSPA_IS_HINT_SAMPLE_RATE(hd)) printf(" *srate");
        if (LADSPA_IS_HINT_LOGARITHMIC(hd)) printf(" log");
        if (LADSPA_IS_HINT_INTEGER(hd))     printf(" int");

        putchar('\n');

        if (LADSPA_IS_HINT_TOGGLED(hd)) {
            /* "LADSPA_HINT_TOGGLED may not be used in conjunction with any
             * other hint except LADSPA_HINT_DEFAULT_0 or LADSPA_HINT_DEFAULT_1." */
            if ((hd                  | LADSPA_HINT_DEFAULT_0 | LADSPA_HINT_DEFAULT_1) !=
                (LADSPA_HINT_TOGGLED | LADSPA_HINT_DEFAULT_0 | LADSPA_HINT_DEFAULT_1))
                printf("      Error: 'toggled' is incompatible with other supplied hints.\n");
        }
    }

    instance = NULL;
    if (ld->instantiate && (dd->get_midi_controller_for_port ||
                            (verbose && dd->get_program))) {
        instance = ld->instantiate(ld, 44100L);
        if (!instance)
            printf("Error: unable to instantiate plugin!\n");
    }

    if (instance) {

        if (dd->get_midi_controller_for_port) {
            int map;
            int found = 0;

            printf("Requested MIDI Control Change/NRPN Mappings:\n");
            for (i = 0; i < ld->PortCount; i++) {
                LADSPA_PortDescriptor pd = ld->PortDescriptors[i];
                if (LADSPA_IS_PORT_INPUT(pd) && LADSPA_IS_PORT_CONTROL(pd)) {
                    map = dd->get_midi_controller_for_port(instance, i);
                    if (DSSI_CONTROLLER_IS_SET(map)) {
                        printf("  %3d %-20s ", i, ld->PortNames[i]);
                        if (DSSI_IS_CC(map))
                            print_cc(DSSI_CC_NUMBER(map));
                        if (DSSI_IS_NRPN(map))
                            printf(" NRPN%d", DSSI_NRPN_NUMBER(map));
                        if (!DSSI_IS_CC(map) && !DSSI_IS_NRPN(map))
                            printf(" Error: no valid mappings returned!");
                        putchar('\n');
                        found = 1;
                    }
                }
            }
            if (!found) printf("    (none)\n");
        }

        if (verbose && dd->get_program) {
            const DSSI_Program_Descriptor *pd;
            int found = 0;

            printf("Default Programs:\n");
            printf("  bank  num  name----------------------------------------\n");

            for (i = 0; (pd = dd->get_program(instance, i)) != NULL; i++) {
                printf("  %4lu  %3lu  %s\n", pd->Bank, pd->Program, pd->Name);
                found = 1;
            }
            if (!found) printf("    (none)\n");
        }

        if (ld->cleanup)
            ld->cleanup(instance);
    }

    { /* user interfaces */
        int so_len, label_len;
        char *so_base, *ui_dir;
        DIR *dir;
        struct dirent *dirent;
        struct stat statbuf;
        int found = 0;

        printf("Available User Interfaces:\n");

        so_len = strlen(so_name);
        if (so_len > 3 && !strcmp(so_name + so_len - 3, ".so")) {
            so_len -= 3;
            so_base = _strdupn(so_name, so_len);
        } else
            so_base = strdup(so_name);  /* okay, so what are you, a .dll, a .dylib? */

        ui_dir = malloc(strlen(so_directory) + strlen(so_base) + 2);
        sprintf(ui_dir, "%s/%s", so_directory, so_base);

        label_len = strlen(label);
        if ((dir = opendir(ui_dir)) != NULL) {
            while ((dirent = readdir(dir)) != NULL) {
                if ((!strncmp(dirent->d_name, so_base, so_len) &&
                         dirent->d_name[so_len] == '_') ||
                    (!strncmp(dirent->d_name, label, label_len) &&
                         dirent->d_name[label_len] == '_')) {

                    char *ui_path = malloc(strlen(ui_dir) + strlen(dirent->d_name) + 2);

                    sprintf(ui_path, "%s/%s", ui_dir, dirent->d_name);

                    if (stat(ui_path, &statbuf)) {
                        printf("    (found %s, but could not stat it: %s)\n", 
                               ui_path, strerror(errno));

                    } else if ((S_ISREG(statbuf.st_mode) || S_ISLNK(statbuf.st_mode)) &&
                               (statbuf.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))) {
                        printf("    %s\n", ui_path);
                        found = 1;
                    } else {
                        printf("    (found %s, but it is not an executable file)\n", ui_path);
                    }

                    free(ui_path);
                }
            }
        }
        free(ui_dir);
        free(so_base);

        if (!found) printf("    (none)\n");
    }
}

void
usage(const char *program_name)
{
    fprintf(stderr, "usage: %s [-v|--verbose] [<path>]<DSSI-so-name>[:<label>]\n\n", program_name);
    fprintf(stderr, "Example: %s -v /usr/lib/dssi/xsynth-dssi.so:Xsynth-DSSI\n", program_name);
    fprintf(stderr, "If <path> is omitted, then the plugin library is searched for in the colon-\n");
    fprintf(stderr, "list of directories specified in the environment variable DSSI_PATH. If\n");
    fprintf(stderr, "<label> is omitted, all plugins in the shared library are shown.\n");
    fprintf(stderr, "Optional argument:\n\n");
    fprintf(stderr, "  -v, --verbose    Provide additional information, including default program\n");
    fprintf(stderr, "                   names.\n");

    exit(1);
}

int 
main(int argc, char *argv[])
{
    char *so_path, *so_directory, *so_name, *label;
    char *tmp;
    void *handle = 0;
    DSSI_Descriptor_Function dssi_descriptor_function;
    LADSPA_Descriptor_Function ladspa_descriptor_function;
    int id;
    const DSSI_Descriptor *descriptor;
    int label_found;

    if (argc < 2 || argc > 3 || !strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
        usage(argv[0]); /* does not return */
    }

    if (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--verbose"))
        verbose = 1;

    /* if plugin label was given, separate it from shared library name */
    tmp = strrchr(argv[argc - 1], ':');
    if (tmp) {
        so_name = _strdupn(argv[argc - 1], tmp - argv[argc - 1]);
        label = strdup(tmp + 1);
    } else {
        so_name = strdup(argv[argc - 1]);
        label = NULL;
    }

    /* if absolute path was given, separate directory from basename */
    tmp = strrchr(so_name, '/');
    if (tmp) {
        so_path = so_name;
        so_directory = _strdupn(so_name, tmp - so_name);
        so_name = strdup(tmp + 1);
    } else {
        so_path = NULL;
        so_directory = NULL;
    }
    
    if (so_path) { /* absolute path */

        if ((handle = dlopen(so_path, RTLD_LAZY)) == NULL) {
            fprintf(stderr, "Error: can't load DSSI or LADSPA plugin from '%s': %s\n",
                    so_path, dlerror());
            exit(1);
        }

    } else { /* no path given, search over DSSI_PATH */

        char *dssi_path = getenv("DSSI_PATH");
        char *pathtmp, *element;

        if (!dssi_path) {
            dssi_path = "/usr/local/lib/dssi:/usr/lib/dssi";
            fprintf(stderr, "Warning: DSSI_PATH not set, defaulting to '%s'\n", dssi_path);
        } else if (verbose) {
            printf("Searching DSSI_PATH '%s'...\n", dssi_path);
        }

        pathtmp = strdup(dssi_path);
        tmp = pathtmp;
        while ((element = strtok(tmp, ":")) != 0) {
            tmp = NULL;

            so_path = malloc(strlen(element) + strlen(so_name) + 2);
            sprintf(so_path, "%s/%s", element, so_name);

            if ((handle = dlopen(so_path, RTLD_LAZY))) {
                so_directory = strdup(element);
                break;
            }

            free(so_path);
            so_path = NULL;
        }

        free(pathtmp);

        if (!handle) {
            fprintf(stderr, "Error: couldn't locate DSSI or LADSPA plugin library '%s' on path '%s'\n",
                    so_name, dssi_path);
            exit(1);
        }
    }

    dssi_descriptor_function = (DSSI_Descriptor_Function)dlsym(handle, "dssi_descriptor");

    if (!dssi_descriptor_function) {
        fprintf(stderr, "Error: shared library '%s' is not a DSSI plugin library\n", so_path);
        exit(1);
    }

    printf("Examining DSSI plugin library %s:\n", so_path);

    label_found = 0;
    for (id = 0; (descriptor = dssi_descriptor_function(id)); id++) {
        if (label) {
            if (!strcmp(label, descriptor->LADSPA_Plugin->Label)) {
                label_found = 1;
                describe_plugin(descriptor, so_directory, so_name, label);
                break;
            }
        } else {
            if (id > 0) putchar('\n');
            describe_plugin(descriptor, so_directory, so_name, descriptor->LADSPA_Plugin->Label);
            label_found++;
        }
    }

    if (!label_found) {
        if (label)
            fprintf(stderr, "Error: could not find plugin '%s' in DSSI library '%s'\n", label, so_path);
        else
            printf("(Hmm, this DSSI plugin library reports no DSSI plugins -- perhaps something\n"
                   " wrong with its initialization code?)\n");
    }

    ladspa_descriptor_function = (LADSPA_Descriptor_Function)dlsym(handle, "ladspa_descriptor");
    if (verbose && ladspa_descriptor_function) {
        if (label)
            printf("This library exports a LADSPA descriptor function as well.\n");
        else {
            const LADSPA_Descriptor *ldescriptor;

            printf("This library exports a LADSPA descriptor function as well. LADSPA plugins are:\n");
            for (id = 0; (ldescriptor = ladspa_descriptor_function(id)); id++)
                printf("    %-16s  (%lu) %s\n", ldescriptor->Label, ldescriptor->UniqueID, ldescriptor->Name);
            if (id == 0)
                printf("    (Oddly, this library has a LADSPA descriptor function but reports no plugins.)\n");
        }
    }

    dlclose(handle);
    free(label);
    free(so_name);
    free(so_directory);
    free(so_path);

    return 0;
}

