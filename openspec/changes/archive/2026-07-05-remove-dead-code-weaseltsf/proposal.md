## Why

WeaselTSF (the TSF text-service DLL) has accumulated unreferenced helpers, dead members,
`#if 0`/commented blocks, and one write-only member. Removing them shrinks the surface before the
higher-risk structural TSF work (com_ptr, refcount base, edit-session unification) in later waves.
Because this is COM code, each removal was proven dead with COM awareness (not a TSF/IUnknown vtable
method, not a `.def` export — only `Dll*` are exported).

## What Changes

- Remove the file-scope `static` helper `error_message` (WeaselTSF.cpp) — no callers.
- Remove the file-scope `static LANGID GetActiveProfileLangId()` (LanguageBar.cpp) — no callers.
- Remove the private inline `WeaselTSF::isImmersive()` and, with it, the now write-only member
  `_activateFlags` (declaration + its lone write in `ActivateEx`). `ActivateEx`'s `dwFlags`
  parameter stays (it is the `ITfTextInputProcessorEx::ActivateEx` contract).
- Remove the never-used member `WeaselTSF::_editSessionText`.
- Remove the `_InitPreservedKey` `#if 0 … #endif` body (dead after an unconditional `return TRUE;`)
  and the now-unused `GUID_IME_MODE_PRESERVED_KEY` (declaration in Globals.h + definition in
  Globals.cpp; its only reference lived inside that `#if 0`).
- Remove the commented-out `MODIFIER_NAME_TABLE` struct/table (KeyEvent.cpp) and the commented
  `ITfCompartmentEventSink`/`OnChange` declaration (WeaselTSF.h — WeaselTSF does not implement that
  interface; the real one is on `CCompartmentEventSink`).
- Remove the write-only DisplayAttributeInfo cluster: member `_pValueName`, its write, and the now
  orphaned `_daiInputName` string (`_daiDescription`/`_pDescription` are read and kept).
- Remove the commented `_UpdateOwner` declaration + call (CandidateList.h/.cpp).

## Capabilities

### New Capabilities
- (none)

### Modified Capabilities
- (none — all removals are unreferenced/never-compiled/write-only; runtime behavior unchanged)

## Impact

- 11 files in `WeaselTSF/`, 85 lines removed, no additions. No whole-file changes (xmake globs
  `*.cpp`; no `.vcxproj`/`.def` edits needed). Mixed CRLF (WeaselTSF.cpp/.h) / LF preserved.
- Not part of the portable harness; verified by COM-aware reference proof + adversarial review.

## Non-goals

- Two latent bugs found nearby are NOT touched here (recorded for a dedicated WeaselTSF bug-fix
  change): `_GetCompartmentDWORD` returning E_FAIL on its success path (Compartment.cpp) and
  `CStartCompositionEditSession::DoEditSession`'s return value (Composition.cpp).
- The structural TSF refactors (com_ptr conversion, centralized refcount, lambda edit session) are
  separate later-wave changes.
