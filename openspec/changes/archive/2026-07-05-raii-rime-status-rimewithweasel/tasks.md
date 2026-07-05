## 1. RAII for RimeStatus (byte-preserving; file is CRLF)

- [x] 1.1 Add non-copyable `RimeStatusGuard` in an anonymous namespace after `static RimeApi* rime_api;`.
- [x] 1.2 Convert the 5 `RimeStatus` sites (AddSession, UpdateColorTheme, _LoadAppInlinePreeditSet,
      _Respond, _GetStatus): add `RimeStatusGuard status_guard(&status, true);` as the first statement
      in each `if (get_status(...))` block; delete the paired manual `free_status`.

## 2. Verify, review, commit

- [x] 2.1 Confirm exactly 5 guards added and 5 manual `free_status` removed (grep `free_status` → only
      the guard dtor + a comment); `session_status.status = status` still precedes each block's close;
      CRLF preserved; harness still green.
- [x] 2.2 Adversarial review — EXHAUSTIVE double-free check: for EACH of the 5 blocks confirm there is
      no remaining manual `free_status` (the guard dtor must be the sole free), the guard is armed on
      the success path only, the copy-then-free order is preserved, and nested handles (#8's
      config_close) keep their order. Confirm no other Rime handle kind was touched.
- [x] 2.3 Commit on `master` (`refactor(RimeWithWeasel):`) + Co-Authored-By; then
      `openspec archive raii-rime-status-rimewithweasel --skip-specs`.
