#pragma once
// Codec for the 32-bit "input position" word carried by
// WEASEL_IPC_UPDATE_INPUT_POS. The TSF client reports the caret rectangle in
// physical pixels; it is packed into a single DWORD (wParam) here and unpacked
// on the server. Encoder and decoder previously duplicated the exact bit layout
// in WeaselIPC (client) and WeaselIPCServer (server); this header is the single
// source of truth for that layout.
//
//   bit  31    : hi_res flag  (fields were >>1 before packing when set)
//   bits 30-24 : height       (7-bit, 0..127)
//   bits 23-12 : top          (12-bit signed, -2048..2047)
//   bits 11-0  : left         (12-bit signed, -2048..2047)
//
// When any field would overflow its range, hi_res is set and left/top/height
// are right-shifted by one (dropping the low bit) to double the reach.
//
// Pure portable C++ (no <windows.h>); host-tested in test/host.
#include <algorithm>
#include <cstdint>

namespace weasel {

// Caret rectangle the client reports, in physical pixels. `height` is
// bottom - top; the packed word does not carry width (the server uses a fixed
// small width when reconstructing the rect).
struct InputPosition {
  int left;
  int top;
  int height;
};

inline std::uint32_t EncodeInputPosition(const InputPosition& p) {
  const int hi_res = (p.height >= 128 || p.left < -2048 || p.left >= 2048 ||
                      p.top < -2048 || p.top >= 2048)
                         ? 1
                         : 0;
  const int left = std::clamp(p.left >> hi_res, -2048, 2047);
  const int top = std::clamp(p.top >> hi_res, -2048, 2047);
  const int height = std::clamp(p.height >> hi_res, 0, 127);
  return (static_cast<std::uint32_t>(hi_res) << 31) |
         ((static_cast<std::uint32_t>(height) & 0x7f) << 24) |
         ((static_cast<std::uint32_t>(top) & 0xfff) << 12) |
         (static_cast<std::uint32_t>(left) & 0xfff);
}

inline InputPosition DecodeInputPosition(std::uint32_t word) {
  const int hi_res = static_cast<int>((word >> 31) & 0x01u);
  InputPosition p;
  // 12-/7-bit sign-extension done in the unsigned domain exactly as the original
  // server did (already UB-free there), then reinterpreted as a signed int.
  p.left =
      static_cast<std::int32_t>(((word & 0x7ffu) - (word & 0x800u)) << hi_res);
  p.top = static_cast<std::int32_t>(
      (((word >> 12) & 0x7ffu) - ((word >> 12) & 0x800u)) << hi_res);
  p.height = static_cast<std::int32_t>(((word >> 24) & 0x7fu) << hi_res);
  return p;
}

}  // namespace weasel
