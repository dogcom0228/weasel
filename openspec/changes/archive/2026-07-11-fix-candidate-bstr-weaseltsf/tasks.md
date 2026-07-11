## 1. Fix the malformed BSTR returns (byte-preserving; CandidateList.cpp is LF, no BOM)

- [x] 1.1 `GetString`: change `SysAllocStringLen(str.c_str(), static_cast<UINT>(str.size()) + 1)`
      to `... static_cast<UINT>(str.size()))` (drop the `+ 1`).
- [x] 1.2 `GetDescription`: replace the function-local `static` BSTR with a per-call
      `*pbstr = SysAllocString(L"Candidate List");`, keeping the `if (pbstr)` guard and `S_OK`.

## 2. Verify, review, commit

- [x] 2.1 Confirmed against the Win32 contracts: `SysAllocStringLen(psz, len)` copies `len` chars +
      appends a NUL (so `size()` is correct); the returned BSTR out-param is caller-owned/freed (so
      a fresh alloc per call is required). No other `GetString`/`GetDescription` behavior changes;
      LF/no-BOM preserved; `git diff --stat` shows only the intended 2 methods (2 ins, 3 del).
- [x] 2.2 Adversarial review — COM/memory focus: PASS, no blocking findings. Confirmed the
      off-by-one fix does not truncate (SysAllocStringLen still NUL-terminates), the per-call alloc
      matches BSTR ownership and cannot itself leak (caller frees), no in-repo caller depends on the
      old shared-static identity or +1 length (the accessors are TSF vtable entry points), and the
      OOM behavior (S_OK with *pbstr==null) is unchanged from before.
- [x] 2.3 Commit on `master` (`fix(WeaselTSF):`) + Co-Authored-By trailer; then
      `openspec archive fix-candidate-bstr-weaseltsf --skip-specs --no-validate -y`.
