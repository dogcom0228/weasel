## Why

Follow-up to `raii-rime-status-rimewithweasel`: apply the same exception-safe RAII to the remaining
Rime C handles in `RimeWithWeasel.cpp` (`RimeConfig`, `RimeConfigIterator`, `RimeContext`,
`RimeCommit`), which were also freed manually and leak on early return / thrown exception between
acquire and release.

## What Changes

- Add four non-copyable guards next to the existing `RimeStatusGuard` (same shape; each frees its
  handle in the destructor iff armed with a successful acquire): `RimeConfigGuard` (→ `config_close`),
  `RimeConfigIteratorGuard` (→ `config_end`), `RimeContextGuard` (→ `free_context`), `RimeCommitGuard`
  (→ `free_commit`).
- Convert the 10 remaining sites (RimeConfig ×5, RimeConfigIterator ×3, RimeContext ×1, RimeCommit ×1):
  - **Guarded-block sites** (`if (acquire) { … release; }`): place the guard as the block's first
    statement and delete the paired release — the destructor fires at the same closing brace.
  - **Early-return sites** (`if (!acquire) return; … release;`): place the guard *after* the failure
    `return` (so the uninitialized struct is never wrapped on the failure path) and delete the
    scope-final release.

After this change every `config_close`/`config_end`/`free_context`/`free_commit`/`free_status` call
in the file appears exactly once — inside its guard's destructor — so a double free is structurally
impossible, and no handle is freed unless its acquire succeeded.

## Capabilities

### New Capabilities
- (none)

### Modified Capabilities
- (none — free points and ordering are byte-identical on the normal path; only exception/early-return
  safety is added)

## Impact

- One file, `RimeWithWeasel/RimeWithWeasel.cpp` (+55/−10). No new headers. Not host-testable; verified
  by close reading + an exhaustive adversarial double-free / unacquired-free review. CRLF preserved.

## Non-goals

- No semantic change to the config parsing itself; guards only move the free to scope exit. The
  `RimeTraits` struct (no `free_*`) and the long-lived `session_status.status` copy are untouched.
