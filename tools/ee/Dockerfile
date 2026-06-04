# Runs the SN ProDG ee-gcc 2.95.2 (i386 Win32 PE) toolchain to compile/match
# Going Commando. The i386 PEs are loaded by wibo; because Apple Rosetta cannot
# execute wibo's i386 segment thunks, we emulate i386 with qemu-user and the
# 32-bit wibo-i686. The container is amd64 (Rosetta) but the inner compiler runs
# under qemu-i386.
FROM --platform=linux/amd64 debian:bookworm-slim

RUN apt-get update \
 && apt-get install -y --no-install-recommends \
      ca-certificates curl binutils qemu-user libc6-i386 \
 && rm -rf /var/lib/apt/lists/*

# wibo (x86_64 ELF) loads the i386 Win32 PEs. Run it under qemu-x86_64 so the
# PE's 32-bit segment thunks are emulated in software (Rosetta can't do them).
RUN curl -fsSL https://github.com/decompals/wibo/releases/download/1.1.0/wibo-x86_64 \
      -o /usr/local/bin/wibo-x86_64 \
 && chmod +x /usr/local/bin/wibo-x86_64

RUN printf '#!/bin/sh\nexec qemu-x86_64 /usr/local/bin/wibo-x86_64 "$@"\n' > /usr/local/bin/wibo \
 && chmod +x /usr/local/bin/wibo

WORKDIR /work
