/* dssi_list_plugins.c
 *
 * Written by Sean Bolton, with inspiration from Richard Furse's listplugins
 * from the LADSPA SDK.
 *
 * This program is in the public domain.
 *
 * $Id: dssi_list_plugins.c,v 1.1 2010/06/27 19:30:09 smbolton Exp $
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <dlfcn.h>

#include <ladspa.h>
#include "dssi.h"

struct duplicate_list {
    struct duplicate_list *next;
    const char *item;
};

struct duplicate_list *dup_list = NULL;

int verbose = 0;

void
check_for_duplicates(const char *filename)
{
    struct duplicate_list *d;

    for (d = dup_list; d; d = d->next) {
        if (!strcmp(d->item, filename)) {
            printf("-- warning: shared library '%s' appears more than once on DSSI_PATH\n", filename);
            return;
        }
    }

    /* not found, add it to the list */
    d = (struct duplicate_list *)malloc(sizeof(struct duplicate_list));
    d->item = strdup(filename);
    d->next = dup_list;
    dup_list = d;
}

void
free_dup_list(void)
{
    struct duplicate_list *d;

    while (dup_list) {
        d = dup_list;
        dup_list = d->next;
        free((char *)d->item);
        free(d);
    }
}

void
list_dssi_plugins(DSSI_Descriptor_Function descriptor_function)
{
    int i;
    const DSSI_Descriptor *descriptor;

    for (i = 0; (descriptor = descriptor_function(i)); i++)
        printf("\t%-16s  %s\n", descriptor->LADSPA_Plugin->Label, 
                                descriptor->LADSPA_Plugin->Name);

    if (verbose && i == 0)
        printf("-- odd ... no plugins found within this shared object\n");
}

void
list_ladspa_plugins(LADSPA_Descriptor_Function descriptor_function)
{
    int i;
    const LADSPA_Descriptor *descriptor;

    for (i = 0; (descriptor = descriptor_function(i)); i++)
        printf("\t%-16s  (%lu) %s\n", descriptor->Label, descriptor->UniqueID, descriptor->Name);

    if (verbose && i == 0)
        printf("-- odd ... no plugins found within this shared object\n");
}

void
list_directory(char *directory)
{
    int directory_length = strlen(directory);
    DIR *dir;
    struct dirent *dirent;
    char *filename;
    struct stat statbuf;
    void *handle;
    int found = 0;

    if (directory_length == 0) {
        if (verbose) printf("-- warning: skipping zero-length element in DSSI_PATH\n");
        return;
    }
    if (directory[directory_length - 1] == '/')
        directory[directory_length - 1] = 0; /* slash gets added again below */

    dir = opendir(directory);
    if (!dir) {
        if (verbose) printf("-- warning: couldn't open DSSI_PATH directory element '%s'\n", directory);
        return;
    }

    if (verbose) printf("-- scanning directory %s\n", directory);

    while ((dirent = readdir(dir)) != NULL) {

        if (!strcmp(dirent->d_name, ".") || !strcmp(dirent->d_name, ".."))
            continue;

        filename = malloc(directory_length + strlen(dirent->d_name) + 2);
        sprintf(filename, "%s/%s", directory, dirent->d_name);

        if (stat(filename, &statbuf)) {
            if (verbose)
                printf("-- warning: couldn't stat file '%s': %s\n", filename, strerror(errno));
            free(filename);
            continue;
        }
        if (S_ISDIR(statbuf.st_mode)) { /* silently skip subdirectories */
            free(filename);
            continue;
        }

        handle = dlopen(filename, RTLD_LAZY);
        if (handle) {
            DSSI_Descriptor_Function dssi_descriptor_function;
            LADSPA_Descriptor_Function ladspa_descriptor_function;

            if (verbose)
                check_for_duplicates(dirent->d_name);

            dssi_descriptor_function = (DSSI_Descriptor_Function)dlsym(handle, "dssi_descriptor");
            ladspa_descriptor_function = (LADSPA_Descriptor_Function)dlsym(handle, "ladspa_descriptor");

            if (dssi_descriptor_function) {
                printf("%s\n", filename);
                list_dssi_plugins(dssi_descriptor_function);
                found++;
            } else if (ladspa_descriptor_function) {
                printf("%s (LADSPA-only)\n", filename);
                list_ladspa_plugins(ladspa_descriptor_function);
            } else {
                if (verbose)
                    printf("-- warning: shared object '%s' is neither a LADSPA nor DSSI plugin\n", filename);
            }
            dlclose(handle);
        } else { /* not a library */
            int len = strlen(dirent->d_name);

            if (len > 4 && !strcmp(dirent->d_name + len - 3, ".la")) {  /* libtool *.la file */
                /* if (verbose) printf("-- skipping file '%s'\n", filename); */
            } else
                if (verbose)
                    printf("-- warning: couldn't dlopen file '%s': %s\n", filename, dlerror());
        }
        free(filename);
    }

    closedir(dir);

    if (verbose && found == 0)
        printf("-- odd ... no DSSI plugins were found in this directory\n");
}

void
usage(const char *program_name)
{
    fprintf(stderr, "usage: %s [-v|--verbose]\n", program_name);
    fprintf(stderr, "Scan the directories listed in environment variable DSSI_PATH, and list\n");
    fprintf(stderr, "the DSSI plugins found therein.  Optional arguments:\n");
    fprintf(stderr, "  -v, --verbose    describe the scan and error conditions more verbosely.\n");

    exit(1);
}

int 
main(int argc, char *argv[])
{
    char *path, *pathtmp, *element;

    while (argc > 1) {
        argc--;
        if (!strcmp(argv[argc], "-v") || !strcmp(argv[argc], "--verbose"))
            verbose = 1;
        else
            usage(argv[0]); /* does not return */
    }

    path = getenv("DSSI_PATH");
    if (!path) {
        path = "/usr/local/lib/dssi:/usr/lib/dssi";
        fprintf(stderr, "warning: DSSI_PATH not set, defaulting to '%s'\n", path);
    }

    path = strdup(path);
    pathtmp = path;
    while ((element = strtok(pathtmp, ":")) != 0) {
        pathtmp = NULL;
        list_directory(element);
    }
    free(path);

    free_dup_list();

    return 0;
}

