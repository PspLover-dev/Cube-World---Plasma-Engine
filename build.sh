#!/bin/bash
set -e
export PATH="/mingw64/bin:$PATH"
export TMP="/home/camil/tmp"
mkdir -p "$TMP"
cd "$(dirname "$0")"

if [ ! -f third_party/sqlite3/sqlite3.c ]; then
  echo "Setting up sqlite3 amalgamation..."
  mkdir -p third_party/sqlite3
  SRC="../CUBE WORLD CODE/third_party/sqlite3/sqlite-amalgamation-3460100"
  if [ -f "$SRC/sqlite3.c" ]; then
    cp "$SRC/sqlite3.c" "$SRC/sqlite3.h" "$SRC/sqlite3ext.h" third_party/sqlite3/
  else
    echo "Missing sqlite3 — install from CUBE WORLD CODE/third_party" >&2
    exit 1
  fi
fi

make -j$(nproc)
echo "Run: ./plasma_test.exe ."
