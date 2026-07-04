## Why

The Windows front-end (WeaselTSF/WeaselUI/WeaselServer/…) can only be built with MSVC, and this
refactor runs on a Linux box with no MSVC/Windows SDK — so every change is currently "blind" to the
compiler. However, a real, load-bearing slice of the codebase is fully portable: the IPC
**text-protocol parser** (`WeaselIPC/ResponseParser.cpp` + its deserializers), the shared **data
model** (`include/WeaselIPCData.h`), and the **boost serialization** of candidates/style. We can
compile and run that slice on Linux today and use it as a regression oracle. Standing this up first
de-risks every later change that touches the IPC/data layer and directly guards the cross-cutting
requirement that the IPC text protocol and boost-archive payloads stay byte-identical.

## What Changes

- Add `test/host/` — a Linux-buildable regression harness that compiles the **real** parser
  translation units against a tiny `windows.h` shim and runs golden tests:
  - `test/host/shim/` — minimal `windows.h` (typedefs, no-op `MessageBoxA`, `_wtoi`) plus
    empty/thin shims for `tchar.h`, `WinSDKVer.h`, `SDKDDKVer.h`, `wrl/client.h`, and a reduced
    `WeaselUtility.h` exposing only the portable `unescape_string`/`EscapeChar` helpers.
  - `test/host/golden_test.cpp` — fresh golden tests covering `action=noop`, `commit` (with
    escape/unescape), `ctx.preedit` + 3-field `cursor`, `status.*`, and a **boost round-trip** of
    `CandidateInfo` via the *current* `ctx.cand=<archive>` protocol.
  - `test/host/run.sh` — builds with the `wtest` conda env (gxx 15 + boost) and runs the tests;
    exit code is the failing-assertion count.
  - `test/host/README.md` — what it is, why it exists, how to run, and its scope limits.
- Add `.gitignore` entries for `test/host/` build outputs.

No shipped/production code is modified. The existing MSVC test project `test/TestResponseParser`
is left untouched (see Non-goals).

## Capabilities

### New Capabilities
- `host-test-harness`: a Linux-buildable oracle that verifies the portable IPC text-protocol
  parser, the shared data model, and boost serialization round-trips without a Windows toolchain.

### Modified Capabilities
- (none — no shipped behavior or requirements change)

## Impact

- New directory `test/host/` (self-contained; not part of the MSVC/xmake Windows build).
- Build/verify: introduces the first automated regression check runnable in this environment.
  Requires the `wtest` conda env (documented in `run.sh`/README); no change to Windows build.
- No changes to `include/`, `WeaselIPC/`, or any shipped module. IPC protocol and archive formats
  are exercised, not altered — they must remain **byte-for-byte identical**.

## Non-goals

- No production code changes — in particular, the latent `ContextUpdater::_StoreText` `vec[2]`
  out-of-bounds guard (`vec.size() < 2` should be `< 3`) is *observed* by this harness but is fixed
  later in the `fix-ipc-parser-field-bugs` change, so this change stays test-only and maximally safe.
- Do not modify or delete the stale `test/TestResponseParser` MSVC project (it uses an obsolete
  candidate protocol); reconciling/removing it belongs to a later dead-code cleanup change, to avoid
  touching an unverifiable Windows build config here.
- No Windows build-system (`weasel.sln`/`xmake.lua`) changes; the harness is Linux-only tooling.
- The shim's copied `unescape_string`/`EscapeChar` is a temporary duplication; extracting those
  portable helpers into `include/StringAlgorithm.hpp` (so the harness includes the real code) is a
  separate follow-up refactor.
