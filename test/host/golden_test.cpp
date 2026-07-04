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

int main() {
  test_noop();
  test_commit_unescape();
  test_ctx_and_status();
  test_candidates_roundtrip();
  return boost::report_errors();
}
