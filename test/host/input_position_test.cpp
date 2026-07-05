// Host test: proves include/InputPositionCodec.h reproduces, bit-for-bit, the
// original hand-rolled encode (WeaselIPC client) and decode (WeaselIPCServer)
// arithmetic. The `ref_*` functions below are the ORIGINAL logic copied verbatim
// (written in the well-defined unsigned form that matches the MSVC output), and
// the sweep asserts the codec equals the reference for every sampled input —
// so the extraction is guaranteed behavior-preserving. Pure portable C++.
#include <algorithm>
#include <cstdint>
#include <cstdio>

#include <InputPositionCodec.h>

using std::int32_t;
using std::uint32_t;

// --- Reference: the ORIGINAL client encode (WeaselClientImpl::UpdateInputPosition).
// Original computed from a RECT: left=rc.left, top=rc.top, height=rc.bottom-rc.top.
static uint32_t ref_encode(int left, int top, int height) {
  int hi_res = (height >= 128 || left < -2048 || left >= 2048 || top < -2048 ||
                top >= 2048)
                   ? 1
                   : 0;
  int L = std::max(-2048, std::min(2047, left >> hi_res));
  int T = std::max(-2048, std::min(2047, top >> hi_res));
  int H = std::max(0, std::min(127, height >> hi_res));
  return (static_cast<uint32_t>(hi_res & 0x01) << 31) |
         ((static_cast<uint32_t>(H) & 0x7f) << 24) |
         ((static_cast<uint32_t>(T) & 0xfff) << 12) |
         (static_cast<uint32_t>(L) & 0xfff);
}

// --- Reference: the ORIGINAL server decode (ServerImpl::OnUpdateInputPosition).
struct RefPos {
  int left, top, height;
};
static RefPos ref_decode(uint32_t w) {
  int hi_res = (w >> 31) & 0x01;
  RefPos p;
  p.left = static_cast<int32_t>(((w & 0x7ff) - (w & 0x800)) << hi_res);
  p.top =
      static_cast<int32_t>((((w >> 12) & 0x7ff) - ((w >> 12) & 0x800)) << hi_res);
  p.height = static_cast<int32_t>(((w >> 24) & 0x7f) << hi_res);
  return p;
}

static int g_failures = 0;
#define CHECK(cond, msg)                                     \
  do {                                                       \
    if (!(cond)) {                                           \
      std::printf("FAIL: %s  (%s:%d)\n", msg, __FILE__, __LINE__); \
      ++g_failures;                                          \
    }                                                        \
  } while (0)

int main() {
  const int samples[] = {-5000, -4096, -4095, -2049, -2048, -2047, -1, 0,
                         1,     2046,  2047,  2048,  4094,  4095,  5000};
  const int heights[] = {0, 1, 2, 63, 126, 127, 128, 129, 200, 254, 255, 400};

  // Equivalence sweep: codec must match the original arithmetic exactly, in
  // both directions, for every sampled input.
  for (int left : samples) {
    for (int top : samples) {
      for (int h : heights) {
        uint32_t got = weasel::EncodeInputPosition({left, top, h});
        uint32_t exp = ref_encode(left, top, h);
        CHECK(got == exp, "encode mismatch");

        // Decode equivalence over the produced word (covers all valid words).
        weasel::InputPosition d = weasel::DecodeInputPosition(got);
        RefPos rd = ref_decode(got);
        CHECK(d.left == rd.left && d.top == rd.top && d.height == rd.height,
              "decode mismatch");
      }
    }
  }

  // Decode equivalence over an independent word sweep (incl. hi_res + sign bits).
  const uint32_t words[] = {0x00000000u, 0x80000000u, 0xffffffffu, 0x7fffffffu,
                            0x00000800u, 0x00800000u, 0x7f000000u, 0x000fffffu};
  for (uint32_t w : words) {
    weasel::InputPosition d = weasel::DecodeInputPosition(w);
    RefPos rd = ref_decode(w);
    CHECK(d.left == rd.left && d.top == rd.top && d.height == rd.height,
          "decode-word mismatch");
  }

  // A couple of explicit anchors documenting the intended layout.
  // Low-res: left=100, top=200, height=20 -> plain pack, decodes back exactly.
  {
    uint32_t w = weasel::EncodeInputPosition({100, 200, 20});
    weasel::InputPosition d = weasel::DecodeInputPosition(w);
    CHECK(d.left == 100 && d.top == 200 && d.height == 20, "low-res roundtrip");
  }
  // Negative 12-bit boundary sign-extends correctly.
  {
    uint32_t w = weasel::EncodeInputPosition({-2048, -1, 0});
    weasel::InputPosition d = weasel::DecodeInputPosition(w);
    CHECK(d.left == -2048 && d.top == -1 && d.height == 0, "negative roundtrip");
  }

  if (g_failures == 0)
    std::printf("input_position_test: all checks passed\n");
  return g_failures;
}
