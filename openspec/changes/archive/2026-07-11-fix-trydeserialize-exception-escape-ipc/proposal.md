## Why

`weasel::TryDeserialize` (WeaselIPC/Deserializer.h) guards the boost text-archive deserialization
of the `ctx.cand=` candidate blob (and `style=` via Styler) — data that arrives over IPC from the
server. It catches **only** `boost::archive::archive_exception`. But the archive layer can throw
other `std::exception` types for malformed input: a corrupted string-length field makes
`std::wstring::resize` throw `std::length_error` (or `std::bad_alloc` for allocatable-but-huge
sizes) — those escape `TryDeserialize`, unwind through `ResponseParser::operator()` inside the
**host application's** TSF call path, and crash the host app. An IME must never let a malformed
IPC payload take down the process it is typing into.

This is host-testable: the Linux harness compiles Deserializer.h/ContextUpdater.cpp verbatim, and
the new golden test crashes the test binary before the fix and passes after it (TDD).

## What Changes

- `TryDeserialize`: add a second catch block, `catch (const std::exception& e)`, after the existing
  `archive_exception` catch, reporting via the same `MessageBoxA` path with a `std::exception: `
  prefix. The existing `archive_exception` branch is byte-identical (unchanged message for the
  already-handled case); the new branch only runs where today the process crashes.
- `test/host/golden_test.cpp`: new test `test_corrupt_cand_blob_no_crash` — serializes a valid
  one-candidate blob, corrupts the candidate string's length field to a huge value (verified
  present at runtime before corruption), feeds it through the real parser followed by a
  `status.ascii_mode=1` line, and asserts the parser survives and continues parsing.

## Capabilities

### New Capabilities
- (none)

### Modified Capabilities
- (none user-visible for well-formed traffic — all valid payloads parse byte-identically. For
  malformed payloads that throw during `ia >> t`, the client now reports an IPC error instead of
  crashing the host process. Note: the `text_wiarchive` *construction* in ContextUpdater/Styler
  reads the archive header outside this guard and can still throw — a pre-existing, separate
  escape recorded for a follow-up change.)

## Impact

- `WeaselIPC/Deserializer.h` (+6 lines incl. comment), `test/host/golden_test.cpp` (+1 test). The portable layer
  is touched → `test/host/run.sh` run via WSL is mandatory and part of verification (red before /
  green after). Deserializer.h is CRLF (no BOM), golden_test.cpp is LF — both preserved.

## Non-goals

- Removing/replacing the modal `MessageBoxA` in the IPC parse path (deferred UI/logging concern from
  `fix-ipc-parser-field-bugs`) — the new branch deliberately mirrors the existing reporting so this
  change stays a pure robustness fix.
- Hardening the line-based splitter against embedded newlines in archive blobs (noted latent
  fragility, separate investigation).
