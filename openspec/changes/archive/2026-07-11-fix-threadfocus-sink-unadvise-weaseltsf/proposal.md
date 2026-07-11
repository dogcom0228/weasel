## Why

`WeaselTSF::Deactivate` (WeaselTSF.cpp) calls `_UninitThreadMgrEventSink()` **twice** (lines 94 and
103) — an evident copy-paste slip — and never calls `_UninitThreadFocusSink()`, which has **zero
callers** repo-wide. So the `ITfThreadFocusSink` advised in `ActivateEx` stays registered after the
text service deactivates: TSF keeps a reference to the object via the sink, and
`OnSetThreadFocus`/`OnKillThreadFocus` can still fire on a deactivated service (whose
`_pThreadMgr` has been nulled), doing IPC echoes and composition aborts against dead session state.
(The second call is harmless by itself — `_UninitThreadMgrEventSink` guards on
`TF_INVALID_COOKIE` — but it occupies exactly the slot where the focus-sink unadvise belongs.)

`_UninitThreadFocusSink` itself also lacks the module's established uninit pattern (see
`_UninitThreadMgrEventSink`, ThreadMgrEventSink.cpp): it does not guard on `TF_INVALID_COOKIE`
(so a Deactivate on the `ActivateEx` failure path, before the sink was ever advised, would call
`UnadviseSink` with an invalid cookie) and never resets the cookie, making a second call unadvise a
stale cookie. Similarly `_InitThreadFocusSink` does not reset the cookie on `AdviseSink` failure,
while `_InitThreadMgrEventSink` does.

## What Changes

- `Deactivate`: replace the second (duplicate) `_UninitThreadMgrEventSink();` with
  `_UninitThreadFocusSink();` — before `_pThreadMgr = NULL`, so the unadvise still has the thread
  manager.
- `_UninitThreadFocusSink`: mirror the `_UninitThreadMgrEventSink` pattern — early-return when
  `_dwThreadFocusSinkCookie == TF_INVALID_COOKIE`, unadvise when the `ITfSource` QI succeeds, and
  reset the cookie to `TF_INVALID_COOKIE` afterward (keeping the existing `com_ptr` style).
- `_InitThreadFocusSink`: on `AdviseSink` failure, reset `_dwThreadFocusSinkCookie` to
  `TF_INVALID_COOKIE` (mirroring `_InitThreadMgrEventSink`).

## Capabilities

### New Capabilities
- (none)

### Modified Capabilities
- (none intended — this restores the evidently-intended activate/deactivate symmetry. Observable
  change: after Deactivate, thread-focus notifications stop reaching the dead service, and the
  sink's reference on the service is dropped.)

## Impact

- `WeaselTSF/WeaselTSF.cpp` only (three small edits in Deactivate/_InitThreadFocusSink/
  _UninitThreadFocusSink). Verified by x64 compile + close reading against the module's own
  `_Init/_UninitThreadMgrEventSink` pattern + adversarial review. CRLF, UTF-8 BOM — both preserved.

## Non-goals

- The unwired `ITfActiveLanguageProfileNotifySink` (OnActivated implemented but never advised) is a
  separate, behavior-adding change — not touched.
- No change to what OnSetThreadFocus/OnKillThreadFocus do while the service IS active.
