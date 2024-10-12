#define _GNU_SOURCE

#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "randombytes.h"
#include "api.h"

#include <time.h>
#include <papi.h>

#include <sys/resource.h>

#define NUM_EVENTS 4


// Use monotonic clock for time measurement
inline static uint64_t cpucyclesStart(void) {
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);
    return (uint64_t)start.tv_sec * 1000000000L + start.tv_nsec;
}

inline static uint64_t cpucyclesStop(void) {
    struct timespec stop;
    clock_gettime(CLOCK_MONOTONIC, &stop);
    return (uint64_t)stop.tv_sec * 1000000000L + stop.tv_nsec;
}

int main(void) {
    unsigned long long mira128f_mlen = 7;
    unsigned long long mira128f_smlen;

    unsigned char mira128f_m[] = {0x4d, 0x49, 0x6e, 0x52, 0x41, 0x6e, 0x6b, 
                                    0x4d, 0x49, 0x6e, 0x52, 0x41, 0x6e, 0x6b, 
                                    0x4d, 0x49, 0x6e, 0x52, 0x41, 0x6e, 0x6b
    };
    unsigned char mira128f_pk[CRYPTO_PUBLICKEYBYTES];
    unsigned char mira128f_sk[CRYPTO_SECRETKEYBYTES];
    unsigned char mira128f_sm[CRYPTO_BYTES + mira128f_mlen];

    unsigned long long t1, t2;

    unsigned char seed[48] = {0};
    randombytes_init(seed, NULL, 256);

    struct rusage usage_before, usage_after;
    getrusage(RUSAGE_SELF, &usage_before);

    // PAPI Initialization
    if (PAPI_library_init(PAPI_VER_CURRENT) != PAPI_VER_CURRENT) {
        fprintf(stderr, "PAPI library init error!\n");
        exit(1);
    }

    int event_set = PAPI_NULL;
    long long values[4];  // Store PAPI results
    int events[] = {PAPI_TOT_CYC, PAPI_TOT_INS, PAPI_L1_DCM, PAPI_L2_DCM};  // Track cycles, instructions, L1 cache misses, L2 cache misses

    


    // Create event set
    if (PAPI_create_eventset(&event_set) != PAPI_OK) {
        fprintf(stderr, "PAPI create event set error!\n");
        exit(1);
    }

    // // Add events
    // if (PAPI_add_events(event_set, events, 3) != PAPI_OK) {
    //     fprintf(stderr, "PAPI add events error!\n");
    //     exit(1);
    // }

    // handle errors when adding events
    for (int i = 0; i < NUM_EVENTS; i++) {
        int retval = PAPI_add_event(event_set, events[i]);
        if (retval != PAPI_OK) {
            fprintf(stderr, "PAPI add event error: %s\n", PAPI_strerror(retval));
            exit(1);
        }
    }

    /*************/
    /* MIRA-128F */
    /*************/

    // Start PAPI counters
    if (PAPI_start(event_set) != PAPI_OK) {
        fprintf(stderr, "PAPI start counters error!\n");
        exit(1);
    }
    
    // keygen
    t1 = cpucyclesStart();
    if (crypto_sign_keypair(mira128f_pk, mira128f_sk) == -1) {
        printf("\nFailed\n\n");
        return -1;
    }
    t2 = cpucyclesStop();

    // Stop PAPI and get performance metrics
    if (PAPI_stop(event_set, values) != PAPI_OK) {
        fprintf(stderr, "PAPI stop counters error!\n");
        exit(1);
    }

    getrusage(RUSAGE_SELF, &usage_after);
    long memory_used_kb = usage_after.ru_maxrss - usage_before.ru_maxrss;
    printf("Memory used: %ld KB\n", memory_used_kb);

    printf("MIRA-128F keypair generation:\n");
    printf("  Total cycles: %lld\n", values[0]);
    printf("  Total instructions: %lld\n", values[1]);
    printf("  L1 data cache misses: %lld\n", values[2]);
    printf("  L2 data cache misses: %lld\n", values[3]);
    printf("  Execution time: %llu nanoseconds\n", t2 - t1);


    // Reset PAPI counters before the next measurement
    if (PAPI_reset(event_set) != PAPI_OK) {
        fprintf(stderr, "PAPI reset error!\n");
        exit(1);
    }

    // Measure Signing
    if (PAPI_start(event_set) != PAPI_OK) {
        fprintf(stderr, "PAPI start counters error for signing!\n");
        exit(1);
    }
    getrusage(RUSAGE_SELF, &usage_before);
    t1 = cpucyclesStart();
    if (crypto_sign(mira128f_sm, &mira128f_smlen, mira128f_m, mira128f_mlen, mira128f_sk) != 0) {
        printf("\nSigning Failed\n\n");
        return -1;
    }
    t2 = cpucyclesStop();

    if (PAPI_stop(event_set, values) != PAPI_OK) {
        fprintf(stderr, "PAPI stop counters error for signing!\n");
        exit(1);
    }

    getrusage(RUSAGE_SELF, &usage_after);
    memory_used_kb = usage_after.ru_maxrss - usage_before.ru_maxrss;
    printf("Memory used: %ld KB\n", memory_used_kb);

    printf("MIRA-128F signing:\n");
    printf("  Total cycles: %lld\n", values[0]);
    printf("  Total instructions: %lld\n", values[1]);
    printf("  L1 data cache misses: %lld\n", values[2]);
    printf("  L2 data cache misses: %lld\n", values[3]);
    printf("  Execution time: %llu nanoseconds\n", t2 - t1);

    // Reset PAPI counters before the next measurement
    if (PAPI_reset(event_set) != PAPI_OK) {
        fprintf(stderr, "PAPI reset error!\n");
        exit(1);
    }

    // Measure Verification
    if (PAPI_start(event_set) != PAPI_OK) {
        fprintf(stderr, "PAPI start counters error for verification!\n");
        exit(1);
    }

    getrusage(RUSAGE_SELF, &usage_before);
    t1 = cpucyclesStart();
    unsigned char *mout = malloc(mira128f_smlen);
    unsigned long long mout_len;
    if (crypto_sign_open(mout, &mout_len, mira128f_sm, mira128f_smlen, mira128f_pk) != 0) {
        printf("\nVerification Failed\n\n");
        return -1;
    }
    t2 = cpucyclesStop();

    if (PAPI_stop(event_set, values) != PAPI_OK) {
        fprintf(stderr, "PAPI stop counters error for verification!\n");
        exit(1);
    }

    getrusage(RUSAGE_SELF, &usage_after);
    memory_used_kb = usage_after.ru_maxrss - usage_before.ru_maxrss;
    printf("Memory used: %ld KB\n", memory_used_kb);

    printf("MIRA-128F verification:\n");
    printf("  Total cycles: %lld\n", values[0]);
    printf("  Total instructions: %lld\n", values[1]);
    printf("  L1 data cache misses: %lld\n", values[2]);
    printf("  L2 data cache misses: %lld\n", values[3]);
    printf("  Execution time: %llu nanoseconds\n", t2 - t1);

    free(mout);

    // Clean up PAPI
    if (PAPI_cleanup_eventset(event_set) != PAPI_OK) {
        fprintf(stderr, "PAPI cleanup event set error!\n");
        exit(1);
    }
    if (PAPI_destroy_eventset(&event_set) != PAPI_OK) {
        fprintf(stderr, "PAPI destroy event set error!\n");
        exit(1);
    }

    // Clean up PAPI
    PAPI_shutdown();

    printf("Public key size: %zu bytes\n", sizeof(mira128f_pk));
    printf("Secret key size: %zu bytes\n", sizeof(mira128f_sk));
    printf("Signature size: %d bytes\n", CRYPTO_BYTES);

    return 0;
}
