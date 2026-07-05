## Why

Two latent crash bugs in the WeaselIPC response parser — both in the portable, host-testable layer,
so they can be fixed test-first:

1. `ContextUpdater::_StoreText` reads `vec[2]` (the cursor field of `ctx.preedit.cursor`) but only
   guards `vec.size() < 2`. A message with two fields (`start,end`) indexes out of bounds — undefined
   behavior (garbage cursor in a release build; abort under hardened STL).
2. `Configurator::Store` guards `!m_pTarget->p_context` but then dereferences `m_pTarget->p_config`.
   A client that provides a Context sink but no Config sink (`p_config == nullptr`) crashes on any
   `config.*` line. (Plausibly related to sporadic weasel.dll crash reports such as #1883, though the
   exact field repro is unconfirmed.)

## What Changes

- Add three regression tests to `test/host/golden_test.cpp` FIRST (they reproduce the abort/crash),
  then fix:
  - `_StoreText`: read the cursor only `if (vec.size() >= 3)`, so a two-field cursor keeps its
    start/end highlight (cursor defaults to -1) instead of reading out of bounds. The normal
    three-field message is unchanged.
  - `Configurator::Store`: guard the pointer it actually uses — `!m_pTarget->p_config` — so a missing
    Config sink is a safe no-op (and, symmetrically, a present Config sink is applied even when there
    is no Context sink).

## Capabilities

### New Capabilities
- (none)

### Modified Capabilities
- `host-test-harness`: now also guards these two parser robustness cases (short cursor field; missing
  Config sink).

## Impact

- `WeaselIPC/ContextUpdater.cpp` (+1 line guard) and `WeaselIPC/Configurator.cpp` (1 token) — both
  compiled by the host harness, so the fixes are proven test-first (red → green). New tests in
  `test/host/golden_test.cpp`. Normal-path behavior (3-field cursor, present Config sink) is
  unchanged. Mixed CRLF/LF preserved.

## Non-goals

- The modal `MessageBoxA` on a boost-archive exception (Deserializer.h `TryDeserialize`) is a
  separate concern (a UI/logging change) and is not touched here.
- No change to the IPC protocol or the server; these are client-side parser hardening fixes.
