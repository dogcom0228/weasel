## 1. uiid (CandidateList.h/.cpp are LF, no BOM)

- [x] 1.1 Header: `kInvalidUIElementId` constant + `DWORD uiid = kInvalidUIElementId;`.
- [x] 1.2 `_UpdateUIElement`: early-return `S_OK` when `uiid == kInvalidUIElementId`.
- [x] 1.3 `StartUI`: on `FAILED(BeginUIElement(...))` reset `uiid = kInvalidUIElementId`.
- [x] 1.4 `EndUI`: call `EndUIElement` only when `uiid != kInvalidUIElementId`; reset after.

## 2. _gaDisplayAttributeInput (WeaselTSF.cpp is CRLF + UTF-8 BOM; DisplayAttribute.cpp is LF, no BOM)

- [x] 2.1 Ctor: `_gaDisplayAttributeInput = TF_INVALID_GUIDATOM;`.
- [x] 2.2 `_SetCompositionDisplayAttributes`: early-return `FALSE` on the invalid atom.

## 3. Verify, review, commit

- [x] 3.1 x64 compile-check green; working paths byte-identical; garbage-to-TSF paths now skip
      deterministically. Each file's EOL/BOM preserved.
- [x] 3.2 Adversarial review: confirm every read of `uiid`/`_gaDisplayAttributeInput` is now either
      guarded or post-successful-write on all orderings (UpdateUI-before-StartUI; BeginUIElement
      failure; EndUI-twice; StartUI/EndUI cycles; ActivateEx tolerated-failure path); confirm
      TF_INVALID_GUIDATOM == 0 is never returned by a successful RegisterGUID; confirm the sole
      caller of _SetCompositionDisplayAttributes ignores the return. Address blockers.
- [x] 3.3 Commit on `master` (`fix(WeaselTSF):`) + Co-Authored-By; archive manually.
