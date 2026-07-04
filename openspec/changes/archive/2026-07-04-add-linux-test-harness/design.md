## Context

Weasel builds only under MSVC on Windows; this refactor runs on Linux with no Windows toolchain, so
the shipped modules cannot be compiled or run here. The refactor's safety therefore rests on
adversarial review and on any code we *can* actually execute. A meaningful subset is portable:

- `include/WeaselIPCData.h` — the shared data model (`Text`, `Context`, `CandidateInfo`, `Status`,
  `Config`, `UIStyle`) depends only on `<string>`, `<vector>`, and boost serialization headers.
- `WeaselIPC/ResponseParser.cpp` + `Deserializer.cpp` + `ActionLoader/ContextUpdater/Committer/
  Configurator/Styler` — the client-side parser of the server's `key=value\n` IPC text protocol,
  including boost `text_wiarchive` decoding of `ctx.cand` / `style`.

These `.cpp`s `#include <windows.h>` (via `WeaselIPC.h`) but only for typedefs (`LPWSTR`, `UINT`,
`DWORD`, …) and one `MessageBoxA` error path — none of which the parse logic actually depends on. A
small shim makes them compile and run on Linux g++/boost.

## Goals / Non-Goals

**Goals:**
- A one-command Linux harness (`test/host/run.sh`) that compiles the real parser TUs and runs golden
  tests, exiting non-zero on any failure — usable by later changes and by the pre-commit review.
- Golden tests that exercise the **current** protocol, including the boost-archive candidate path
  (`ctx.cand=<archive>`), so the harness protects protocol/format byte-equivalence.
- Zero changes to shipped code and zero changes to the Windows build.

**Non-Goals:**
- Compiling the non-portable modules (TSF/UI/Server) — impossible here and out of scope.
- Fixing the observed `_StoreText` `vec[2]` guard (deferred to `fix-ipc-parser-field-bugs`).
- Touching the stale MSVC `test/TestResponseParser` project (deferred to a cleanup change).

## Decisions

- **Shim `windows.h` instead of cross-compiling.** We only need typedefs + `_wtoi` + a no-op
  `MessageBoxA`. Cross-compiling with the real Windows SDK/ATL/WTL is infeasible and unnecessary for
  the parse logic. Alternative (mingw) rejected: still needs Win headers and wouldn't run the boost
  wide-archive path any more faithfully.
- **Compile the REAL parser `.cpp`s, not a reimplementation.** The harness's value is that it tests
  shipped logic. We add the WeaselIPC parser sources to the build verbatim.
- **Reduced `WeaselUtility.h` shim exposing only `unescape_string`/`EscapeChar`.** The real
  `WeaselUtility.h` pulls in `wrl/client.h`, `RegGetValue`, `FormatMessage`, etc. (Win32-only). The
  parser path uses only the portable string helpers from it, so the shim provides just those. The
  helper bodies are copied verbatim from the real header; this duplication is called out for removal
  once those helpers are extracted into the already-portable `include/StringAlgorithm.hpp`.
- **Golden tests use boost `text_woarchive` to synthesize server output**, mirroring
  `RimeWithWeasel::_Respond`, then assert the parser reconstructs the data. This tests the exact
  wire format both directions. Empirically a small `CandidateInfo` archive is single-line, so it
  survives the parser's line-based splitting.
- **conda `wtest` env for the toolchain** (gxx 15 + boost 1.9x), since apt needs interactive sudo on
  this box. `run.sh` activates it and links `-lboost_wserialization -lboost_serialization`.

## Risks / Trade-offs

- [Shim diverges from real `windows.h` semantics — e.g. `DWORD` width] → We use fixed-width
  `uint32_t` to match Win32 `DWORD`; the parse logic is width-agnostic (text in, structs out). The
  boost archive path uses portable types. Risk is low and confined to test tooling.
- [Copied `unescape_string` drifts from the real one] → Called out as a temporary duplication with a
  planned follow-up to extract the real helpers into `StringAlgorithm.hpp`; until then a reviewer
  must keep them in sync (small, rarely-changed function).
- [Harness proves *nothing* about non-portable modules] → Accepted and documented; those rely on
  review + byte-equivalence. The harness is scoped to the portable IPC/data/serialization layer.
- [Env dependency (`wtest`) not reproducible on a fresh machine] → `run.sh`/README document the
  one-line `mamba create` to recreate it; failure is loud (build error), never a silent false pass.

## Verification

This change is itself verified by running `test/host/run.sh` and observing all golden assertions
pass (exit 0), plus review of the shim for faithfulness. It adds no shipped behavior, so protocol
byte-equivalence is trivially preserved (no shipped file changes).

## Open Questions

- None blocking. Whether to also port the tests into the MSVC project is deferred with the
  stale-test reconciliation.
