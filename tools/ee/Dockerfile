# Runs the SN ProDG ee-gcc (2.9-ee-991111 / ProDG v2.73a, i386 Win32 PE) to
# compile & match Going Commando. Build/run this image inside a REAL x86_64
# Linux machine (the colima `ee-x86` VM: `colima start ee-x86 --arch x86_64
# --vm-type qemu`). wibo runs the i386 PEs natively there.
#
# NOTE: this does NOT work under Docker Desktop's amd64 emulation on Apple
# Silicon — Rosetta can't run 32-bit x86, and qemu-user segfaults on wibo's PE
# segment thunks. A full x86 machine (qemu-system via colima) is required.
FROM debian:bookworm-slim

# binutils (host) for objcopy; binutils-mips-linux-gnu = modern GNU as/ld that
# understands splat/spimdisasm output and the R5900 (-march=r5900). The period
# ee-gcc is used ONLY to compile C; assembling/linking uses these.
RUN apt-get update \
 && apt-get install -y --no-install-recommends \
      ca-certificates curl binutils binutils-mips-linux-gnu make \
 && rm -rf /var/lib/apt/lists/*

# wibo: i386 Win32 PE loader (runs natively on x86_64).
RUN curl -fsSL https://github.com/decompals/wibo/releases/download/1.1.0/wibo-x86_64 \
      -o /usr/local/bin/wibo \
 && chmod +x /usr/local/bin/wibo

WORKDIR /work
