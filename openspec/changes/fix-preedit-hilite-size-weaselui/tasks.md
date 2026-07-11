## 1. Fix the highlighted-segment measurement (byte-preserving; StandardLayout.cpp is CRLF, no BOM)

- [x] 1.1 `GetPreeditSize`: change `preedit.substr(_range.start, _range.end)` to
      `preedit.substr(_range.start, _range.end - _range.start)`.

## 2. Verify, review, commit

- [x] 2.1 Confirmed the `std::wstring::substr(pos, count)` contract: 2nd arg is a length, so
      `_range.end - _range.start` is the highlighted-segment length; TextRange::start/end are `int`
      (WeaselIPCData.h:22-23) and the `if (_range.start < _range.end)` guard makes it positive;
      matches the sibling `substr(0, _range.start)` / `substr(_range.end)`. CRLF/no-BOM preserved.
- [x] 2.2 Adversarial review: PASS, no blocking findings. Confirmed the new length equals the
      highlighted range width, cannot underflow (signed, guarded), and the paint path uses the
      identical `substr(range.start, range.end - range.start)` (WeaselPanel.cpp:660) so measurement
      and paint now agree; the untouched `pos` arg means no new out_of_range risk.
- [x] 2.3 Commit on `master` (`fix(WeaselUI):`) + Co-Authored-By trailer; then archive
      (git mv into openspec/changes/archive/2026-07-11-... — openspec CLI absent; --skip-specs equiv).
