## Why

`CCompartmentEventSink` (WeaselTSF/Compartment.h/.cpp) has two crash-grade error paths and a type
slip, all on the advise/unadvise lifecycle:

1. **`_cookie` is never initialized** — the constructor inits only `_callback` and `_refCount`. If
   `_Advise` fails before `AdviseSink` runs (QI or `GetCompartment` failure), `_cookie` stays
   garbage, and a later `_Unadvise` hands that garbage to `UnadviseSink`.
2. **`_Unadvise` dereferences `_compartment` with no null check** (`_compartment->QueryInterface`).
   `_compartment` (a `com_ptr`) is only set when `GetCompartment` succeeded inside `_Advise`; on the
   failure paths it stays null. `_UninitCompartment` calls `_Unadvise()` whenever the sink pointer
   itself is non-null — which is always, after `new` — so a failed `_Advise` followed by Deactivate
   is a null-pointer crash (CComPtr `operator->` on null).
3. `_InitCompartment` stores the `_Advise` HRESULT in a **`DWORD`** — an unsigned type standing in
   for HRESULT; the value bits are identical but the type is wrong and invites sign-test mistakes.

## What Changes

- `Compartment.h`: initialize `_cookie(0)` in the constructor's member-init list (0 matches the
  value `_Unadvise` itself resets to).
- `Compartment.cpp` `_Unadvise`: early-return `S_OK` when `_compartment == nullptr` (nothing was
  advised, so there is nothing to unadvise; callers ignore the return value).
- `Compartment.cpp` `_InitCompartment`: `DWORD hr` → `HRESULT hr` (same bits, correct type; the
  `SUCCEEDED(hr)` result is unchanged).

## Capabilities

### New Capabilities
- (none)

### Modified Capabilities
- (none on any currently-working path — successful advise/unadvise cycles are byte-identical. The
  only affected paths are ones that today are undefined behavior: unadvising after a failed advise
  now returns cleanly instead of dereferencing null / passing a garbage cookie.)

## Impact

- `WeaselTSF/Compartment.h` (+ ctor init) and `WeaselTSF/Compartment.cpp` (guard + type), both LF,
  no BOM — preserved. Verified by x64 compile + close reading + adversarial review.

## Non-goals

- `_InitCompartment`'s return-value POLICY is deliberately unchanged: it still returns
  `SUCCEEDED(second _Advise)` only, silently tolerating a failed FIRST advise (keyboard open/close
  sink). "Fixing" that to fail activation would change behavior — today the IME still activates in
  a degraded mode; failing `ActivateEx` outright could make the IME unusable in a host where that
  advise fails. Recorded as a separate policy decision on the roadmap.
- The hand-rolled refcounting (`_refCount`, ctor-ref-1 + CComPtr double-ref interplay) is the
  Wave-C/E lifetime work, not touched here.
