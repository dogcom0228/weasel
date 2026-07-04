#!/usr/bin/env bash
# Build and run Weasel's portable IPC-parser golden tests on a Linux host (no MSVC).
# See test/host/README.md. Exit code = number of failing assertions (0 = all pass).
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO="$(cd "$HERE/../.." && pwd)"
ENV_NAME="${WEASEL_TEST_ENV:-wtest}"

# Activate the conda env that provides g++ and Boost (serialization).
if command -v conda >/dev/null 2>&1; then
  # shellcheck disable=SC1091
  source "$(conda info --base)/etc/profile.d/conda.sh"
else
  for base in "$HOME/miniforge3" "$HOME/miniconda3" "$HOME/anaconda3"; do
    if [ -f "$base/etc/profile.d/conda.sh" ]; then
      # shellcheck disable=SC1091
      source "$base/etc/profile.d/conda.sh"
      break
    fi
  done
fi
conda activate "$ENV_NAME" 2>/dev/null || {
  echo "error: conda env '$ENV_NAME' not found." >&2
  echo "bootstrap it once with: mamba create -n $ENV_NAME gxx_linux-64 libboost-devel" >&2
  exit 127
}

CXX="${CXX:-x86_64-conda-linux-gnu-g++}"
command -v "$CXX" >/dev/null 2>&1 || CXX=g++

PARSER_SRC=(
  "$REPO/WeaselIPC/ResponseParser.cpp"
  "$REPO/WeaselIPC/Deserializer.cpp"
  "$REPO/WeaselIPC/ActionLoader.cpp"
  "$REPO/WeaselIPC/ContextUpdater.cpp"
  "$REPO/WeaselIPC/Committer.cpp"
  "$REPO/WeaselIPC/Configurator.cpp"
  "$REPO/WeaselIPC/Styler.cpp"
)

BIN="$HERE/golden_test"
echo "[host-test] compiling with $CXX ..."
"$CXX" -std=c++17 -O0 -w -D_GLIBCXX_ASSERTIONS \
  -I "$HERE/shim" -I "$REPO/include" -I "$CONDA_PREFIX/include" \
  "$HERE/golden_test.cpp" "${PARSER_SRC[@]}" \
  -L "$CONDA_PREFIX/lib" -lboost_wserialization -lboost_serialization \
  -Wl,-rpath,"$CONDA_PREFIX/lib" \
  -o "$BIN"

echo "[host-test] running ..."
if "$BIN" < /dev/null; then
  echo "[host-test] PASS"
else
  rc=$?
  echo "[host-test] FAIL ($rc failing assertion(s))"
  exit "$rc"
fi
