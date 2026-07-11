## Why

`WeaselTSF::_HandleLangBarMenuSelect` (WeaselTSF/LanguageBar.cpp, `ID_WEASELTRAY_USERCONFIG` case)
tests a registry read with the HRESULT macro:

```cpp
if (FAILED(RegGetStringValue(HKEY_CURRENT_USER, L"Software\\Rime\\Weasel",
                             L"RimeUserDir", dir)) ||
    dir.empty()) {
```

`RegGetStringValue` returns an `LSTATUS` (Win32 error code): `ERROR_SUCCESS` is 0 and failures are
small **positive** integers, so `FAILED()` — which tests the HRESULT sign bit — is **always false**.
The check silently degenerates to `dir.empty()`. Today that masks the bug (a failed read leaves
`dir` empty, so the `%AppData%\Rime` fallback still triggers), but the expression is semantically
wrong, misleads readers, and would break if the helper ever wrote a partial value on failure. The
same function already uses the correct form five lines up
(`RegGetStringValue(...) == ERROR_SUCCESS`, line 285).

## What Changes

- Replace `FAILED(RegGetStringValue(...))` with `RegGetStringValue(...) != ERROR_SUCCESS`, matching
  the sibling call site in the same function.

## Capabilities

### New Capabilities
- (none)

### Modified Capabilities
- (none — the composite condition evaluates identically for every current behavior of
  `RegGetStringValue` (failure leaves `dir` untouched/empty): fallback on failure-or-empty, open the
  registry-configured dir otherwise. Only the dead sub-expression becomes live and correct.)

## Impact

- `WeaselTSF/LanguageBar.cpp`, one line. Verified by x64 compile + close reading of the
  LSTATUS/HRESULT semantics against the sibling call + adversarial review. LF, UTF-8 BOM — both
  preserved.

## Non-goals

- The per-keystroke LanguageBar OnUpdate/GetIcon churn (audit UX finding) is a separate change.
