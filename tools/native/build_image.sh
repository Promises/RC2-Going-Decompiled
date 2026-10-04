#!/usr/bin/env bash
# build_image.sh — build the `native-build` Docker image (gcc-multilib, clang-14) in the
# colima ee-x86 VM. One-time setup for the Tier 2 functional-test runner.
set -eu
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
CTX="${DOCKER_CONTEXT:-colima-ee-x86}"
exec docker --context "$CTX" build -t "${NATIVE_IMG:-native-build}" "$ROOT/tools/native"
