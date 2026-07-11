## 1. Release the fetched selection range (byte-preserving; TextEditSink.cpp is LF, no BOM)

- [x] 1.1 `OnEndEdit`: add `tfSelection.range->Release();` at the end of the
      `if (GetSelection(...) == S_OK && cFetched == 1)` block, after the
      `_pComposition->GetRange(...)` handling.

## 2. Verify, review, commit

- [x] 2.1 x64 compile-check green. Confirm the release is inside the `cFetched == 1` guard (so it
      only runs when the range was actually fetched/AddRef'd), mirrors the file's existing
      `pRangeComposition->Release()`, and does not change the composition-end decision. LF/no-BOM
      preserved; `git diff --stat` shows one added line in TextEditSink.cpp.
- [x] 2.2 Adversarial review: confirm `GetSelection` AddRefs `tfSelection.range` on `cFetched >= 1`;
      the release cannot run on an unfetched/garbage range; no double-release with pRangeComposition
      (distinct object); no early-return path skips a needed release. Address blocking findings.
- [x] 2.3 Commit on `master` (`fix(WeaselTSF):`) + Co-Authored-By; archive manually
      (git mv → openspec/changes/archive/2026-07-11-...).
