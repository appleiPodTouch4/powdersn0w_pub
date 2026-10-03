#!/bin/bash
#
# Build the Windows (MSYS2 / MinGW-w64) binaries for powdersn0w.
#
# Run from an MSYS2 shell with the mingw64 toolchain on PATH, e.g.
#     C:/msys64/usr/bin/bash -lc 'cd /c/path/to/xpwn && ./compile-windows.sh'
#
# Required mingw64 packages: gcc, make (or ninja), cmake, zlib, libpng, bzip2,
# openssl. Everything is linked statically, so the resulting .exe files have no
# dependency other than the standard Windows system DLLs.
#
# Output: bin/ with powdersn0w.exe, ticket.exe, validate.exe, hdutil.exe,
# hfsplus.exe plus the FirmwareBundles/DaibutsuBundles the tools look up
# relative to the working directory.
#
# NOTE: unlike the macOS/Linux builds this one applies the wincompat/ shims
# (fnmatch, memmem) and the plist parser fixes in ipsw-patch/plist.c - see the
# comments there.

set -e

cd "$(dirname "$0")"

# --- environment checks ------------------------------------------------------
if [[ "$(uname -s)" != MINGW* && "$(uname -s)" != MSYS* ]]; then
    echo "[Error] Building Windows binaries requires MSYS2 (found: $(uname -s))"
    exit 1
fi

for tool in cmake gcc strip; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "[Error] '$tool' not found in PATH."
        echo "        Open the 'MSYS2 MINGW64' shell, or add C:/msys64/mingw64/bin to PATH."
        exit 1
    fi
done

if [[ "$(gcc -dumpmachine)" != "x86_64-w64-mingw32" ]]; then
    echo "[Error] expected the mingw64 (x86_64-w64-mingw32) toolchain, got: $(gcc -dumpmachine)"
    exit 1
fi

# --- mach-o headers ----------------------------------------------------------
# asr.c / iboot.c / kernel.c include <mach-o/loader.h>. The macOS/Linux build
# downloads cctools and installs the header into /usr/local/include; on Windows
# we keep a local copy under wincompat/ (which is on the include path) so the
# build stays self-contained and never touches the system.
if [[ ! -f wincompat/mach-o/loader.h ]]; then
    echo "* mach-o/loader.h missing, fetching it from cctools-927.0.2"
    cctools_url=https://opensource.apple.com/tarballs/cctools/cctools-927.0.2.tar.gz
    tmpdir=$(mktemp -d)
    trap 'rm -rf "$tmpdir"' EXIT

    curl -fsSL -o "$tmpdir/cctools.tar.gz" "$cctools_url" || {
        echo "[Error] could not download $cctools_url"
        exit 1
    }
    tar xzf "$tmpdir/cctools.tar.gz" -C "$tmpdir"

    mkdir -p wincompat/mach-o
    cp "$tmpdir"/*cctools-927.0.2/include/mach-o/loader.h wincompat/mach-o/loader.h

    # Same transformation compile.sh applies for Linux: drop the Apple-only
    # includes, then reintroduce <stdint.h> and the types the remaining
    # declarations rely on.
    sed -i "s_#include_//_g" wincompat/mach-o/loader.h
    sed -i -e "s=<stdint.h>=\n#include <stdint.h>\ntypedef int integer_t;\ntypedef integer_t cpu_type_t;\ntypedef integer_t cpu_subtype_t;\ntypedef integer_t cpu_threadtype_t;\ntypedef int vm_prot_t;=g" wincompat/mach-o/loader.h

    echo "* mach-o/loader.h: done"
fi

# --- build -------------------------------------------------------------------
# CMake 4.x refuses cmake_minimum_required(<3.5); the upstream files still
# declare 2.8.12, so raise the floor instead of touching every CMakeLists.
#
# Ninja is preferred: with an sh.exe on PATH CMake picks "Unix Makefiles" and
# drives it through MSYS make, which mangles some paths. Both are fine, Ninja
# is just less surprising.
if command -v ninja >/dev/null 2>&1; then
    generator=(-G Ninja)
else
    generator=(-G "MinGW Makefiles")
fi

rm -rf new bin
mkdir -p new bin

echo "* Configuring (${generator[*]})"
(
    cd new
    cmake .. \
        "${generator[@]}" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5
)

echo "* Compiling"
cmake --build new --parallel

echo "* Collecting binaries"
cp new/ipsw-patch/ipsw.exe bin/powdersn0w.exe
cp new/ipsw-patch/ticket.exe bin/ticket.exe
cp new/ipsw-patch/validate.exe bin/validate.exe
cp new/hdutil/hdutil.exe bin/hdutil.exe
cp new/hfs/hfsplus.exe bin/hfsplus.exe

strip bin/*.exe 2>/dev/null || true

# The tools resolve FirmwareBundles/ and DaibutsuBundles/ relative to the
# current directory, so ship them next to the executables.
cp -r FirmwareBundles DaibutsuBundles bin/
cp LICENSE bin/LICENSE.txt

echo
echo "Done! Builds at bin/"
ls -la bin/*.exe
