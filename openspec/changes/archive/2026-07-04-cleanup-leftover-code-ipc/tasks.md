## 1. Remove dead code (byte-preserving; repo has mixed CRLF/LF)

- [x] 1.1 `WeaselIPC/Styler.h`: delete the `namespace weasel { class Deserializr; }` block.
- [x] 1.2 `WeaselIPC/Deserializer.cpp`: delete the two commented `insert(make_pair(...))` lines.
- [x] 1.3 `WeaselIPC/WeaselClientImpl.h`: delete `_Connected()`.
- [x] 1.4 `WeaselIPCServer/WeaselServerImpl.cpp`: delete `GetServerRunner` (decl + def) and the
      `ServerRunner` alias.
- [x] 1.5 `WeaselIPCServer/WeaselServerImpl.h`: delete the commented `// class ServerImpl`.

## 2. Verify, review, commit

- [x] 2.1 Confirm zero residual references to `Deserializr`, `GetServerRunner`, `ServerRunner`,
      `_Connected`; confirm `git diff --stat` is small (no line-ending churn) and CRLF preserved.
- [x] 2.2 Run `test/host/run.sh` — must pass (compile-verifies the Styler.h/Deserializer.cpp edits).
- [x] 2.3 Adversarial review agent on the diff (deadness proofs; no behavior change; line endings
      intact). Address blocking findings.
- [x] 2.4 Commit on `master` (`refactor(WeaselIPC):`) + Co-Authored-By; then
      `openspec archive cleanup-leftover-code-ipc --skip-specs`.
