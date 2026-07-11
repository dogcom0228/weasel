## 1. Guard the registry closes (byte-preserving; Register.cpp is LF, no BOM)

- [x] 1.1 `FindIME`: move `RegCloseKey(hSubKey)` inside the sub-key `if (ret == ERROR_SUCCESS)`
      block (last statement before its `}`), and move `RegCloseKey(hKey)` inside the outer
      `if (ret == ERROR_SUCCESS)` block (after the `for`).

## 2. Verify, review, commit

- [x] 2.1 x64 compile-check green (regenerate weasel.props, msbuild x64). Confirm success-path
      behavior unchanged and both closes now only run on successfully-opened handles; LF/no-BOM
      preserved; `git diff --stat` shows only Register.cpp with a small hunk.
- [x] 2.2 Adversarial review: confirm no handle is leaked on the success path (each opened handle is
      still closed exactly once) and none is closed on the failure path; `ret` reuse is handled by
      moving the close inside the opening block rather than re-testing `ret`. Address blocking findings.
- [x] 2.3 Commit on `master` (`fix(WeaselTSF):`) + Co-Authored-By; archive manually
      (git mv → openspec/changes/archive/2026-07-11-...).
