// Host-side golden tests for Weasel's portable IPC text-protocol parser.
//
// These feed *server-formatted* responses (the same key=value\n text protocol and
// boost text_woarchive payloads that RimeWithWeasel::_Respond emits) through the REAL
// client parser translation units and assert the reconstructed data. They run on Linux
// via test/host/run.sh; see test/host/README.md. A failing assertion makes the process
// exit with the number of failures (boost::report_errors()).
//
// NOTE: ResponseParser::operator() drops a final line that has no trailing '\n'
// (it is line-based and returns before Feed() on stream end). Every response below
// therefore terminates its LAST line with '\n' — including the boost `ctx.cand=`
// blob, which already ends in one. Omitting it would silently skip that line.
#include <windows.h>  // shim: WCHAR/UINT/LPWSTR + friends

#include <boost/archive/text_woarchive.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <sstream>
#include <string>

#include <ResponseParser.h>

using namespace weasel;

// Serialize a CandidateInfo exactly as the server does for the `ctx.cand=` line.
static std::wstring cand_blob(const CandidateInfo& ci) {
  std::wstringstream ss;
  {
    boost::archive::text_woarchive oa(ss);
    oa << ci;
  }
  return ss.str();  // boost text archive ends with a newline
}

// action=noop must leave commit and context untouched.
static void test_noop() {
  WCHAR resp[] = L"action=noop\n";
  std::wstring commit;
  Context ctx;
  Status st;
  ResponseParser parse(&commit, &ctx, &st);
  parse(resp, (UINT)wcslen(resp));
  BOOST_TEST(commit.empty());
  BOOST_TEST(ctx.empty());
}

// commit action delivers the (unescaped) commit string and leaves aux alone.
static void test_commit_unescape() {
  WCHAR resp[] =
      L"action=commit\n"
      L"commit=hello\\nworld\n";  // \n is an escaped linefeed in the protocol
  std::wstring commit;
  Context ctx;
  Status st;
  ctx.aux.str = L"prev";
  ResponseParser parse(&commit, &ctx, &st);
  parse(resp, (UINT)wcslen(resp));
  BOOST_TEST(commit == L"hello\nworld");
  BOOST_TEST(ctx.aux.str == L"prev");
  BOOST_TEST(ctx.preedit.empty());
}

// preedit text + three-field cursor (start,end,cursor) and status flags.
static void test_ctx_and_status() {
  WCHAR resp[] =
      L"action=ctx,status\n"
      L"ctx.preedit=abc\n"
      L"ctx.preedit.cursor=0,3,3\n"
      L"status.ascii_mode=1\n"
      L"status.composing=1\n";
  std::wstring commit;
  Context ctx;
  Status st;
  ResponseParser parse(&commit, &ctx, &st);
  parse(resp, (UINT)wcslen(resp));
  BOOST_TEST(ctx.preedit.str == L"abc");
  BOOST_ASSERT(ctx.preedit.attributes.size() == 1);
  BOOST_TEST_EQ(HIGHLIGHTED, ctx.preedit.attributes[0].type);
  BOOST_TEST_EQ(0, ctx.preedit.attributes[0].range.start);
  BOOST_TEST_EQ(3, ctx.preedit.attributes[0].range.end);
  BOOST_TEST_EQ(3, ctx.preedit.attributes[0].range.cursor);
  BOOST_TEST(st.ascii_mode);
  BOOST_TEST(st.composing);
}

// candidates travel as a boost text_woarchive blob in `ctx.cand=`; round-trip them.
static void test_candidates_roundtrip() {
  CandidateInfo ci;
  ci.currentPage = 0;
  ci.totalPages = 1;
  ci.highlighted = 1;
  ci.is_last_page = true;
  ci.candies.push_back(Text(L"cand-A"));
  ci.candies.push_back(Text(L"cand-B"));
  ci.labels.push_back(Text(L"1"));
  ci.labels.push_back(Text(L"2"));

  std::wstring resp = L"action=ctx\nctx.cand=" + cand_blob(ci);
  std::wstring commit;
  Context ctx;
  Status st;
  ResponseParser parse(&commit, &ctx, &st);
  parse((LPWSTR)resp.c_str(), (UINT)resp.size());

  BOOST_ASSERT(ctx.cinfo.candies.size() == 2);
  BOOST_TEST(ctx.cinfo.candies[0].str == L"cand-A");
  BOOST_TEST(ctx.cinfo.candies[1].str == L"cand-B");
  BOOST_TEST_EQ(1, ctx.cinfo.highlighted);
  BOOST_TEST_EQ(1, ctx.cinfo.totalPages);
  BOOST_TEST(ctx.cinfo.is_last_page);
}

