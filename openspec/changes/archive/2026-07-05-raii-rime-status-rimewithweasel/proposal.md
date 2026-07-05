## Why

`RimeWithWeasel.cpp` acquires Rime C handles and frees them manually, which leaks the handle on any
early return or thrown exception between acquire and free (the module audit flagged this as a
high-severity smell; `u8tow`, `boost::archive`, `swprintf_s`, and `std::string`/map operations can
all throw between `get_status` and `free_status`). Introduce RAII for the most repetitive handle —
`RimeStatus` — so the free is exception-safe, while keeping the free point byte-identical.

## What Changes

- Add a tiny non-copyable `RimeStatusGuard` (anonymous namespace next to the file-static `rime_api`)
  that calls `rime_api->free_status` in its destructor iff it was armed with a successful acquire.
- Convert the **5** `RimeStatus` acquire/free sites (`AddSession`, `UpdateColorTheme`,
  `_LoadAppInlinePreeditSet`, `_Respond`, `_GetStatus`): place `RimeStatusGuard status_guard(&status,
  true);` as the first statement inside each `if (rime_api->get_status(...))` block and delete the
  paired manual `free_status`. Because the manual free was the last statement of that block, the
  destructor fires at the same closing brace — the free point is unchanged; the only new behavior is
  that the handle is also freed if the block exits early or throws.

## Capabilities

### New Capabilities
- (none)

### Modified Capabilities
- (none — free point and ordering are byte-identical on the normal path; the change only adds
  exception/early-return safety)

## Impact

- One file, `RimeWithWeasel/RimeWithWeasel.cpp` (+23/−5). No new headers (the guard uses the
  file-static `rime_api`, so it must live in this TU). Not host-testable (Rime C API); verified by
  close reading + an exhaustive adversarial review focused on double-free.

## Non-goals

- The other Rime handle kinds — `RimeConfig`/`schema_open` (5 sites), `RimeContext` (1), `RimeCommit`
  (1), and `RimeConfigIterator` (3) — are deliberately deferred to a follow-up change. They are also
  safe-mechanical but involve uninitialized-struct and early-return variants; scoping this first pass
  to the 5 identical `RimeStatus` sites minimizes execution risk while we lack a compiler.
- The pre-existing shallow-copy of `session_status.status = status` (whose `char*` members alias the
  freed buffers but are never read) is unchanged — RAII preserves the exact copy-then-free order.
