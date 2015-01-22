/*
#pragma once

#include <windows.h>
#include <winuser.h>
#include <wtypes.h>

#include <vector>
#include <map>

#include "Platform.h"
#include "Scintilla.h"
#include "UniConversion.h"
#include "XPM.h"

HWND hClipWnd; //Clipboard window
CLIPFORMAT cfColumnSelect;
CLIPFORMAT cfLineSelect;

void Platform_Initialise(HWND hWnd)
{
	hClipWnd = hWnd;
	// There does not seem to be a real standard for indicating that the clipboard
	// contains a rectangular selection, so copy Developer Studio.
	cfColumnSelect = static_cast<CLIPFORMAT>(
		::RegisterClipboardFormat(TEXT("MSDEVColumnSelect")));

	// Likewise for line-copy (copies a full line when no text is selected)
	cfLineSelect = static_cast<CLIPFORMAT>(
		::RegisterClipboardFormat(TEXT("MSDEVLineSelect")));
	//platform::InitializeFontSubsytem();
}

void Platform_Finalise()
{
	//platform::ShutdownFontSubsytem();
}

class GlobalMemory {
	HGLOBAL hand;
public:
	void *ptr;
	GlobalMemory() : hand(0), ptr(0) {
	}
	GlobalMemory(HGLOBAL hand_) : hand(hand_), ptr(0) {
		if (hand) {
			ptr = ::GlobalLock(hand);
		}
	}
	~GlobalMemory() {
		PLATFORM_ASSERT(!ptr);
	}
	void Allocate(size_t bytes) {
		hand = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, bytes);
		if (hand) {
			ptr = ::GlobalLock(hand);
		}
	}
	HGLOBAL Unlock() {
		PLATFORM_ASSERT(ptr);
		HGLOBAL handCopy = hand;
		::GlobalUnlock(hand);
		ptr = 0;
		hand = 0;
		return handCopy;
	}
	void SetClip(UINT uFormat) {
		::SetClipboardData(uFormat, Unlock());
	}
	operator bool() const {
		return ptr != 0;
	}
	SIZE_T Size() {
		return ::GlobalSize(hand);
	}
};

int IsClipboardTextAvailable(AdditionalTextFormat fmt)
{
	if (fmt == TEXT_FORMAT_UTF8_LINE) return ::IsClipboardFormatAvailable(cfLineSelect) != 0;
	if (fmt == TEXT_FORMAT_UTF8_RECT) return ::IsClipboardFormatAvailable(cfColumnSelect) != 0;
	if (::IsClipboardFormatAvailable(CF_TEXT))
		return true;
	return ::IsClipboardFormatAvailable(CF_UNICODETEXT) != 0;
}

void SetClipboardTextUTF8(const char* text, size_t len, int additionalFormat)
{
	if (!::OpenClipboard(hClipWnd))
		return;
	::EmptyClipboard();

	GlobalMemory uniText;

	int uchars = UTF16Length(text, len);
	uniText.Allocate(2 * uchars);
	if (uniText) {
		UTF16FromUTF8(text, len, static_cast<wchar_t *>(uniText.ptr), uchars);
	}

	if (uniText) {
		uniText.SetClip(CF_UNICODETEXT);
	}

	if (additionalFormat==TEXT_FORMAT_UTF8_RECT) {
		::SetClipboardData(cfColumnSelect, 0);
	} else if (additionalFormat==TEXT_FORMAT_UTF8_LINE) {
		::SetClipboardData(cfLineSelect, 0);
	}

	::CloseClipboard();
}

int GetClipboardTextUTF8(char* text, size_t len)
{
	if (!::OpenClipboard(hClipWnd))
		return 0;

	unsigned int clipLen = 0;

	GlobalMemory memUSelection(::GetClipboardData(CF_UNICODETEXT));
	if (memUSelection) {
		wchar_t *uptr = static_cast<wchar_t *>(memUSelection.ptr);
		unsigned int bytes = memUSelection.Size();
		clipLen = UTF8Length(uptr, bytes / 2);
		if (uptr && text) {
			UTF8FromUTF16(uptr, bytes / 2, text, len);
		}
		memUSelection.Unlock();
	} else {
		// CF_UNICODETEXT not available, paste ANSI text
		GlobalMemory memSelection(::GetClipboardData(CF_TEXT));
		if (memSelection) {
			char *ptr = static_cast<char *>(memSelection.ptr);
			if (ptr) {
				unsigned int bytes = memSelection.Size();
				unsigned int len = bytes;
				for (unsigned int i = 0; i < bytes; i++) {
					if ((len == bytes) && (0 == ptr[i]))
						len = i;
				}

				// convert clipboard text to UTF-8
				wchar_t *uptr = new wchar_t[len+1];

				unsigned int ulen = ::MultiByteToWideChar(CP_ACP, 0,
				                    ptr, len, uptr, len+1);

				clipLen = UTF8Length(uptr, ulen);
				if (text) {
					// CP_UTF8 not available on Windows 95, so use UTF8FromUTF16()
					UTF8FromUTF16(uptr, ulen, text, len);
				}

				delete []uptr;
			}
			memSelection.Unlock();
		}
	}
	::CloseClipboard();

	return clipLen;
}


*/
