## 1. Harden the sink lifecycle (byte-preserving; Compartment.h/.cpp are LF, no BOM)

- [x] 1.1 `Compartment.h` ctor: add `_cookie(0)` to the member-init list.
- [x] 1.2 `Compartment.cpp` `_Unadvise`: `if (_compartment == nullptr) return S_OK;` before the QI.
- [x] 1.3 `Compartment.cpp` `_InitCompartment`: `DWORD hr` → `HRESULT hr`.

## 2. Verify, review, commit

- [x] 2.1 x64 compile-check green. Successful advise/unadvise cycle unchanged; failed-advise →
      Deactivate path now a clean no-op (no null deref, no garbage cookie). LF/no-BOM preserved.
- [x] 2.2 Adversarial review: confirm `_compartment` is null exactly on the failed-advise paths
      (CComPtr default-null; only GetCompartment success sets it); `_cookie` reset value 0 matches
      the ctor init; `UnadviseSink` is never reached with a garbage cookie on any ordering
      (including double `_Unadvise`); the DWORD→HRESULT change is bit-identical through
      `SUCCEEDED`. Confirm the return-policy Non-goal is accurately scoped. Address blockers.
- [x] 2.3 Commit on `master` (`fix(WeaselTSF):`) + Co-Authored-By; archive manually.
