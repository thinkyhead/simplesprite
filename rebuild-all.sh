#!/usr/bin/env zsh
#
# rebuild-all.sh — SimpleSprite engine.
# Builds all variant configs under build/.  Uses `cmake --build --parallel`
# for speed and avoids `rm -rf build` (let cmake handle staleness).
#
# Usage:
#   ./rebuild-all.sh              # all variants
#   ./rebuild-all.sh release      # release variants only
#   ./rebuild-all.sh -DSS_PHYSICS_ENABLE=0  # override any flag

set -e
SDK="${0:A:h}"

filter="${1:-}"

# Build configs: name  build_type  extra_cmake_flags
while read name btype extra; do
  [[ "$filter" == "release" && "$name" == debug* ]] && continue
  echo "--- SimpleSprite/$name ---"
  bdir="$SDK/build/$name"
  cmake -S "$SDK" -B "$bdir" -DCMAKE_BUILD_TYPE="$btype" $extra
  cmake --build "$bdir" --parallel
done <<EOF
release      Release
debug        Debug
release-box2d Release    -DSS_PHYSICS_ENABLE=1
debug-box2d  Debug       -DSS_PHYSICS_ENABLE=1
EOF
echo "=== SimpleSprite done ==="
