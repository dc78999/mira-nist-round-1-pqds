CC=aarch64-linux-gnu-gcc
WARNING_FLAGS=-Wall -Wextra -Wpedantic -Wno-unused-function
CFLAGS= -O3 -march=armv8-a $(WARNING_FLAGS) -std=gnu99 -I/${PAPI_DIR}/include
ASMFLAGS := -x assembler-with-cpp -Wa,-defsym,old_gas_syntax=1 -Wa,-defsym,no_plt=1

PLATFORM=armv8

COMMON_SOURCES=KeccakHash.c KeccakSponge.c
COMMON_OBJECTS=KeccakHash.o KeccakSponge.o 

ifeq ($(PLATFORM), opt64)
	SPECIFIC_SOURCES=opt64/KeccakP-1600-opt64.c opt64/KeccakP-1600-times4-on1.c
	SPECIFIC_OBJECTS=opt64/KeccakP-1600-opt64.o opt64/KeccakP-1600-times4-on1.o
endif
ifeq ($(PLATFORM), avx2)
	SPECIFIC_SOURCES=avx2/KeccakP-1600-AVX2.s avx2/KeccakP-1600-times4-SIMD256.c
	SPECIFIC_OBJECTS=avx2/KeccakP-1600-AVX2.o avx2/KeccakP-1600-times4-SIMD256.o
endif
ifeq ($(PLATFORM), armv8)
	SPECIFIC_SOURCES=ARMv8A/KeccakP-1600-armv8a-neon.s ARMv8A/KeccakP-1600-SnP.h
	SPECIFIC_OBJECTS=ARMv8A/KeccakP-1600-armv8a-neon.o
endif

SOURCES=$(COMMON_SOURCES) $(SPECIFIC_SOURCES)
OBJECTS=$(COMMON_OBJECTS) $(SPECIFIC_OBJECTS)
SHA3LIB=libshake.a

%.o : %.c
	$(CC) -c $(CFLAGS) -I. -I./$(PLATFORM) $< -o $@

all: $(SOURCES) $(SHA3LIB)

$(SHA3LIB): $(OBJECTS)
	ar rcs $@ $^

%.o : %.s
#	as -o $@ $< -Dno_type -Dno_size -Dadd_underscore -Dno_plot
	$(CC) -c $(ASMFLAGS) $< -o $@

#.c.o: 
#	    $(CC) -c $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm *.o $(PLATFORM)/*.o 2>/dev/null || true
	rm $(SHA3LIB) 2>/dev/null || true
