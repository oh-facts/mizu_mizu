# valid args: clean, debug, release, clang, gcc 

cd "$(dirname "$0")"

for arg in "$@"; do declare $arg='1'; done

[ "$clean" == "1" ] && rm -f yk && exit 0

debug_build="-g -O0"
release_build="-O2"
compiler="gcc"
out=""
build_type="$debug_build"
platform_flags=""
[ "$release" == "1" ] && build_type="$release_build"
[ "$clang" == "1" ] && compiler="clang"

echo "[$compiler]"
echo "[$build_type]"

gfx_lib=""

if [ "$(uname)" == "Linux" ]; then
    echo "[Linux]"
    gfx_lib="-lX11"
    out="yk"
elif [ "$(uname)" == "Darwin" ]; then
    echo [Darwin]
    out="contents/MacOS/yk.app"
    platform_flags="-x objective-c"
    gfx_lib="-framework AppKit -framework Foundation"
else
    echo "Unsupported OS: $(uname)"
    exit 1
fi

$compiler $platform_flags -Wall -Wextra -Wno-unused-function -Wno-format -Wno-int-conversion -Wno-sign-compare -Wno-unused-parameter -std=c99 -D_GNU_SOURCE $build_type ./main.c -o $out -lm $gfx_lib