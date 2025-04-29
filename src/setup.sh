#!/bin/bash

# DO NOT TOUCH THIS
EXPECTED_SHA="77ea9deec2fbbe93fc181c5079081c66d7e1504a"  # Update this with your expected SHA

# Check if SPIKE_SOURCE_DIR is defined
if [ -z "${SPIKE_SOURCE_DIR}" ]; then
    echo "Error: SPIKE_SOURCE_DIR environment variable is not set"
    echo "Please set SPIKE_SOURCE_DIR to point to your Spike repository"
    echo "Example: export SPIKE_SOURCE_DIR=/path/to/riscv-isa-sim"
    exit 1
fi

# Check if SPIKE_SOURCE_DIR exists and is a directory
if [ ! -d "${SPIKE_SOURCE_DIR}" ]; then
    echo "Error: ${SPIKE_SOURCE_DIR} is not a directory"
    exit 1
fi

# Check if files are already patched
if [ -f "${SPIKE_SOURCE_DIR}/.copy_files_stamp" ]; then
    echo "Files are already patched (${SPIKE_SOURCE_DIR}/.copy_files_stamp exists)"
    echo "If you want to patch again, remove ${SPIKE_SOURCE_DIR}/.copy_files_stamp and rerun this script"
    exit 0
fi

# Get current SHA of Spike repository
cd "${SPIKE_SOURCE_DIR}"
CURRENT_SHA=$(git rev-parse HEAD)

# Compare SHAs
if [ "${CURRENT_SHA}" != "${EXPECTED_SHA}" ]; then
    echo "Warning: Spike repository SHA mismatch"
    echo "Expected: ${EXPECTED_SHA}"
    echo "Current:  ${CURRENT_SHA}"
    echo "This might cause compatibility issues"
    read -p "Do you want to continue anyway? (y/N) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo "Continuing with current SHA. You're on your own risk!"
    else
        echo "Please checkout the recommended SHA and try again"
        exit 1
    fi
fi

# Return to original directory
cd - > /dev/null

# info
echo "This script will patch the following files in your Spike repository:"
echo "1. softfloat/softfloat.mk.in"
echo "2. riscv/riscv.mk.in"
echo "3. riscv/mmu.h"
echo "-----------------------------------"

# Copy files from copy-files directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
COPY_FILES_DIR="${SCRIPT_DIR}/copy-files"
if [ ! -d "${COPY_FILES_DIR}" ]; then
    echo "Error: ${COPY_FILES_DIR} directory not found"
    exit 1
fi

# Copy softfloat.mk.in separately
if [ -f "${COPY_FILES_DIR}/softfloat.mk.in" ]; then
    cp "${COPY_FILES_DIR}/softfloat.mk.in" "${SPIKE_SOURCE_DIR}/softfloat/" || {
        echo "Error: Failed to copy softfloat.mk.in"
        exit 1
    }
fi

# Copy all other files to riscv directory
find "${COPY_FILES_DIR}" -type f ! -name 'softfloat.mk.in' -exec cp {} "${SPIKE_SOURCE_DIR}/riscv/" \;
if [ $? -ne 0 ]; then
    echo "Error: Failed to copy files to ${SPIKE_SOURCE_DIR}/riscv/"
    exit 1
fi

echo "Successfully copied files to Spike repository"
echo "Files copied:"
find "${COPY_FILES_DIR}" -type f -exec basename {} \;

# Create a stamp file to mark successful copy
touch "${SPIKE_SOURCE_DIR}/.copy_files_stamp"

echo "Done! You can now build Spike with the copied files"
