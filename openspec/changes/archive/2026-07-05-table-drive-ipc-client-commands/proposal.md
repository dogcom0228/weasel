## Why

Five `ClientImpl` command methods (`CommitComposition`, `ClearComposition`,
`SelectCandidateOnCurrentPage`, `HighlightCandidateOnCurrentPage`, `ChangePage`) were byte-identical
apart from the command id and the parameter: each did `if (!_Active()) return false; LRESULT ret =
_SendMessage(CMD, PARAM, session_id); return ret != 0;`. Collapse that shared shape into one helper.

## What Changes

- Add `ClientImpl::_ActiveSendMessage(WEASEL_IPC_COMMAND command, DWORD param)` =
  `if (!_Active()) return false; return _SendMessage(command, param, session_id) != 0;`.
- Rewrite the five methods as one-line `return _ActiveSendMessage(CMD, PARAM);`.

## Capabilities

### New Capabilities
- (none)

### Modified Capabilities
- (none — behavior-identical dedup)

## Impact

- `WeaselIPC/WeaselClientImpl.{h,cpp}` (−10 lines net). The `size_t index` / `bool backward` / `0`
  arguments convert to the helper's `DWORD param` exactly as they did when passed straight to
  `_SendMessage`, so behavior is identical. Not host-tested (client transport isn't in the harness);
  verified by close reading + review. CRLF preserved.

## Non-goals

- Methods with different shapes are left alone: `StartSession` (Echo/connect logic), `EndSession`
  (clears `session_id`), `FocusIn` (client_caps), `FocusOut`/`TrayCommand`/`ShutdownServer` (no
  `_Active` guard / different params), `ProcessKeyEvent`, `UpdateInputPosition`.
