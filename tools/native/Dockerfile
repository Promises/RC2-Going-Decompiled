# native-build — image for the TARGET_NATIVE functional-test harness.
# Plain x86 Debian + gcc-multilib so `gcc -m32` produces a runnable ILP32
# (4-byte-pointer) x86 ELF — pointers stay 4 bytes like the PS2/EE, keeping the
# decomp's struct layouts byte-identical. Runs in the same colima ee-x86 VM as
# the MIPS toolchain; built once via tools/native/build_image.sh.
# g++-multilib: .cpp units compile as C++ (cc1plus), which gcc-multilib lacks;
# runtime/arena/verify_link.sh links them (task #1419). Rebuild the image.
FROM debian:bookworm-slim
RUN apt-get update \
 && apt-get install -y --no-install-recommends gcc-multilib g++-multilib libc6-dev-i386 make \
 && rm -rf /var/lib/apt/lists/*
WORKDIR /work
