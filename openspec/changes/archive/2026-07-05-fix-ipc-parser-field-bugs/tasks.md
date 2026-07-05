## 1. Test-first (red), then fix (green)

- [x] 1.1 Add regression tests to `test/host/golden_test.cpp`: two-field `ctx.preedit.cursor`;
      `config.inline_preedit` with and without a Config sink. Run harness → RED (OOB abort).
- [x] 1.2 Fix `WeaselIPC/ContextUpdater.cpp` `_StoreText`: read `vec[2]` only `if (vec.size() >= 3)`.
- [x] 1.3 Fix `WeaselIPC/Configurator.cpp` `Store`: guard `!m_pTarget->p_config` (the pointer it
      dereferences) instead of `!m_pTarget->p_context`.
- [x] 1.4 Run harness → GREEN (all suites pass).

## 2. Verify, review, commit

- [x] 2.1 Confirm harness green; normal 3-field cursor + present-Config-sink paths unchanged; CRLF/LF
      preserved.
- [x] 2.2 Adversarial review: confirm the cursor fix leaves the normal 3-field path byte-identical
      and only guards the short case; the Configurator guard change is correct (function only uses
      p_config) and doesn't regress any other config key handling. Address blocking findings.
- [x] 2.3 Commit on `master` (`fix(WeaselIPC):`) + Co-Authored-By; then
      `openspec archive fix-ipc-parser-field-bugs` (spec delta present — do NOT skip specs).
