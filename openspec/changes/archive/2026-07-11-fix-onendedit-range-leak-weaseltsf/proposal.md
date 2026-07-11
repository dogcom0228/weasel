## Why

`WeaselTSF::OnEndEdit` (WeaselTSF/TextEditSink.cpp) runs on every edit while composing. When the
selection changed and a composition is active, it fetches the current selection:

```cpp
if (pContext->GetSelection(ecReadOnly, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) == S_OK &&
    cFetched == 1) {
  ITfRange* pRangeComposition;
  if (_pComposition->GetRange(&pRangeComposition) == S_OK) {
    if (!IsRangeCovered(ecReadOnly, tfSelection.range, pRangeComposition))
      _EndComposition(pContext, true);
    pRangeComposition->Release();
  }
}
```

`GetSelection` fills `TF_SELECTION` with an **AddRef'd** `ITfRange` (`tfSelection.range`). The
composition range `pRangeComposition` is correctly released, but `tfSelection.range` is **never
released** — so each selection-change edit during composition leaks one `ITfRange`. Because caret
movement while typing triggers `OnEndEdit`, this leaks repeatedly over a composing session.

## What Changes

- `OnEndEdit`: release `tfSelection.range` at the end of the `GetSelection … && cFetched == 1`
  block (the same scope that guarantees it was fetched and AddRef'd), after the composition-range
  handling. `pRangeComposition`'s existing release is unchanged.

## Capabilities

### New Capabilities
- (none)

### Modified Capabilities
- (none — a resource-leak fix; the composition-end decision and all observable behavior are
  unchanged. Only the previously-leaked selection range is now released.)

## Impact

- `WeaselTSF/TextEditSink.cpp`, function `OnEndEdit` only (+1 line). Verified by x64 compile + close
  reading of the `ITfContext::GetSelection` AddRef contract (mirrored by the file's own
  `pRangeComposition->Release()`) + adversarial review. LF line endings, no BOM — preserved.

## Non-goals

- The `GetTextAndPropertyUpdates` block lower in `OnEndEdit` (a leftover from the MS TSF sample that
  enumerates one range and releases it with no observable effect) is a separate dead-code question,
  not touched here — it already releases its handles, so it does not leak.
- The analogous `selection.range` / composition-range leak in
  `CGetTextExtentEditSession::DoEditSession` is a separate change (it additionally needs an
  `nSelection`-fetched guard the current code lacks).
