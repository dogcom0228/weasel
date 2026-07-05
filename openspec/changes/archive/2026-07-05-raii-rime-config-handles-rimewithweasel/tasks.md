## 1. RAII for the remaining Rime handles (byte-preserving; file is CRLF)

- [x] 1.1 Add `RimeConfigGuard`, `RimeConfigIteratorGuard`, `RimeContextGuard`, `RimeCommitGuard`
      next to `RimeStatusGuard` (same non-copyable, armed-on-success shape).
- [x] 1.2 Convert the 5 `RimeConfig` sites (Initialize, UpdateColorTheme, _LoadSchemaSpecificSettings
      main + nested weaselconfig, _LoadAppInlinePreeditSet), the 3 `RimeConfigIterator` sites (preset,
      ForEachRimeMap, ForEachRimeList), the 1 `RimeContext` and 1 `RimeCommit` site in `_Respond`.
      Guarded-block → guard first stmt; early-return → guard after the failure `return`.

## 2. Verify, review, commit

- [x] 2.1 Confirm each of `config_close`/`config_end`/`free_context`/`free_commit`/`free_status`
      appears exactly once (its guard dtor); 15 guard sites total; early-return guards are AFTER the
      `return`; CRLF preserved; harness green.
- [x] 2.2 Adversarial review — EXHAUSTIVE: per site confirm no leftover manual release (no double
      free), guard armed on success only (esp. the 3 early-return sites must not wrap an
      uninitialized struct on the failure path), free point + nested ordering preserved, guards
      non-copyable. Confirm no non-Rime code changed.
- [x] 2.3 Commit on `master` (`refactor(RimeWithWeasel):`) + Co-Authored-By; then
      `openspec archive raii-rime-config-handles-rimewithweasel --skip-specs`.
