## 1. Correct the LSTATUS check (byte-preserving; LanguageBar.cpp is LF with a UTF-8 BOM)

- [x] 1.1 `_HandleLangBarMenuSelect` / `ID_WEASELTRAY_USERCONFIG`:
      `FAILED(RegGetStringValue(...))` → `RegGetStringValue(...) != ERROR_SUCCESS`.

## 2. Verify, review, commit

- [x] 2.1 x64 compile-check green; condition truth-table identical for all reachable states
      (failure leaves `dir` empty ⇒ fallback either way; success+non-empty ⇒ open configured dir).
      LF + BOM preserved.
- [x] 2.2 Adversarial review: confirm RegGetStringValue's return type/contract (LSTATUS, positive
      error codes; whether it can write a non-empty dir on failure), and that the new form matches
      the sibling at line ~285. Address blocking findings.
- [x] 2.3 Commit on `master` (`fix(WeaselTSF):`) + Co-Authored-By; archive manually.
