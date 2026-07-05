## 1. Extract the active-send helper (byte-preserving; files are CRLF)

- [x] 1.1 Declare `_ActiveSendMessage(WEASEL_IPC_COMMAND, DWORD)` in WeaselClientImpl.h and define it.
- [x] 1.2 Rewrite the 5 uniform command methods as `return _ActiveSendMessage(CMD, PARAM);`.

## 2. Verify, review, commit

- [x] 2.1 Confirm the 5 methods are one-liners; helper guards `_Active()` and returns
      `_SendMessage(...) != 0`; CRLF preserved; harness green.
- [x] 2.2 Adversarial review: confirm each rewritten method is behavior-identical (same command id,
      same param, same _Active guard, same `!= 0` return, same size_t/bool→DWORD conversion), and no
      other method was changed. Address blocking findings.
- [x] 2.3 Commit on `master` (`refactor(WeaselIPC):`) + Co-Authored-By; then
      `openspec archive table-drive-ipc-client-commands --skip-specs`.
