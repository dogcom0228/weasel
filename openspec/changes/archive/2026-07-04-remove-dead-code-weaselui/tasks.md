## 1. Remove dead code + fix defects (byte-preserving; files are CRLF)

- [x] 1.1 `WeaselPanel.cpp`: remove the `#if 0 … #endif` debug-draw block in `_TextOut`.
- [x] 1.2 `include/WeaselUI.h`: remove unused `ResetLayout()` inline.
- [x] 1.3 `WeaselPanel.cpp`: fix `"OpenClipord ailed"` → `"OpenClipboard failed"`.
- [x] 1.4 `WeaselPanel.cpp`: remove dead locals `before_str`/`hilited_str`/`after_str` in
      `_DrawPreedit` (confirmed unused; real vars are `str_before`/`str_highlight`/`str_after`).

## 2. Verify, review, commit

- [x] 2.1 Confirm zero remaining references to `ResetLayout` and the removed locals; `git diff --stat`
      small; CRLF preserved. `SetDpi` intentionally kept.
- [x] 2.2 Run `test/host/run.sh` (invariant sanity — unrelated portable layer stays green).
- [x] 2.3 Adversarial review agent on the diff (deadness, the dead-locals are truly unread, no
      behavior change, line endings intact). Address blocking findings.
- [x] 2.4 Commit on `master` (`refactor(WeaselUI):`) + Co-Authored-By; then
      `openspec archive remove-dead-code-weaselui --skip-specs`.
