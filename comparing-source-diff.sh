#!/bin/bash

# Define the paths for the reference and optimized implementations
REF_IMPL_PATH="Reference_Implementation"
OPT_IMPL_PATH="Optimized_Implementation"

# List of MIRA variants
VARIANTS=("MIRA-128f" "MIRA-128s" "MIRA-192f" "MIRA-192s" "MIRA-256f" "MIRA-256s")

# Loop through each MIRA variant and perform diff for each source file
for variant in "${VARIANTS[@]}"; do
  echo "Comparing $variant"
  
  # Get the source files in the reference and optimized implementations
  REF_SRC_PATH="$REF_IMPL_PATH/$variant/src"
  OPT_SRC_PATH="$OPT_IMPL_PATH/$variant/src"

  # Loop through each file in the reference source folder and compare with the corresponding file in the optimized folder
  for ref_file in "$REF_SRC_PATH"/*.c "$REF_SRC_PATH"/*.h; do
    # Get the base name of the reference file
    base_name=$(basename "$ref_file")

    if [ "$base_name" == "PQCgenKAT_sign.c" ]; then
	    echo "Skipping $base_name"
	    continue
    fi
    # Define the corresponding optimized file
    opt_file="$OPT_SRC_PATH/$base_name"

    # Check if the optimized file exists before comparing
    if [ -f "$opt_file" ]; then
      echo "Diff between $ref_file and $opt_file:"
      diff "$ref_file" "$opt_file"
    else
      echo "Optimized file $opt_file not found, skipping."
    fi
  done
done

