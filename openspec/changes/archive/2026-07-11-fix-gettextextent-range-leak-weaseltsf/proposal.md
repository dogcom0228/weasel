## Why

`CGetTextExtentEditSession::DoEditSession` (WeaselTSF/Composition.cpp) runs on every composition-
window update (i.e. effectively every keystroke while composing) and leaks an `ITfRange` each time:

```cpp
ITfRange* pRange;
...
_pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &selection, &nSelection);  // AddRef's selection.range
...
if (_pComposition != nullptr && _pComposition->GetRange(&pRange) == S_OK) {  // AddRef's pRange
  pRange->Collapse(ec, TF_ANCHOR_START);
} else {
  pRange = selection.range;
}
if ((_pContextView->GetTextExt(ec, pRange, &rc, &fClipped)) == S_OK && ...) { ... }
return S_OK;   // neither selection.range nor pRange is ever Released
```

- `GetSelection` AddRefs `selection.range`; it is never released (leaked on both branches).
- When a composition exists, `GetRange` AddRefs a **separate** `pRange`; that is never released either.
- `nSelection` is requested but **never checked**, so on the composition-end branch the code uses
  `selection.range` even if `GetSelection` fetched zero selections (a latent garbage-pointer use).

## What Changes

Rewrite the range handling with `com_ptr` (the module's `CComPtr` alias) so every AddRef'd range is
released exactly once, and guard the selection use with the fetched count:

- Own the fetched selection range in `com_ptr<ITfRange> pSelectionRange`, attached **only when
  `nSelection >= 1`** (so an unfetched, un-AddRef'd range is never attached/released).
- Make `pRange` a `com_ptr<ITfRange>` (fed by `GetRange(&pRange)` on the composition branch, or
  assigned from `pSelectionRange` on the composition-end branch).
- Guard `GetTextExt` with `pRange != nullptr` (the only new-behavior path is the degenerate
  `nSelection == 0` composition-end case, which previously dereferenced a garbage `selection.range`).
- Remove the unused `com_ptr<ITfRange> pRangeComposition;` local (declared, never used).

## Capabilities

### New Capabilities
- (none)

### Modified Capabilities
- (none for the normal path — with a selection present (`nSelection >= 1`), the ranges chosen and
  passed to `GetTextExt` and the caret-position update are byte-identical; only the previously-leaked
  refs are now released. The single behavior difference is the degenerate `nSelection == 0`
  composition-end case: it now skips `GetTextExt` (position unchanged) instead of dereferencing a
  garbage pointer — strictly safer, and unreachable in normal editing where a caret selection exists.)

## Impact

- `WeaselTSF/Composition.cpp`, function `CGetTextExtentEditSession::DoEditSession` only. Verified by
  x64 compile + close reading of the GetSelection/GetRange AddRef contracts and `CComPtr`
  Attach/assignment/`operator&` semantics + adversarial review. LF line endings, no BOM — preserved.

## Non-goals

- The unrelated `CStartCompositionEditSession::DoEditSession` E_FAIL-on-success return-code issue in
  the same file is a separate deferred change.
- No change to `_SetCompositionPosition`, the enhanced-position logic, or the composition lifecycle.
