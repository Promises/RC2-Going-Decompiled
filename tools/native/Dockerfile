# native-build — image for the TARGET_NATIVE functional-test harness.
# Plain x86 Debian + gcc-multilib so `gcc -m32` produces a runnable ILP32
# (4-byte-pointer) x86 ELF — pointers stay 4 bytes like the PS2/EE, keeping the
# decomp's struct layouts byte-identical. Runs in the same colima ee-x86 VM as
# the MIPS toolchain; built once via tools/native/build_image.sh.
# gcc-multilib + libc6-dev-i386: run_test.sh, run_state_native.sh,
# run_state_batch.sh and runtime/link_native.sh call `gcc` in this image, and
# clang -m32 links against the same 32-bit crt/libgcc.
# g++-multilib: run_state_batch.sh and link_native.sh hand every TARGET_NATIVE
# unit, .cpp included, to the `gcc` driver, which runs cc1plus on a .cpp; keep it.
# clang-14: runtime/arena/verify_link.sh's docker arm compiles with clang, as its
# host arm and check.sh do. g++ 12 at -std=gnu++98 rejects the `_Static_assert`
# the .cpp units carry; clang accepts it as an extension (task #1445). The major
# is PINNED because the clang version moves the verdict (#1421: Apple clang 17
# compiled all 29 linkgap units, clang 23 dropped 8). The symlink keeps the pin
# here, so `clang` in a script means clang-14.
FROM debian:bookworm-slim
RUN apt-get update \
 && apt-get install -y --no-install-recommends gcc-multilib g++-multilib libc6-dev-i386 make clang-14 \
 && ln -s /usr/bin/clang-14 /usr/local/bin/clang \
 && rm -rf /var/lib/apt/lists/*
WORKDIR /work
