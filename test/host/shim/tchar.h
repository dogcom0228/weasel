#pragma once
// Host-test shim for MSVC <tchar.h>. Only the non-UNICODE mappings are needed.
typedef char _TCHAR;
#define _tmain main
