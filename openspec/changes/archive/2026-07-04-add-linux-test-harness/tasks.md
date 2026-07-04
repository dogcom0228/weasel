## 1. Shim layer

- [x] 1.1 Create `test/host/shim/windows.h` with the Win32 typedefs used by the parse path
      (`WCHAR/LPWSTR/LPCWSTR/UINT/UINT32/UINT16/DWORD/BOOL/LONG/BYTE/LPBYTE/LPARAM/WPARAM/LRESULT/
      HWND/HANDLE/HKEY/WORD/SHORT/USHORT`, `RECT`), `NULL/TRUE/FALSE`, `WM_APP`, `MB_OK`,
      `MB_ICONERROR`, a no-op `MessageBoxA`, and `_wtoi` via `wcstol`.
- [x] 1.2 Create empty/thin shims: `test/host/shim/tchar.h` (`_TCHAR`, `#define _tmain main`),
      `WinSDKVer.h`, `SDKDDKVer.h`, `wrl/client.h`.
- [x] 1.3 Create `test/host/shim/WeaselUtility.h` exposing only `EscapeChar`, `unescape_string`
      (copied verbatim from the real header) and a stub `getUsername`; `#include`s the real portable
      `StringAlgorithm.hpp`. Add a comment marking the copy as temporary pending helper extraction.

## 2. Golden tests

- [x] 2.1 Create `test/host/golden_test.cpp` with a plain `int main()` using boost
      `lightweight_test`, covering: `action=noop`; `commit` with `\n` escape/unescape; `ctx.preedit`
      + three-field `ctx.preedit.cursor`; `status.ascii_mode`/`composing`; and a `CandidateInfo`
      boost round-trip via `ctx.cand=<text_woarchive blob>`.
- [x] 2.2 Ensure assertions check reconstructed fields (candidate strings, highlight, pages,
      cursor start/end/cursor, unescaped commit) and that `main` returns `boost::report_errors()`.

## 3. Build + run script

- [x] 3.1 Create `test/host/run.sh` that activates the `wtest` conda env, compiles
      `golden_test.cpp` + the 7 WeaselIPC parser `.cpp`s with `-I test/host/shim -I include
      -I $CONDA_PREFIX/include -std=c++17 -D_GLIBCXX_ASSERTIONS`, links
      `-lboost_wserialization -lboost_serialization` with rpath, runs the binary with stdin from
      `/dev/null`, and propagates its exit code. Make it executable.
- [x] 3.2 Create `test/host/README.md`: purpose, scope limits (portable layer only), the one-line
      `mamba create -n wtest gxx_linux-64 libboost-devel` bootstrap, and how to run.
- [x] 3.3 Add `.gitignore` entries for `test/host/` build outputs (compiled binary, any `*.o`).

## 4. Verify, review, commit

- [x] 4.1 Run `test/host/run.sh`; confirm it builds and all golden assertions pass (exit 0). Capture
      the output as evidence.
- [x] 4.2 Deliberately perturb one parser detail in a scratch copy (not committed) to confirm the
      harness turns red, proving it is a real oracle; revert.
- [x] 4.3 Spawn an adversarial review agent on the full diff (shim faithfulness, no shipped-file
      edits, protocol coverage). Address blocking findings.
- [x] 4.4 Commit on `master` with a `test(host):` conventional-commit message + Co-Authored-By
      trailer; then `openspec archive add-linux-test-harness`.
