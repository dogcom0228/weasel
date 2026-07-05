## 1. Fix the compartment accessors (byte-preserving; files are LF)

- [x] 1.1 `_GetCompartmentDWORD`: `com_ptr<ITfCompartment>` + `hr = S_OK` on VT_I4; drop manual Release.
- [x] 1.2 `_SetCompartmentDWORD`: `com_ptr<ITfCompartment>`; drop manual Release.
- [x] 1.3 `_SetKeyboardOpen`: `com_ptr<ITfCompartment>` (fix the no-Release leak).
- [x] 1.4 `_UpdateLanguageBar`: `DWORD flags = 0;`.

## 2. Verify, review, commit

- [x] 2.1 Confirm no manual `pCompartment->Release()` remains; the com_ptr pattern matches the file's
      `_IsKeyboardOpen`; LF preserved; harness green.
- [x] 2.2 Adversarial review — COM focus: confirm the com_ptr ref-count behavior matches the old
      manual Release on the success path (single release, no leak/double-free), the GetCompartment
      failure path no longer touches an uninitialized pointer, `hr = S_OK` is correct and doesn't
      change the (return-ignoring) caller, and `flags = 0` is a safe default. Address blocking findings.
- [x] 2.3 Commit on `master` (`fix(WeaselTSF):`) + Co-Authored-By; then
      `openspec archive fix-compartment-dword-weaseltsf --skip-specs`.
