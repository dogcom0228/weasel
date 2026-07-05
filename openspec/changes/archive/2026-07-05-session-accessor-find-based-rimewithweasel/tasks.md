## 1. Non-inserting to_session_id (byte-preserving; file is CRLF)

- [x] 1.1 Rewrite `to_session_id` in `include/RimeWithWeasel.h` as a `find` returning the entry's
      `session_id` or `0` on miss; leave `get_session_status`/`new_session_status` unchanged.

## 2. Verify, review, commit

- [x] 2.1 Confirm the diff is 3 lines in one file; CRLF preserved; harness still green.
- [x] 2.2 Adversarial review: confirm value-identity (hit → same session_id, miss → 0) and that all
      `to_session_id` call sites use the returned value (never rely on the insert side effect); confirm
      get_session_status/new_session_status untouched. Address blocking findings.
- [x] 2.3 Commit on `master` (`fix(RimeWithWeasel):`) + Co-Authored-By; then
      `openspec archive session-accessor-find-based-rimewithweasel --skip-specs`.
