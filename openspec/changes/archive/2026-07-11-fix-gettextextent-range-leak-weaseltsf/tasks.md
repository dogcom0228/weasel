## 1. com_ptr the ranges + guard nSelection (byte-preserving normal path; Composition.cpp is LF, no BOM)

- [x] 1.1 In `CGetTextExtentEditSession::DoEditSession`: drop the unused `pRangeComposition` and the
      raw `ITfRange* pRange`; init `ULONG nSelection = 0;`.
- [x] 1.2 After a successful `GetSelection`, `com_ptr<ITfRange> pSelectionRange;` and
      `if (nSelection >= 1) pSelectionRange.Attach(selection.range);`.
- [x] 1.3 `com_ptr<ITfRange> pRange;` filled by `GetRange(&pRange)` on the composition branch, else
      `pRange = pSelectionRange;`.
- [x] 1.4 Guard `if (pRange != nullptr && GetTextExt(...) == S_OK && ...)`.

## 2. Verify, review, commit

- [x] 2.1 x64 compile-check green. Confirm: normal path (nSelection>=1) selects the same range and
      updates position identically; every AddRef'd range released exactly once (pSelectionRange owns
      GetSelection's ref via Attach; pRange owns GetRange's ref or an AddRef of pSelectionRange; both
      released at scope end — no double-free); nSelection==0 no longer touches garbage. LF/no-BOM
      preserved; diff limited to the one function.
- [x] 2.2 Adversarial review — COM/lifetime focus: verify CComPtr `.Attach()` takes ownership without
      an extra AddRef; `pRange = pSelectionRange` AddRefs so both releasing once is balanced;
      `GetRange(&pRange)` on a null com_ptr is valid; the `pRange != nullptr` guard only changes the
      degenerate nSelection==0 else-branch (previously UB). Address blocking findings.
- [x] 2.3 Commit on `master` (`fix(WeaselTSF):`) + Co-Authored-By; archive manually
      (git mv → openspec/changes/archive/2026-07-11-...).
