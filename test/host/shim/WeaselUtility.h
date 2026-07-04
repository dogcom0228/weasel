#pragma once
// Host-test shim for <WeaselUtility.h>.
//
// The real header is Win32-heavy (wrl/client.h, RegGetValue, FormatMessage, ...),
// but the IPC parse path uses only its portable string helpers. This shim exposes
// just those so the parser sources compile on Linux without dragging in Win32.
//
// TEMPORARY DUPLICATION: EscapeChar/unescape_string below are copied VERBATIM from
// include/WeaselUtility.h. A follow-up refactor should extract these portable helpers
// into include/StringAlgorithm.hpp so this shim (and shipped code) can include the
// single real definition instead of a copy. Until then, keep them in sync.
#include <sstream>
#include <string>

#include <StringAlgorithm.hpp>  // real portable header: split(), etc.

template <typename CharT>
struct EscapeChar {
  static const CharT escape;
  static const CharT linefeed;
  static const CharT tab;
  static const CharT linefeed_escape;
  static const CharT tab_escape;
};
template <>
inline const char EscapeChar<char>::escape = '\\';
template <>
inline const char EscapeChar<char>::linefeed = '\n';
template <>
inline const char EscapeChar<char>::tab = '\t';
template <>
inline const char EscapeChar<char>::linefeed_escape = 'n';
template <>
inline const char EscapeChar<char>::tab_escape = 't';
template <>
inline const wchar_t EscapeChar<wchar_t>::escape = L'\\';
template <>
inline const wchar_t EscapeChar<wchar_t>::linefeed = L'\n';
template <>
inline const wchar_t EscapeChar<wchar_t>::tab = L'\t';
template <>
inline const wchar_t EscapeChar<wchar_t>::linefeed_escape = L'n';
template <>
inline const wchar_t EscapeChar<wchar_t>::tab_escape = L't';

template <typename CharT>
inline std::basic_string<CharT> unescape_string(
    const std::basic_string<CharT>& input) {
  using Esc = EscapeChar<CharT>;
  std::basic_stringstream<CharT> res;
  for (auto p = input.begin(); p != input.end(); ++p) {
    if (*p == Esc::escape) {
      if (++p == input.end()) {
        break;
      } else if (*p == Esc::linefeed_escape) {
        res << Esc::linefeed;
      } else if (*p == Esc::tab_escape) {
        res << Esc::tab;
      } else {  // \a => a
        res << *p;
      }
    } else {
      res << *p;
    }
  }
  return res.str();
}

// Referenced only by WeaselIPC.h's inline GetPipeName(), which the parse path
// never calls; a declaration/stub is enough to compile.
inline std::wstring getUsername() {
  return L"";
}
