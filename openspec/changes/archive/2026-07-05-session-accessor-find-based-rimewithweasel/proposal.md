## Why

`RimeWithWeaselHandler::to_session_id()` looked up `m_session_status_map[ipc_id]` with
`std::map::operator[]`, which **default-inserts** a `SessionStatus{session_id=0}` for any unknown
key. So merely translating a stale/unknown ipc_id — reachable from the client via
`OnEcho → FindSession(arbitrary DWORD) → to_session_id` — fabricates a phantom session entry that
lingers in the map and can even inflate future session ids (rime/weasel#1303).

## What Changes

- Make `to_session_id()` a non-inserting `find`: return the real `session_id` for a known key, or `0`
  for an unknown key, without touching the map. This is value-identical to the old behavior on both
  paths (a miss already returned `0`), minus the fabrication side effect.

## Capabilities

### New Capabilities
- (none)

### Modified Capabilities
- (none — `to_session_id` returns the same value it did before for every input; only the phantom
  map insertion on a miss is removed)

## Impact

- `include/RimeWithWeasel.h` only (3 lines). All 16 `to_session_id` call sites consume the return by
  value and none relies on the insert, so behavior is unchanged for valid sessions. Not host-testable
  (Rime C API); verified by close reading of every call site + review. CRLF preserved.

## Non-goals

- `get_session_status()` (which returns a reference and would need a non-inserting fallback object)
  is deliberately left as `operator[]` for now: its only miss paths are internal `ipc_id == 0`
  operations where every `rime_api` call already fails, and a reference-returning fix warrants a
  separate change that traces the `ipc_id == 0` write-persistence paths. This change fixes the one
  client-reachable fabrication vector.
