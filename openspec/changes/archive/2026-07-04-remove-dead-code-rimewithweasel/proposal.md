## Why

`RimeWithWeasel.cpp` is a single 1500-line translation unit that has accumulated unreferenced code.
Removing provably-dead code shrinks the surface before later structural work touches this file, and
one of the dead fragments is hiding a real latent bug (an uninitialized struct member read before
first write). This module is a static library with no `.def` exports, so unreferenced free/private
functions and macros are genuinely unreachable and safe to delete.

## What Changes

- Remove `RimeWithWeaselHandler::_GetContext` — definition (`RimeWithWeasel.cpp`) and its private
  declaration (`include/RimeWithWeasel.h`). It has zero call sites (the other `_GetContext` in the
  tree is an unrelated `PipeChannel` method), is non-virtual, and is not exported.
- Remove the unused `#define TRANSPARENT_COLOR` macro (`RimeWithWeasel.cpp`) — referenced nowhere.
- Remove the unused file-scope `inline std::string _GetLabelText(...)` helper (`RimeWithWeasel.cpp`)
  — no header declaration and no call sites.
- Fix the `SessionStatus` constructor (`include/RimeWithWeasel.h`): its body
  `RIME_STRUCT(RimeStatus, status);` only ever declared a *local* that shadowed and discarded the
  member of the same name, leaving the member `status` uninitialized. The member is read field-wise
  (`.is_ascii_mode`) before its first assignment (RimeWithWeasel.cpp:178 and :776). Replace the dead
  body with a `status{}` member value-initializer so those reads see deterministic zeros.

## Capabilities

### New Capabilities
- (none)

### Modified Capabilities
- (none — no requirement or user-visible behavior changes; the constructor fix restores the
  intended default-initialized state rather than changing a specified behavior)

## Impact

- Files: `RimeWithWeasel/RimeWithWeasel.cpp`, `include/RimeWithWeasel.h`. ~35 lines removed, 1 line
  changed. No other module references any removed symbol (verified by repo-wide reference search).
- No IPC protocol, serialization, or UI behavior changes. Not part of the portable test harness, so
  verification is by reference-search proof + adversarial review (no Windows compiler available).

## Non-goals

- No other refactoring of `RimeWithWeasel.cpp` (the god-object split, RAII, dark-mode dedup, etc. are
  separate later changes). This change only *removes* proven-dead code and fixes the one bug the dead
  code concealed.
- The `SessionStatus` fix deliberately uses a macro-independent `status{}` member initializer rather
  than `RIME_STRUCT_INIT` (which is not used anywhere in the repo and cannot be compile-verified
  here); the member's `data_size` is never passed to librime (only locals are), so zero-initializing
  the member's fields is sufficient and provably safe.
