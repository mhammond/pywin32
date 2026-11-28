#!/usr/bin/env bash
exec wine "$(dirname "$0")/swig.exe" "$@"
