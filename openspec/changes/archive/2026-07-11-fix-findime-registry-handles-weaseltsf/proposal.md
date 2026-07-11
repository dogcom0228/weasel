## Why

`FindIME` (WeaselTSF/Register.cpp) walks the `Keyboard Layouts` registry hive to find weasel's HKL.
It closes two registry handles unconditionally, even on the paths where the corresponding
`RegOpenKeyExW` **failed** and left the handle variable uninitialized:

1. `HKEY hSubKey;` is declared uninitialized; inside the loop `RegCloseKey(hSubKey)` runs after the
   `if (ret == ERROR_SUCCESS)` block, so when `RegOpenKeyExW(hKey, key, …, &hSubKey)` fails,
   `RegCloseKey` is called on a garbage handle (undefined behavior — may close an unrelated handle).
2. `HKEY hKey;` is declared uninitialized; if the outer
   `RegOpenKeyExW(HKEY_LOCAL_MACHINE, …, &hKey)` fails, the `if (ret == ERROR_SUCCESS)` body is
   skipped but `RegCloseKey(hKey)` at function end still runs on the garbage handle.

`ret` is reused by the intervening `RegQueryValueExW`, so the sub-key close cannot simply be guarded
by `ret` after the fact — the close must move inside the block that actually opened the handle.

## What Changes

- `FindIME`: move `RegCloseKey(hSubKey)` inside the `if (ret == ERROR_SUCCESS)` block that opened it
  (so it closes only a successfully-opened sub-key), and move `RegCloseKey(hKey)` inside the outer
  `if (ret == ERROR_SUCCESS)` block (so it closes only a successfully-opened key). No other logic
  changes — the search behavior and return value are identical on the success paths.

## Capabilities

### New Capabilities
- (none)

### Modified Capabilities
- (none — memory-safety fix; on the success path (weasel installed, keys present) behavior is
  byte-identical. Only the failure paths stop invoking `RegCloseKey` on an uninitialized handle.)

## Impact

- `WeaselTSF/Register.cpp`, function `FindIME` only (~4 lines moved, no additions/removals of logic).
  Verified by x64 compile + close reading of the RegOpenKeyExW/RegCloseKey contract + adversarial
  review. LF line endings, no BOM — preserved.

## Non-goals

- The `c_szTipKeyPrefix` `"Software\\Microsft\\CTF\\TIP\\"` typo in the same file is NOT touched here:
  that Win8 TIP-cleanup block also targets the wrong registry root (`HKEY_CLASSES_ROOT` instead of
  `HKLM\SOFTWARE\Microsoft\CTF\TIP`, which `ITfInputProcessorProfileMgr::UnregisterProfile` already
  manages), so it needs a dedicated investigation, not a mechanical typo fix.
- The other WeaselTSF handle/leak findings (thread-focus sink, ITfRange leaks, compartment sink
  error paths) are separate changes.
