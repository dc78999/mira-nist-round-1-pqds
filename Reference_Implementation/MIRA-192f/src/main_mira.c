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

    unsigned long long mira192f_mlen = 21;
    unsigned long long mira192f_smlen;

    unsigned char mira192f_m[] = {0x4d, 0x49, 0x6e, 0x52, 0x41, 0x6e, 0x6b,
                                    0x4d, 0x49, 0x6e, 0x52, 0x41, 0x6e, 0x6b,
                                    0x4d, 0x49, 0x6e, 0x52, 0x41, 0x6e, 0x6b
    };

    unsigned char mira192f_pk[CRYPTO_PUBLICKEYBYTES];
    unsigned char mira192f_sk[CRYPTO_SECRETKEYBYTES];
    unsigned char mira192f_sm[CRYPTO_BYTES + mira192f_mlen];

    unsigned long long t1, t2, t3, t4, t5, t6;



    unsigned char seed[48] = {0};
//    (void)syscall(SYS_getrandom, seed, 48, 0);
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

    // handle errors when adding events
    for (int i = 0; i < NUM_EVENTS; i++) {
        int retval = PAPI_add_event(event_set, events[i]);
        if (retval != PAPI_OK) {
            fprintf(stderr, "PAPI add event error: %s\n", PAPI_strerror(retval));
            exit(1);
        }
    }

    /*************/
    /* MIRA-192F */
    /*************/

    // Start PAPI counters
    if (PAPI_start(event_set) != PAPI_OK) {
        fprintf(stderr, "PAPI start counters error!\n");
        exit(1);
    }

    t1 = cpucyclesStart();
    if (crypto_sign_keypair(mira192f_pk, mira192f_sk) == -1) {
        printf("\nnFailed\n\n");
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

    printf("MIRA-192F keypair generation:\n");
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
    t3 = cpucyclesStart();
    if (crypto_sign(mira192f_sm, &mira192f_smlen, mira192f_m, mira192f_mlen, mira192f_sk) != 0) {
        printf("\nnFailed\n\n");
        return -1;
    };
    t4 = cpucyclesStop();

    if (PAPI_stop(event_set, values) != PAPI_OK) {
        fprintf(stderr, "PAPI stop counters error for signing!\n");
        exit(1);
    }

    getrusage(RUSAGE_SELF, &usage_after);
    memory_used_kb = usage_after.ru_maxrss - usage_before.ru_maxrss;
    printf("Memory used: %ld KB\n", memory_used_kb);

    printf("MIRA-192F signing:\n");
    printf("  Total cycles: %lld\n", values[0]);
    printf("  Total instructions: %lld\n", values[1]);
    printf("  L1 data cache misses: %lld\n", values[2]);
    printf("  L2 data cache misses: %lld\n", values[3]);
    printf("  Execution time: %llu nanoseconds\n", t4 - t3);

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
    unsigned char *mout = malloc(mira192f_smlen);
    unsigned long long mout_len;

    t5 = cpucyclesStart();
    if (crypto_sign_open(mout, &mout_len, mira192f_sm, mira192f_smlen, mira192f_pk) == -1) {
        printf("\nFailed\n\n");
        return -1;
    }
    t6 = cpucyclesStop();

    if (PAPI_stop(event_set, values) != PAPI_OK) {
        fprintf(stderr, "PAPI stop counters error for verification!\n");
        exit(1);
    }

    getrusage(RUSAGE_SELF, &usage_after);
    memory_used_kb = usage_after.ru_maxrss - usage_before.ru_maxrss;
    printf("Memory used: %ld KB\n", memory_used_kb);

    printf("MIRA-192F verification:\n");
    printf("  Total cycles: %lld\n", values[0]);
    printf("  Total instructions: %lld\n", values[1]);
    printf("  L1 data cache misses: %lld\n", values[2]);
    printf("  L2 data cache misses: %lld\n", values[3]);
    printf("  Execution time: %llu nanoseconds\n", t6 - t5);

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

    printf("Public key size: %zu bytes\n", sizeof(mira192f_pk));
    printf("Secret key size: %zu bytes\n", sizeof(mira192f_sk));
    printf("Signature size: %d bytes\n", CRYPTO_BYTES);

    // #ifndef VERBOSE
    // printf("\n MIRA-192F");
    // printf("\n  crypto_sign_keypair: %lld CPU cycles", t2 - t1);
    // printf("\n  crypto_sign:         %lld CPU cycles", t4 - t3);
    // printf("\n  crypto_sign_open:    %lld CPU cycles", t6 - t5);
    // printf("\n\n");

    // printf("\n sk: "); for(int k = 0 ; k < CRYPTO_SECRETKEYBYTES ; ++k) printf("%02x", mira192f_sk[k]);
    // printf("\n pk: "); for(int k = 0 ; k < CRYPTO_PUBLICKEYBYTES ; ++k) printf("%02x", mira192f_pk[k]);
    // printf("\n  m: "); for(int k = 0 ; k < (int)mira192f_mlen ; ++k) printf("%02x", mira192f_m[k]);
    // #endif


    // printf("\n\n");
    return 0;
}

