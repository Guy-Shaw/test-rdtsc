/*
 * Filename: src/main.c
 * Project: test-rdtsc
 * Brief: main() driver program for test-rdtsc.  Handles options, etc.
 *
 * Copyright (C) 2016 Guy Shaw
 * Written by Guy Shaw <gshaw@acm.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define _GNU_SOURCE 1

#include <stdlib.h>     // Import strtoul(), exit()
#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <ctype.h>      // Import isprint()

#include <cscript.h>

extern int test_rdtsc(size_t n, bool test_rdtscp, bool show_samples, bool show_statistics);

const char *program_path;
const char *program_name;

bool verbose     = false;
bool debug       = false;
bool test_rdtscp = false;

static bool show_samples    = false;
static bool show_statistics = false;

FILE *errprint_fh = NULL;
FILE *dbgprint_fh = NULL;

static struct option long_options[] = {
    {"help",            no_argument, 0,  'h'},
    {"version",         no_argument, 0,  'V'},
    {"verbose",         no_argument, 0,  'v'},
    {"debug",           no_argument, 0,  'd'},
    {"show-samples",    no_argument, 0,  's'},
    {"show-statistics", no_argument, 0,  'S'},
    {"rdtscp",          no_argument, 0,  'p'},
    {0, 0, 0, 0}
};

static const char usage_text[] =
    "Options:\n"
    "  --help|-h|-?           Show this help message and exit\n"
    "  --version              Show version information and exit\n"
    "  --verbose|-v           verbose\n"
    "  --debug|-d             debug\n"
    "  --rdtscp|-p            Test RDTSCP, instead of RDTSC\n"
    "  --show-samples|-s      show the samples\n"
    "  --show-statistics|-S   show the samples\n"
    ;


static const char version_text[] =
    "0.1\n"
    ;

static const char copyright_text[] =
    "Copyright (C) 2016 Guy Shaw\n"
    "Written by Guy Shaw\n"
    ;

static const char license_text[] =
    "License GPLv3+: GNU GPL version 3 or later"
    " <http://gnu.org/licenses/gpl.html>.\n"
    "This is free software: you are free to change and redistribute it.\n"
    "There is NO WARRANTY, to the extent permitted by law.\n"
    ;

static void
fshow_program_version(FILE *f)
{
    fputs(version_text, f);
    fputc('\n', f);
    fputs(copyright_text, f);
    fputc('\n', f);
    fputs(license_text, f);
    fputc('\n', f);
}

static void
show_program_version(void)
{
    fshow_program_version(stdout);
}

static void
usage(void)
{
    eprintf("usage: %s [ <options> ]\n", program_name);
    eprint(usage_text);
}

static inline bool
is_long_option(const char *s)
{
    return (s[0] == '-' && s[1] == '-');
}

static inline char *
vischar_r(char *buf, size_t sz, int c)
{
    if (isprint(c)) {
        buf[0] = c;
        buf[1] = '\0';
    }
    else {
        snprintf(buf, sz, "\\x%02x", c);
    }
    return (buf);
}

int
main(int argc, char **argv)
{
    extern char *optarg;
    extern int optind, opterr, optopt;
    int option_index;
    int err_count;
    int optc;
    size_t nsamples = 0;
    int rv;

    set_eprint_fh();
    set_debug_fh("");
    program_path = *argv;
    program_name = sname(program_path);
    option_index = 0;
    err_count = 0;
    opterr = 0;

    while (true) {
        int this_option_optind;

        if (err_count > 10) {
            eprintf("%s: Too many option errors.\n", program_name);
            break;
        }

        this_option_optind = optind ? optind : 1;
        optc = getopt_long(argc, argv, "+hVdpsv", long_options, &option_index);

        if (optc == -1) {
            break;
        }

        rv = 0;
        if (optc == '?' && optopt == '?') {
            optc = 'h';
        }

        switch (optc) {
        case 'V':
            show_program_version();
            exit(0);
            break;
        case 'h':
            fputs(usage_text, stdout);
            exit(0);
            break;
        case 'd':
            debug = true;
            set_debug_fh(NULL);
            break;
        case 'v':
            verbose = true;
            break;
        case 'p':
            test_rdtscp = true;
            break;
        case 's':
            show_samples = true;
            break;
        case 'S':
            show_statistics = true;
            break;
        case '?':
            eprint(program_name);
            eprint(": ");
            if (is_long_option(argv[this_option_optind])) {
                eprintf("unknown long option, '%s'\n",
                    argv[this_option_optind]);
            }
            else {
                char chrbuf[10];
                eprintf("unknown short option, '%s'\n",
                    vischar_r(chrbuf, sizeof (chrbuf), optopt));
            }
            ++err_count;
            break;
        default:
            eprintf("%s: INTERNAL ERROR: unknown option, '%c'\n",
                program_name, optopt);
            exit(2);
            break;
        }
    }

    if (optind < argc) {
        nsamples = strtoul(argv[optind], NULL, 10);
        if (nsamples) {
            ++optind;
        }
        else {
            eprintf("Invalid number of samples, %s.\n", argv[optind]);
            ++err_count;
        }
    }
    else {
        nsamples = 20;
    }

    if (verbose && optind < argc) {
        eprintf("non-option ARGV-elements:\n");
        while (optind < argc) {
            eprintf("    %s\n", argv[optind]);
            ++optind;
        }
    }

    if (err_count != 0) {
        usage();
        exit(1);
    }

    verbose = verbose || debug;

    rv = test_rdtsc(nsamples, test_rdtscp, show_samples, show_statistics);
    exit(rv);
}
