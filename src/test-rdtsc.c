/*
 * Filename: src/test-rdtsc.c
 * Project: test-rdtsc
 * Brief: Do sanity tests and some statistical tests on Intel x86 rdtsc()
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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>     // Import malloc()
#include <stdint.h>
#include <unistd.h>     // Import usleep()
#include <time.h>       // Import clock_gettime()

#include "rdtsc.h"
#include "rdtscp.h"

extern bool debug;

typedef uint64_t tsc_t;
typedef uint64_t hrtime_t;

static inline uint32_t
hi32(tsc_t t)
{
    return ((uint32_t)(t >> 32));
}

static inline uint32_t
lo32(tsc_t t)
{
    return ((uint32_t)(t));
}

static hrtime_t
timespec_diff(struct timespec *t0, struct timespec *t1)
{
    return (((t1->tv_sec - t0->tv_sec) * 1000000000) + (t1->tv_nsec - t0->tv_nsec));
}

int
test_rdtsc(size_t n, bool test_rdtscp, bool show_samples, bool show_stats)
{
    tsc_t *sample_tsc;
    struct timespec *sample_hrtime;
    size_t i;
    size_t inversions;
    bool err;

    sample_tsc    = (tsc_t *)malloc(n * sizeof (*sample_tsc));
    sample_hrtime = (struct timespec *)malloc(n * sizeof (*sample_hrtime));
    if (sample_tsc == NULL) {
        fprintf(stderr, "malloc() failed.\n");
         return (2);
    }

    if (sample_hrtime == NULL) {
        fprintf(stderr, "malloc() failed.\n");
         return (2);
    }

    if (test_rdtscp) {
        for (i = 0; i < n; ++i) {
            int rv;
            sample_tsc[i] = rdtscp();
            rv = clock_gettime(CLOCK_MONOTONIC, sample_hrtime + i);
            if (rv) {
                fprintf(stderr, "clock_gettime() failed.\n");
                exit(2);
            }
        }
    }
    else {
        for (i = 0; i < n; ++i) {
            int rv;
            sample_tsc[i] = rdtsc();
            rv = clock_gettime(CLOCK_MONOTONIC, sample_hrtime + i);
            if (rv) {
                fprintf(stderr, "clock_gettime() failed.\n");
                exit(2);
            }
        }
    }

    err = false;
    inversions = 0;
    for (i = 1; i < n; ++i) {
        if (sample_tsc[i] < sample_tsc[i - 1]) {
            ++inversions;
        }
    }

    if (inversions) {
        fprintf(stderr, "Inversions: %zu\n", inversions);
        err = true;
    }

    if (show_samples || show_stats) {
        struct timespec hrtime_res;
        double mean_hrtime_diff  = 0.0;
        double mean_tsc_diff     = 0.0;
        double tsc_diff_variance = 0.0;
        int rv;
        size_t sample_start = 5;

        rv = clock_getres(CLOCK_MONOTONIC, &hrtime_res);
        if (rv) {
            fprintf(stderr, "clock_getres() failed.\n");
            exit(2);
        }

        printf("Resolution of CLOCK_MONOTONIC: %lds:%lu nsec\n",
            hrtime_res.tv_sec, hrtime_res.tv_nsec);
        for (i = 0; i < n; ++i) {
            tsc_t t = sample_tsc[i];
            if (show_samples) {
                printf("%3zu %8x:%08x %16lu", i, hi32(t), lo32(t), t);
            }

            if (i > 0) {
                uint64_t tsc_diff;
                uint64_t hrtime_diff;

                tsc_diff = sample_tsc[i] - sample_tsc[i - 1];
                hrtime_diff = timespec_diff(sample_hrtime + i - 1, sample_hrtime + i);

                if (show_samples) {
                    printf(" %16lu %16lu", tsc_diff, hrtime_diff);
                }

                if (i > sample_start) {
                    double k = (double)(i - sample_start);
                    double ddiff = (double)tsc_diff - mean_tsc_diff;
                    double prev_m = mean_tsc_diff;
                    mean_tsc_diff += ddiff / k;
                    tsc_diff_variance = ((k - 1.0) * (ddiff * ddiff)) / k;

                    if (debug) {
                        double q    = tsc_diff_variance;
                        double svar = q / k;
                        fprintf(stderr, "\nsvar k=%g, M=%g, Q=%g, svar=%g\n",
                            k, prev_m, q, svar);
                    }

                    mean_hrtime_diff += ((double)hrtime_diff - mean_hrtime_diff) / k;
                }
                else {
                    mean_tsc_diff = tsc_diff;
                    tsc_diff_variance = 0.0;
                    mean_hrtime_diff = hrtime_diff;
                }
            }
            if (show_samples) {
                printf("\n");
            }
        }

        if (show_stats) {
            double hz = (double)mean_tsc_diff / (double)mean_hrtime_diff * 1000000000.0;
            double hzscale = 1.0;
            const char *units;

            if (hz > 1000000000000.0) {
                hzscale = 1000000000000.0;
                units = "THz";
            }
            else if (hz > 1000000000.0) {
                hzscale = 1000000000.0;
                units = "GHz";
            }
            else if (hz > 1000000.0) {
                hzscale = 1000000.0;
                units = "MHz";
            }
            else if (hz > 1000.0) {
                hzscale = 1000.0;
                units = "KHz";
            }
            else {
                hzscale = 1.0;
                units = "Hz";
            }

            double nsamples = n - (double)sample_start;
            double sample_var = tsc_diff_variance / (nsamples - 1.0);
            printf("Mean TSC diff = %g\n", mean_tsc_diff);
            printf("Sample variance of TSC diff = %g\n", sample_var);
            printf("%% coefficient of variance = %0.2f%%\n",
                sample_var / mean_tsc_diff * 100.0);
            printf("Approx. TSC frequency = %g %s\n", hz / hzscale, units);
        }
    }

    if (err) {
         return (1);
    }
    return (0);
}
