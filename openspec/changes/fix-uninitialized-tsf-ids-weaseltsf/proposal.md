## Why

Two TSF identifiers are read while potentially uninitialized and handed straight to TSF APIs:

1. **`CCandidateList::uiid`** (CandidateList.h:82) is written only by `BeginUIElement` in
   `StartUI()`. But `UpdateUI()` — reached from `WeaselTSF::_UpdateUI` on **every** response edit
   session — unconditionally calls `_UpdateUIElement()`, which passes `uiid` to
   `ITfUIElementMgr::UpdateUIElement` even when `StartUI` never ran (or `BeginUIElement` failed).
   `EndUI()` likewise passes it to `EndUIElement`. Until the first successful `BeginUIElement`,
   that is a garbage element id sent to the host's UI element manager on every keystroke response.
2. **`WeaselTSF::_gaDisplayAttributeInput`** (WeaselTSF.h:223) is written only by `RegisterGUID` in
   `_InitDisplayAttributeGuidAtom` — whose failure `ActivateEx` deliberately tolerates (some hosts
   provide no display-attribute support; see the comment at WeaselTSF.cpp:133). On that tolerated
   path the atom stays garbage, and `_SetCompositionDisplayAttributes` writes it into
   `GUID_PROP_ATTRIBUTE` as the composition's display attribute (`var.lVal`).

## What Changes

- `CandidateList.h`: add `static constexpr DWORD kInvalidUIElementId = static_cast<DWORD>(-1);`
  and default-init `DWORD uiid = kInvalidUIElementId;` (matching the class's existing in-class
  init style, e.g. `_selectionStyle`).
- `CandidateList.cpp`:
  - `_UpdateUIElement()`: early-return `S_OK` while `uiid == kInvalidUIElementId`.
  - `StartUI()`: check `BeginUIElement`'s HRESULT; on failure reset `uiid = kInvalidUIElementId`
    (covers both "failed without writing" and "stale id from a previous cycle").
  - `EndUI()`: only call `EndUIElement` when `uiid != kInvalidUIElementId`, and reset `uiid` to the
    sentinel afterward (so a later `UpdateUI` without a new `StartUI` cannot replay a dead id).
- `WeaselTSF.cpp` constructor: `_gaDisplayAttributeInput = TF_INVALID_GUIDATOM;` (next to the
  existing cookie inits).
- `DisplayAttribute.cpp` `_SetCompositionDisplayAttributes`: early-return `FALSE` when
  `_gaDisplayAttributeInput == TF_INVALID_GUIDATOM` (its only caller, Composition.cpp:296, ignores
  the return value).

## Capabilities

### New Capabilities
- (none)

### Modified Capabilities
- (none on working paths — once `BeginUIElement`/`RegisterGUID` succeed, every call is
  byte-identical. The changed paths are exactly the ones that today pass garbage to TSF: they now
  skip the call (element updates before a begun element; display attributes in hosts where atom
  registration failed — the already-tolerated degraded mode) deterministically.)

## Impact

- `WeaselTSF/CandidateList.h`(+4/-1)/`CandidateList.cpp`(guards), LF no BOM;
  `WeaselTSF/WeaselTSF.cpp` (ctor, +1), CRLF with UTF-8 BOM; `WeaselTSF/DisplayAttribute.cpp`
  (guard) — check its EOL before editing. Verified by x64 compile + close reading + adversarial
  review.

## Non-goals

- `_pbShow`'s freshness across `BeginUIElement` failures (ctor-initialized TRUE, unchanged).
- The commented-out `UpdateUIElement` call in `StartUI` and the `TF_INVALID_GUIDATOM`-vs-0 cookie
  convention questions stay as-is.
- No change to when `StartUI`/`EndUI` are invoked.