// Regression (bug): ctx.preedit.cursor with only two fields (start,end) must not
// read a non-existent third field. The server normally sends three, but the
// parser must not index out of bounds on a short message.
static void test_preedit_cursor_two_fields() {
  WCHAR resp[] =
      L"action=ctx\n"
      L"ctx.preedit=abc\n"
      L"ctx.preedit.cursor=0,3\n";
  std::wstring commit;
  Context ctx;
  Status st;
  ResponseParser parse(&commit, &ctx, &st);
  parse(resp, (UINT)wcslen(resp));
  BOOST_TEST(ctx.preedit.str == L"abc");
  BOOST_ASSERT(ctx.preedit.attributes.size() == 1);
  BOOST_TEST_EQ(0, ctx.preedit.attributes[0].range.start);
  BOOST_TEST_EQ(3, ctx.preedit.attributes[0].range.end);
}

// Correctness: config.inline_preedit sets the Config sink when one is provided.
static void test_config_inline_preedit() {
  WCHAR resp[] =
      L"action=config\n"
      L"config.inline_preedit=1\n";
  std::wstring commit;
  Context ctx;
  Status st;
  Config cfg;
  ResponseParser parse(&commit, &ctx, &st, &cfg);
  parse(resp, (UINT)wcslen(resp));
  BOOST_TEST(cfg.inline_preedit);
}

// Regression (bug): config.* must not crash when no Config sink was provided
// (p_config is null) even though a Context sink is. Reaching the end is the test.
static void test_config_null_sink_safe() {
  WCHAR resp[] =
      L"action=config\n"
      L"config.inline_preedit=1\n";
  std::wstring commit;
  Context ctx;
  Status st;
  ResponseParser parse(&commit, &ctx, &st);  // no Config sink
  parse(resp, (UINT)wcslen(resp));
  BOOST_TEST(ctx.empty());
}

// Regression (robustness): a corrupted `ctx.cand=` blob whose string-length
// field is absurd makes the underlying wstring::resize throw std::length_error
// — which is NOT a boost archive_exception. TryDeserialize must contain it so
// a malformed IPC payload cannot crash the host application, and the parser
// must keep consuming subsequent lines.
static void test_corrupt_cand_blob_no_crash() {
  CandidateInfo ci;
  ci.currentPage = 0;
  ci.totalPages = 1;
  ci.highlighted = 0;
  ci.candies.push_back(Text(L"cand-A"));

  std::wstring blob = cand_blob(ci);
  // The text archive stores the candidate string as "<len> <chars>"; corrupt
  // the length of "cand-A" to a value wstring::resize must reject.
  const std::wstring good = L"6 cand-A";
  std::size_t pos = blob.find(good);
  BOOST_ASSERT(pos != std::wstring::npos);  // archive layout as expected
  blob.replace(pos, good.size(), L"9999999999999999999 cand-A");

  // blob already ends with the archive's trailing newline, so the status line
  // that follows starts a fresh protocol line.
  std::wstring resp = L"action=ctx,status\nctx.cand=" + blob;
  resp += L"status.ascii_mode=1\n";
  std::wstring commit;
  Context ctx;
  Status st;
  ResponseParser parse(&commit, &ctx, &st);
  parse((LPWSTR)resp.c_str(), (UINT)resp.size());
  BOOST_TEST(st.ascii_mode);  // parser survived the corrupt blob and kept going
}

int main() {
  test_noop();
  test_commit_unescape();
  test_ctx_and_status();
  test_candidates_roundtrip();
  test_preedit_cursor_two_fields();
  test_config_inline_preedit();
  test_config_null_sink_safe();
  test_corrupt_cand_blob_no_crash();
  return boost::report_errors();
}
