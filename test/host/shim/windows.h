#pragma once
// Minimal <windows.h> shim for HOST-SIDE Linux testing of Weasel's portable
// IPC text-protocol parser (see test/host/README.md). It provides only the
// Win32 typedefs and the two CRT/Win32 symbols the parse path references; it is
// NOT a general Windows compatibility layer and is never part of any shipped or
// Windows build.
#include <cstdint>
#include <cstdlib>
#include <cwchar>

typedef wchar_t WCHAR;
typedef wchar_t* LPWSTR;
typedef const wchar_t* LPCWSTR;
typedef char* LPSTR;
typedef const char* LPCSTR;
typedef unsigned int UINT;
typedef uint32_t UINT32;
typedef uint16_t UINT16;
typedef uint32_t DWORD;  // Win32 DWORD is a fixed 32-bit unsigned
typedef int BOOL;
typedef int INT;
typedef long LONG;
typedef unsigned short WORD;
typedef unsigned short USHORT;
typedef short SHORT;
typedef unsigned char BYTE;
typedef BYTE* LPBYTE;
typedef intptr_t LPARAM;
typedef uintptr_t WPARAM;
typedef intptr_t LRESULT;
typedef intptr_t LONG_PTR;
typedef void* HWND;
typedef void* HANDLE;
typedef void* HKEY;

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef NULL
#define NULL 0
#endif

struct RECT {
  long left;
  long top;
  long right;
  long bottom;
};

#define WM_APP 0x8000
#define MB_OK 0x0u
#define MB_ICONERROR 0x10u

// The parser's only Win32 UI call is a boost-archive error dialog; make it a
// no-op so a malformed payload does not pop UI during a headless test run.
inline int MessageBoxA(void*, const char*, const char*, unsigned) {
  return 0;
}

// MSVC CRT wide atoi used by ContextUpdater.
inline int _wtoi(const wchar_t* s) {
  return static_cast<int>(wcstol(s, nullptr, 10));
}
