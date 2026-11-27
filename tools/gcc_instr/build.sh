#!/bin/bash
# Build an instrumented cc1 (allocation trace) from the decompals mips-gcc-2.7.2 source that produced
# tools/gcc_2.7.2/cc1, and check that it compiles every unit to the official compiler's bytes.
#
#   tools/gcc_instr/build.sh            fetch v0.1, patch, build into tools/gcc_2.7.2_instr/ (ignored)
#   tools/gcc_instr/build.sh verify     compile every C unit (and NON_MATCHING drafts) with both
#                                       compilers and compare the objects byte for byte
#
# Needs a 32-bit host C toolchain (gcc -m32). Without 32-bit glibc development files on Fedora, the
# script fetches glibc-devel.i686 with `dnf download` (no root) into a private sysroot.
set -euo pipefail

ROOT=$(cd "$(dirname "$0")/../.." && pwd)
OUT=${GCC_INSTR_DIR:-$ROOT/tools/gcc_2.7.2_instr}
REPO=https://github.com/decompals/mips-gcc-2.7.2
TAG=v0.1                      # e6271f1: the release tools/gcc_2.7.2 was downloaded from
PATCH=$ROOT/tools/gcc_instr/allocation-trace.patch
OFFICIAL=$ROOT/tools/gcc_2.7.2

host_cc() {
    # Prints the host compiler command for 32-bit builds, creating a sysroot if needed.
    local t; t=$(mktemp -d)
    echo 'int main(void){return 0;}' > "$t/t.c"
    if gcc -m32 "$t/t.c" -o "$t/t" 2>/dev/null; then
        echo "gcc -m32"
    else
        local sr=$OUT/sysroot
        if [ ! -f "$sr/usr/lib/crt1.o" ]; then
            mkdir -p "$sr" "$OUT/rpms"
            (cd "$OUT/rpms" && dnf download -q glibc-devel.i686) >&2
            (cd "$sr" && rpm2cpio "$OUT"/rpms/glibc-devel-*.i686.rpm | cpio -idm --quiet)
            mkdir -p "$sr/lib"
            ln -sf /usr/lib/libc.so.6 /usr/lib/ld-linux.so.2 /usr/lib/libm.so.6 "$sr/lib/"
            ln -sf /usr/lib/libc.so.6 /usr/lib/ld-linux.so.2 /usr/lib/libm.so.6 "$sr/usr/lib/"
        fi
        echo "gcc -m32 --sysroot=$sr -isystem $sr/usr/include -idirafter /usr/include"
    fi
    rm -rf "$t"
}

build() {
    mkdir -p "$OUT"
    if [ ! -d "$OUT/src/.git" ]; then
        git clone -q "$REPO" "$OUT/src"
    fi
    git -C "$OUT/src" checkout -q "$TAG"
    rm -rf "$OUT/build"
    mkdir -p "$OUT/build"
    git -C "$OUT/src" archive "$TAG" | tar x -C "$OUT/build"
    patch -s -p1 -d "$OUT/build" < "$PATCH"
    local cc; cc=$(host_cc)
    (
        cd "$OUT/build"
        # The release workflow's configure line; generated parsers are committed, so touch them.
        ./configure --target=mips-mips-gnu --prefix=/opt/cross --with-gnu-as --disable-gprof \
            --disable-gdb --disable-werror --host=i386-pc-linux --build=i386-pc-linux > configure.log 2>&1
        touch c-parse.c cp/parse.c cexp.c
        make -j"$(nproc)" CC="$cc" \
            CFLAGS="-std=gnu89 -m32 -fcommon -fpermissive -w -D_FILE_OFFSET_BITS=64" cc1 > make.log 2>&1
    )
    # A compiler directory like tools/gcc_2.7.2: the official driver, cpp and as, with the traced cc1.
    mkdir -p "$OUT/bin"
    for f in "$OFFICIAL"/*; do ln -sf "$f" "$OUT/bin/"; done
    ln -sf "$OUT/build/cc1" "$OUT/bin/cc1"
    echo "built $OUT/bin/cc1"
}

verify() {
    # Every C compile of `make` (game units and ultralib), rerun with the traced cc1; whole objects compared.
    [ -x "$OUT/bin/cc1" ] || { echo "no $OUT/bin/cc1: run build first"; exit 1; }
    cd "$ROOT"
    make -s >/dev/null
    local tmp; tmp=$(mktemp -d)
    local n=0 bad=0 nm=0
    while read -r line; do
        local obj src
        obj=$(sed -E 's/.* -o (build\/[^ ]+\.o) .*/\1/' <<< "$line")
        src=$(grep -o 'src/[^ ]*\.c$' <<< "$line")
        mkdir -p "$tmp/$(dirname "$obj")"
        eval "$(sed -e "s#tools/gcc_2.7.2#$OUT/bin#g" -e "s# -o build/# -o $tmp/build/#" <<< "$line")" 2>/dev/null
        n=$((n + 1))
        cmp -s "$obj" "$tmp/$obj" || { echo "DIFF $obj"; bad=$((bad + 1)); }
        if grep -q 'ifdef NON_MATCHING' "$src"; then
            local a b
            a=$(sed -e 's# -c # -DNON_MATCHING -c #' -e "s# -o build/[^ ]*# -o $tmp/nm_a.o#" <<< "$line")
            b=$(sed -e "s#tools/gcc_2.7.2#$OUT/bin#g" -e 's#nm_a.o#nm_b.o#' <<< "$a")
            eval "$a" 2>/dev/null; eval "$b" 2>/dev/null
            nm=$((nm + 1))
            cmp -s "$tmp/nm_a.o" "$tmp/nm_b.o" || { echo "DIFF $src (NON_MATCHING)"; bad=$((bad + 1)); }
        fi
    done < <(make -n -B 2>/dev/null | grep -E '^COMPILER_PATH=tools/gcc_2.7.2 .* src/[^ ]*\.c$')
    # ultralib, built by its own Makefile into a scratch tree
    local rel; rel=$(realpath -m --relative-to="$ROOT/lib/ultralib" "$tmp/ultra")
    make -s --no-print-directory -C lib/ultralib TARGET=libgultra_rom VERSION=K COMPARE=0 \
        CROSS="${CROSS:-mips64-linux-gnu-}" COMPILER_DIR="$OUT/bin" MDEBUG_FILES= BUILD_ROOT="$rel" \
        -j"$(nproc)" > "$tmp/ultra.log" 2>&1
    local u=0
    while read -r f; do
        u=$((u + 1))
        cmp -s "build/ultralib/K/libgultra_rom/$f" "$tmp/ultra/K/libgultra_rom/$f" || { echo "DIFF ultralib $f"; bad=$((bad + 1)); }
    done < <(cd build/ultralib/K/libgultra_rom && find src -name '*.o')
    rm -rf "$tmp"
    echo "units=$n NON_MATCHING=$nm ultralib=$u differ=$bad"
    [ "$bad" -eq 0 ]
}

case "${1:-build}" in
    build) build ;;
    verify) verify ;;
    *) echo "usage: $0 [build|verify]"; exit 2 ;;
esac
