## 1. Remove dead code (byte-preserving; WeaselTSF.cpp/.h are CRLF, others LF)

- [x] 1.1 `WeaselTSF.cpp`: remove `error_message` helper.
- [x] 1.2 `LanguageBar.cpp`: remove `GetActiveProfileLangId`.
- [x] 1.3 `WeaselTSF.h`/`WeaselTSF.cpp`: remove `isImmersive()` + `_activateFlags` (member + write in
      `ActivateEx`; keep the `dwFlags` parameter — COM contract).
- [x] 1.4 `WeaselTSF.h`: remove `_editSessionText`.
- [x] 1.5 `KeyEventSink.cpp`: remove the `_InitPreservedKey` `#if 0` block; `Globals.{h,cpp}`: remove
      `GUID_IME_MODE_PRESERVED_KEY` decl + def.
- [x] 1.6 `KeyEvent.cpp`: remove commented `MODIFIER_NAME_TABLE`; `WeaselTSF.h`: remove commented
      `ITfCompartmentEventSink`/`OnChange` decl.
- [x] 1.7 `DisplayAttributeInfo.{h,cpp}`: remove `_pValueName` (member + write) and orphaned
      `_daiInputName`.
- [x] 1.8 `CandidateList.{h,cpp}`: remove commented `_UpdateOwner` decl + call.

## 2. Verify, review, commit

- [x] 2.1 Confirm zero remaining references to every removed symbol; `_InitPreservedKey` still
      compiles (`return TRUE;`); `ActivateEx` keeps its signature; `git diff --stat` small; CRLF/LF
      preserved (grep -c $'\r').
- [x] 2.2 Run `test/host/run.sh` (invariant sanity).
- [x] 2.3 Adversarial review agent on the diff — COM focus: re-prove none of the removed methods are
      TSF/IUnknown vtable methods or `.def` exports; confirm `_activateFlags`/`_pValueName` are truly
      write-only; confirm no dangling references and line endings intact. Address blocking findings.
- [x] 2.4 Commit on `master` (`refactor(WeaselTSF):`) + Co-Authored-By; then
      `openspec archive remove-dead-code-weaseltsf --skip-specs`.
