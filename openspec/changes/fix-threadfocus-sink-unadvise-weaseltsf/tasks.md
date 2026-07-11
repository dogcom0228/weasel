## 1. Restore activate/deactivate symmetry (byte-preserving; WeaselTSF.cpp is CRLF with a UTF-8 BOM)

- [x] 1.1 `Deactivate`: second duplicate `_UninitThreadMgrEventSink();` → `_UninitThreadFocusSink();`.
- [x] 1.2 `_UninitThreadFocusSink`: guard `TF_INVALID_COOKIE`, unadvise on successful QI, reset
      cookie (mirror `_UninitThreadMgrEventSink`).
- [x] 1.3 `_InitThreadFocusSink`: reset cookie to `TF_INVALID_COOKIE` on `AdviseSink` failure.

## 2. Verify, review, commit

- [x] 2.1 x64 compile-check green. Confirm ordering (unadvise before `_pThreadMgr = NULL`), the
      failure-path Deactivate (ExitError before the sink was advised) is now a guarded no-op, and a
      double Deactivate cannot unadvise a stale cookie. CRLF + UTF-8 BOM preserved.
- [x] 2.2 Adversarial review: confirm the first `_UninitThreadMgrEventSink()` at line 94 still runs
      (mgr sink still unadvised exactly once); no other caller of `_UninitThreadFocusSink` exists;
      cookie state machine (ctor TF_INVALID_COOKIE → advise sets → unadvise resets) is consistent on
      all Activate/Deactivate orderings incl. ActivateEx ExitError. Address blocking findings.
- [x] 2.3 Commit on `master` (`fix(WeaselTSF):`) + Co-Authored-By; archive manually
      (git mv → openspec/changes/archive/2026-07-11-...).
