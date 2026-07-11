#pragma once
#include <algorithm>
#include <clocale>
#include <cwctype>
#include <numeric>
#include <set>
#include <string>
#include <vector>

inline bool ends_with(const std::wstring& wstr, const std::wstring& wsub)
{
	if (wstr.size() < wsub.size())
		return false;
	else
		return std::equal(wsub.rbegin(), wsub.rend(), wstr.rbegin());
}

inline bool iequals(const std::wstring& str1, const std::wstring& str2)
{
	return std::equal(str1.begin(), str1.end(), str2.begin(), [](const wchar_t& wc1, const wchar_t& wc2)
	{
		return std::towlower(wc1) == std::towlower(wc2);
	});
}

inline void ireplace_last(std::wstring& input, const std::wstring& search, const std::wstring& sub)
{
	std::size_t pos = input.rfind(search);
	if (pos != std::wstring::npos)
		input.replace(pos, search.length(), sub);
}

inline std::string join(const std::set<std::string>& list, const std::string& delim)
{
	return std::accumulate(list.begin(), list.end(), std::string(), [&delim](std::string& str1, const std::string& str2)
	{
		return str1.empty() ? str2 : str1 + delim + str2;
	});
}

inline std::vector<std::wstring>& split(std::vector<std::wstring>& result, const std::wstring& input, const wchar_t* delim)
{
	result.clear();
	size_t current = 0;
	size_t next = std::wstring::npos;
	do
	{
		current = next + 1;
		next = input.find_first_of(delim, next + 1);
		result.push_back(input.substr(current, next - current));
	} while (next != std::wstring::npos);
	return result;
}

inline bool starts_with(const std::wstring& wstr, const std::wstring& wsub)
{
	if (wstr.size() < wsub.size())
		return false;
	else
		return std::equal(wsub.begin(), wsub.end(), wstr.begin());
}

inline void to_lower(std::wstring& wstr)
{
	std::setlocale(LC_ALL, "");
	std::transform(wstr.begin(), wstr.end(), wstr.begin(), std::towlower);
}

// Safely apply a user-configured candidate-label format (style/label_format,
// default L"%s.") to a label. The FIRST "%s" is replaced by `label`, "%%" is a
// literal '%', and every other character -- including a stray conversion such as
// "%d" or "%n", or a trailing '%' -- is emitted verbatim. This replaces
// swprintf_s(buffer, format, label) at the label-rendering sites: `format` is
// user config, so a bogus conversion there would feed the CRT a non-existent
// argument and trip the invalid-parameter handler, crashing the in-process host
// application (or the server). Output is unbounded (no 127-wchar truncation).
inline std::wstring format_label(const std::wstring& format,
                                 const std::wstring& label)
{
	std::wstring out;
	out.reserve(format.size() + label.size());
	bool substituted = false;
	for (std::size_t i = 0; i < format.size(); ++i)
	{
		if (format[i] == L'%' && i + 1 < format.size())
		{
			const wchar_t next = format[i + 1];
			if (next == L's' && !substituted)
			{
				out += label;
				substituted = true;
				++i;
				continue;
			}
			if (next == L'%')
			{
				out += L'%';
				++i;
				continue;
			}
		}
		out += format[i];
	}
	return out;
}
