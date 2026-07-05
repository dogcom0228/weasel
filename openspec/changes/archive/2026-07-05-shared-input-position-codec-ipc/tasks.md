## 1. Portable codec + host test

- [x] 1.1 Add `include/InputPositionCodec.h` (`InputPosition` struct + `EncodeInputPosition` /
      `DecodeInputPosition`), documenting the packed 32-bit layout; unsigned-domain packing to avoid
      the original's signed-shift UB while staying bit-identical.
- [x] 1.2 Add `test/host/input_position_test.cpp` with reference encode/decode copied verbatim from
      the originals and an equivalence sweep; add its binary to `.gitignore`.
- [x] 1.3 Update `test/host/run.sh` to build+run both suites and aggregate exit codes.

## 2. Wire codec into the call sites (byte-preserving; both files CRLF)

- [x] 2.1 `WeaselClientImpl.cpp`: `#include <InputPositionCodec.h>`; replace the inline encode with
      `EncodeInputPosition({rc.left, rc.top, rc.bottom - rc.top})`.
- [x] 2.2 `WeaselServerImpl.cpp`: `#include <InputPositionCodec.h>`; replace the inline decode with
      `DecodeInputPosition(wParam)` + the same rect reconstruction (`right=left+6`,
      `bottom=top+height`); DPI transform unchanged.

## 3. Verify, review, commit

- [x] 3.1 Run `test/host/run.sh` — both suites pass (codec equivalence proves the wire word is
      byte-identical). Confirm `git diff --stat` small and CRLF preserved.
- [x] 3.2 Adversarial review agent: confirm the codec is bit-identical to the originals (esp. sign
      extension + hi_res clamping), the call sites feed/consume the exact same values as before, no
      `.vcxproj` edit is needed, and line endings intact. Address blocking findings.
- [x] 3.3 Commit on `master` (`refactor(WeaselIPC):`) + Co-Authored-By; then
      `openspec archive shared-input-position-codec-ipc` (spec delta present — do NOT skip specs).
