#!/bin/bash

# Check if the TARGET_APP environment variable is set
if [ -z "$TARGET_APP" ]; then
  echo "TARGET_APP environment variable is not set. Exiting."
  current_directory=$(pwd)
  echo "Current directory: $current_directory"
  echo "List of files and directories:"
  ls -a
  exit 1
fi

# Check if the target executable exists and is executable
if [ -x "/chat_server/bin/$TARGET_APP" ]; then
  echo "Running /chat_server/bin/$TARGET_APP with arguments: $@"
  current_directory=$(pwd)
  echo "Current directory: $current_directory"
  echo "List of files and directories:"
  ls -a
  /chat_server/bin/"$TARGET_APP" "$@" # Execute the target with additional arguments
else
  echo "Target $TARGET_APP does not exist or is not executable. Exiting."
  current_directory=$(pwd)
  echo "Current directory: $current_directory"
  echo "List of files and directories:"
  ls -a
  exit 1
fi
