#!/usr/bin/env bash
# Build and run Weasel's portable host tests on Linux (no MSVC). Three suites:
#   1) IPC text-protocol parser golden tests (needs the windows.h shim + Boost),
#   2) input-position codec equivalence (pure portable, no deps),
#   3) label-format safety (format_label; pure portable, no deps).
# See test/host/README.md. Exit code = total failing checks across suites (0 = all pass).
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

fail=0

# --- Suite 1: IPC parser golden tests -----------------------------------------
PARSER_SRC=(
  "$REPO/WeaselIPC/ResponseParser.cpp"
  "$REPO/WeaselIPC/Deserializer.cpp"
  "$REPO/WeaselIPC/ActionLoader.cpp"
  "$REPO/WeaselIPC/ContextUpdater.cpp"
  "$REPO/WeaselIPC/Committer.cpp"
  "$REPO/WeaselIPC/Configurator.cpp"
  "$REPO/WeaselIPC/Styler.cpp"
)
GOLDEN="$HERE/golden_test"
echo "[host-test] building parser golden_test ..."
"$CXX" -std=c++17 -O0 -w -D_GLIBCXX_ASSERTIONS \
  -I "$HERE/shim" -I "$REPO/include" -I "$CONDA_PREFIX/include" \
  "$HERE/golden_test.cpp" "${PARSER_SRC[@]}" \
  -L "$CONDA_PREFIX/lib" -lboost_wserialization -lboost_serialization \
  -Wl,-rpath,"$CONDA_PREFIX/lib" \
  -o "$GOLDEN"
if "$GOLDEN" < /dev/null; then
  echo "[host-test] golden_test PASS"
else
  rc=$?; echo "[host-test] golden_test FAIL ($rc)"; fail=$((fail + rc))
fi

# --- Suite 2: input-position codec equivalence (pure portable) ----------------
CODEC="$HERE/input_position_test"
echo "[host-test] building input_position_test ..."
"$CXX" -std=c++17 -O0 -w -D_GLIBCXX_ASSERTIONS \
  -I "$REPO/include" \
  "$HERE/input_position_test.cpp" \
  -o "$CODEC"
if "$CODEC" < /dev/null; then
  echo "[host-test] input_position_test PASS"
else
  rc=$?; echo "[host-test] input_position_test FAIL ($rc)"; fail=$((fail + rc))
fi

# --- Suite 3: label-format safety (pure portable) -----------------------------
LABELFMT="$HERE/format_label_test"
echo "[host-test] building format_label_test ..."
"$CXX" -std=c++17 -O0 -w -D_GLIBCXX_ASSERTIONS \
  -I "$REPO/include" \
  "$HERE/format_label_test.cpp" \
  -o "$LABELFMT"
if "$LABELFMT" < /dev/null; then
  echo "[host-test] format_label_test PASS"
else
  rc=$?; echo "[host-test] format_label_test FAIL ($rc)"; fail=$((fail + rc))
fi

# --- Summary ------------------------------------------------------------------
if [ "$fail" -eq 0 ]; then
  echo "[host-test] ALL PASS"
else
  echo "[host-test] FAIL ($fail failing check(s))"
  exit "$fail"
fi
