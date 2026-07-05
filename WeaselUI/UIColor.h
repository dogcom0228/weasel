#pragma once
// Shared color / alpha helpers for WeaselUI.
//
// Weasel stores colors as 0xAARRGGBB (high byte = alpha) in a COLORREF-typed
// int. These macros were previously private to WeaselPanel.cpp; the layout code
// duplicated the raw `(color & 0xff000000)` alpha test instead of using them.
//
// The macro bodies are pure text substitution; their expansions reference
// COLORREF / GetR|G|BValue / Gdiplus, so include this header AFTER the platform
// and GDI+ headers (every WeaselUI TU already pulls in <windows.h> via stdafx.h
// and <gdiplus.h> via Layout.h before this).

#define COLORTRANSPARENT(color) ((color & 0xff000000) == 0)
#define COLORNOTTRANSPARENT(color) ((color & 0xff000000) != 0)
#define GDPCOLOR_FROM_COLORREF(color)                                \
  Gdiplus::Color::MakeARGB(((color >> 24) & 0xff), GetRValue(color), \
                           GetGValue(color), GetBValue(color))
#define HALF_ALPHA_COLOR(color) \
  ((((color & 0xff000000) >> 25) & 0xff) << 24) | (color & 0x00ffffff)
