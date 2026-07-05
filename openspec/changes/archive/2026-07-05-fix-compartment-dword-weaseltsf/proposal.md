## Why

`WeaselTSF::_GetCompartmentDWORD` / `_SetCompartmentDWORD` had a use-of-uninitialized-pointer bug,
and `_GetCompartmentDWORD` also returned a misleading `HRESULT`, which in turn left the sole caller
reading an uninitialized local:

1. `ITfCompartment* pCompartment;` was declared uninitialized and `pCompartment->Release()` was
   called **outside** the `if (GetCompartment(...) == S_OK)` guard — so if `GetCompartment` fails,
   `Release()` runs on an uninitialized pointer (crash / UB). Both functions share this.
2. `_GetCompartmentDWORD` left `hr` at its initial `E_FAIL` even when it successfully read the value,
   so a successful get reported failure.
3. Its only caller, `_UpdateLanguageBar`, declared `DWORD flags;` uninitialized and used it after
   `_GetCompartmentDWORD` even though that call may not set it (compartment absent/empty/not VT_I4),
   corrupting the conversion-mode flags it then writes back.

## What Changes

- `_GetCompartmentDWORD` / `_SetCompartmentDWORD`: hold the compartment in a `com_ptr<ITfCompartment>`
  (mirroring the already-correct `_IsKeyboardOpen` in the same file), which releases only when
  actually acquired and drops the unsafe manual `Release()`.
- `_GetCompartmentDWORD`: set `hr = S_OK` when a VT_I4 value is read.
- `_SetKeyboardOpen`: same `com_ptr<ITfCompartment>` fix — it held the compartment in a raw pointer
  with **no `Release()` at all**, leaking the interface on every keyboard-open toggle (surfaced by
  review; same one-line fix and pattern).
- `_UpdateLanguageBar`: initialize `DWORD flags = 0;` so a get that doesn't populate it is
  deterministic.

## Capabilities

### New Capabilities
- (none)

### Modified Capabilities
- (none — bug fixes; the normal success path is unchanged, and the only caller ignores the returned
  HRESULT, so the corrected return code has no call-site behavior impact)

## Impact

- `WeaselTSF/Compartment.cpp` (the two accessors) and `WeaselTSF/LanguageBar.cpp` (one initializer).
  Not host-testable (TSF/COM); verified by close reading against the file's own working com_ptr
  pattern + adversarial review. LF line endings preserved.

## Non-goals

- The `#1843` Ctrl+Space OPENCLOSE-bypass / open-status-signalling behavior (the `_isToOpenClose`
  logic in `_HandleCompartment`) is a separate change; this one is scoped to the compartment
  accessors' memory-safety and return-code correctness.
