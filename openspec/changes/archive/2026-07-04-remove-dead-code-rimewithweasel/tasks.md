## 1. Remove dead code

- [x] 1.1 Delete `#define TRANSPARENT_COLOR 0x00000000` (`RimeWithWeasel.cpp`, ~line 15).
- [x] 1.2 Delete the unused `inline std::string _GetLabelText(...)` helper and its trailing blank
      line (`RimeWithWeasel.cpp`, ~lines 730-737).
- [x] 1.3 Delete `RimeWithWeaselHandler::_GetContext` definition (`RimeWithWeasel.cpp`, ~lines
      1476-1500, including the following blank line) and its declaration (`include/RimeWithWeasel.h`,
      ~line 83).

## 2. Fix SessionStatus constructor bug

- [x] 2.1 In `include/RimeWithWeasel.h`, change the `SessionStatus` constructor to add a `status{}`
      member initializer (in declaration order: `style`, `status`, `__synced`, `session_id`) and
      remove the dead `RIME_STRUCT(RimeStatus, status);` body.

## 3. Verify, review, commit

- [x] 3.1 Re-grep the whole repo to confirm each removed symbol has zero remaining references and the
      build files/`.def`/`.rc` do not name them.
- [x] 3.2 Run `test/host/run.sh` — it must still pass (this change does not touch the portable layer,
      so it is a regression sanity check that nothing shared broke).
- [x] 3.3 Spawn an adversarial review agent on the diff (deadness proofs, the `status{}` fix
      correctness, member order / no `-Wreorder`, no unintended edits). Address blocking findings.
- [x] 3.4 Commit on `master` with a `refactor(RimeWithWeasel):` message + Co-Authored-By trailer;
      then `openspec archive remove-dead-code-rimewithweasel`.
