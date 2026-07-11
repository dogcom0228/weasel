// Host test for format_label (include/StringAlgorithm.hpp): the safe replacement
// for swprintf_s(buffer, user_label_format, label) at the candidate-label sites.
// A user-config format like "%d"/"%n" used to trip the CRT invalid-parameter
// handler and crash the host/server; format_label must render those literally
// while staying byte-identical for valid "%s"-based formats. Pure portable, no
// Boost; built as a third suite by test/host/run.sh.
#include <cstdio>
#include <string>

#include <StringAlgorithm.hpp>

static int g_failures = 0;

static void expect_eq(const std::wstring& got,
                      const std::wstring& want,
                      const char* name) {
  if (got != want) {
    ++g_failures;
    std::printf("  FAIL %s\n", name);
  }
}

int main() {
  // Valid formats: byte-identical to the old swprintf output.
  expect_eq(format_label(L"%s.", L"1"), L"1.", "default %s.");
  expect_eq(format_label(L"%s", L"abc"), L"abc", "bare %s");
  expect_eq(format_label(L"(%s)", L"2"), L"(2)", "surrounded %s");
  expect_eq(format_label(L"foo", L"1"), L"foo", "no-%s literal");
  expect_eq(format_label(L"%s%%", L"a"), L"a%", "escaped percent");
  expect_eq(format_label(L"%s.", L"①"), L"①.", "unicode label");

  // Only the first %s is substituted (there is a single argument).
  expect_eq(format_label(L"%s-%s", L"x"), L"x-%s", "first-%s-only");

  // Formats that USED TO CRASH swprintf are now rendered verbatim.
  expect_eq(format_label(L"%d", L"1"), L"%d", "stray %d literal");
  expect_eq(format_label(L"%n", L"1"), L"%n", "stray %n literal");
  expect_eq(format_label(L"%s%d", L"5"), L"5%d", "%s then stray %d");
  expect_eq(format_label(L"a%", L"1"), L"a%", "trailing percent");
  expect_eq(format_label(L"%", L"1"), L"%", "lone percent");

  // Degenerate inputs.
  expect_eq(format_label(L"", L"1"), L"", "empty format");
  expect_eq(format_label(L"%s", L""), L"", "empty label");

  if (g_failures == 0)
    std::printf("format_label_test: all checks passed\n");
  else
    std::printf("format_label_test: %d failing check(s)\n", g_failures);
  return g_failures;
}
