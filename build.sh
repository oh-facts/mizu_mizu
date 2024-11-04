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
frameworks=""
[ "$release" == "1" ] && build_type="$release_build"
[ "$clang" == "1" ] && compiler="clang"

echo "[$compiler]"
echo "[$build_type]"
echo "OS: $(uname)"
if [[ "$(uname -s)" == "Darwin" ]]; then
  platform_flags="-x objective-c"
  frameworks="-framework Cocoa"
fi

$compiler -Wall -Wextra -Wno-unused-function -Wno-format -Wno-int-conversion -Wno-incompatible-pointer-types -Wno-sign-compare -Wno-unused-parameter -std=c99 -D_GNU_SOURCE $build_type $platform_flags -I. ./main.c -o ./yk -lm  -lSDL2 $frameworks
