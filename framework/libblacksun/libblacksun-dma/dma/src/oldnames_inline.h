#pragma once
#ifndef _OLDNAMES_INLINE
#define _OLDNAMES_INLINE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define __forceinline __attribute__((always_inline)) inline
#define __cdecl

namespace oldinline
{

	__forceinline wchar_t *emu_wcsncpy(wchar_t *dest, const wchar_t *src, size_t n)
	{
		unsigned short c;
		wchar_t *const s = dest;

		--dest;

		if (n >= 4)
		{
			size_t n4 = n >> 2;

			for (;;)
			{
				c = *src++;
				*++dest = c;
				if (c == L'\0')
					break;
				c = *src++;
				*++dest = c;
				if (c == L'\0')
					break;
				c = *src++;
				*++dest = c;
				if (c == L'\0')
					break;
				c = *src++;
				*++dest = c;
				if (c == L'\0')
					break;
				if (--n4 == 0)
					goto last_chars;
			}
			n = n - (dest - s) - 1;
			if (n == 0)
				return s;
			goto zero_fill;
		}

	last_chars:
		n &= 3;
		if (n == 0)
			return s;

		do
		{
			c = *src++;
			*++dest = c;
			if (--n == 0)
				return s;
		} while (c != L'\0');

	zero_fill:
		do
			*++dest = L'\0';
		while (--n > 0);

		return s;
	}
	__forceinline size_t emu_wcslen(const wchar_t *s)
	{
		size_t len = 0;

		while (s[len] != L'\0')
		{
			if (s[++len] == L'\0')
				return len;
			if (s[++len] == L'\0')
				return len;
			if (s[++len] == L'\0')
				return len;
			++len;
		}

		return len;
	}
	__forceinline size_t emu_wcsnlen(const wchar_t *s, size_t maxlen)
	{
		size_t len = 0;

		while (maxlen > 0 && s[len] != L'\0')
		{
			++len;
			if (--maxlen == 0 || s[len] == L'\0')
				return len;
			++len;
			if (--maxlen == 0 || s[len] == L'\0')
				return len;
			++len;
			if (--maxlen == 0 || s[len] == L'\0')
				return len;
			++len;
			--maxlen;
		}

		return len;
	}
	__forceinline size_t emu_strlen(const char *str)
	{
		const char *char_ptr;
		const unsigned long int *longword_ptr;
		unsigned long int longword, himagic, lomagic;

		/* Handle the first few characters by reading one character at a time.
		   Do this until CHAR_PTR is aligned on a longword boundary.  */
		for (char_ptr = str; ((unsigned long int)char_ptr & (sizeof(longword) - 1)) != 0;
			 ++char_ptr)
			if (*char_ptr == '\0')
				return char_ptr - str;

		/* All these elucidatory comments refer to 4-byte longwords,
		   but the theory applies equally well to 8-byte longwords.  */

		longword_ptr = (unsigned long int *)char_ptr;

		/* Bits 31, 24, 16, and 8 of this number are zero.  Call these bits
		   the "holes."  Note that there is a hole just to the left of
		   each byte, with an extra at the end:

		   bits:  01111110 11111110 11111110 11111111
		   bytes: AAAAAAAA BBBBBBBB CCCCCCCC DDDDDDDD

		   The 1-bits make sure that carries propagate to the next 0-bit.
		   The 0-bits provide holes for carries to fall into.  */
		himagic = 0x80808080L;
		lomagic = 0x01010101L;
		if (sizeof(longword) > 4)
		{
			/* 64-bit version of the magic.  */
			/* Do the shift in two steps to avoid a warning if long has 32 bits.  */
			himagic = ((himagic << 16) << 16) | himagic;
			lomagic = ((lomagic << 16) << 16) | lomagic;
		}
		if (sizeof(longword) > 8)
			return 0;

		/* Instead of the traditional loop which tests each character,
		   we will test a longword at a time.  The tricky part is testing
		   if *any of the four* bytes in the longword in question are zero.  */
		for (;;)
		{
			longword = *longword_ptr++;

			if (((longword - lomagic) & ~longword & himagic) != 0)
			{
				/* Which of the bytes was the zero?  If none of them were, it was
				   a misfire; continue the search.  */

				const char *cp = (const char *)(longword_ptr - 1);

				if (cp[0] == 0)
					return cp - str;
				if (cp[1] == 0)
					return cp - str + 1;
				if (cp[2] == 0)
					return cp - str + 2;
				if (cp[3] == 0)
					return cp - str + 3;
				if (sizeof(longword) > 4)
				{
					if (cp[4] == 0)
						return cp - str + 4;
					if (cp[5] == 0)
						return cp - str + 5;
					if (cp[6] == 0)
						return cp - str + 6;
					if (cp[7] == 0)
						return cp - str + 7;
				}
			}
		}
		return 0;
	}
	__forceinline size_t emu_strnlen(const char *str, size_t maxlen)
	{
		const char *char_ptr, *end_ptr = str + maxlen;
		const unsigned long int *longword_ptr;
		unsigned long int longword, himagic, lomagic;

		if (maxlen == 0)
			return 0;

		if ((end_ptr < str))
			end_ptr = (const char *)~0UL;

		/* Handle the first few characters by reading one character at a time.
		   Do this until CHAR_PTR is aligned on a longword boundary.  */
		for (char_ptr = str; ((unsigned long int)char_ptr & (sizeof(longword) - 1)) != 0;
			 ++char_ptr)
			if (*char_ptr == '\0')
			{
				if (char_ptr > end_ptr)
					char_ptr = end_ptr;
				return char_ptr - str;
			}

		/* All these elucidatory comments refer to 4-byte longwords,
		   but the theory applies equally well to 8-byte longwords.  */

		longword_ptr = (unsigned long int *)char_ptr;

		/* Bits 31, 24, 16, and 8 of this number are zero.  Call these bits
		   the "holes."  Note that there is a hole just to the left of
		   each byte, with an extra at the end:

		   bits:  01111110 11111110 11111110 11111111
		   bytes: AAAAAAAA BBBBBBBB CCCCCCCC DDDDDDDD

		   The 1-bits make sure that carries propagate to the next 0-bit.
		   The 0-bits provide holes for carries to fall into.  */
		himagic = 0x80808080L;
		lomagic = 0x01010101L;
		if (sizeof(longword) > 4)
		{
			/* 64-bit version of the magic.  */
			/* Do the shift in two steps to avoid a warning if long has 32 bits.  */
			himagic = ((himagic << 16) << 16) | himagic;
			lomagic = ((lomagic << 16) << 16) | lomagic;
		}
		if (sizeof(longword) > 8)
			return 0;

		/* Instead of the traditional loop which tests each character,
		   we will test a longword at a time.  The tricky part is testing
		   if *any of the four* bytes in the longword in question are zero.  */
		while (longword_ptr < (unsigned long int *)end_ptr)
		{
			/* We tentatively exit the loop if adding MAGIC_BITS to
		   LONGWORD fails to change any of the hole bits of LONGWORD.

		   1) Is this safe?  Will it catch all the zero bytes?
		   Suppose there is a byte with all zeros.  Any carry bits
		   propagating from its left will fall into the hole at its
		   least significant bit and stop.  Since there will be no
		   carry from its most significant bit, the LSB of the
		   byte to the left will be unchanged, and the zero will be
		   detected.

		   2) Is this worthwhile?  Will it ignore everything except
		   zero bytes?  Suppose every byte of LONGWORD has a bit set
		   somewhere.  There will be a carry into bit 8.  If bit 8
		   is set, this will carry into bit 16.  If bit 8 is clear,
		   one of bits 9-15 must be set, so there will be a carry
		   into bit 16.  Similarly, there will be a carry into bit
		   24.  If one of bits 24-30 is set, there will be a carry
		   into bit 31, so all of the hole bits will be changed.

		   The one misfire occurs when bits 24-30 are clear and bit
		   31 is set; in this case, the hole at bit 31 is not
		   changed.  If we had access to the processor carry flag,
		   we could close this loophole by putting the fourth hole
		   at bit 32!

		   So it ignores everything except 128's, when they're aligned
		   properly.  */

			longword = *longword_ptr++;

			if ((longword - lomagic) & himagic)
			{
				/* Which of the bytes was the zero?  If none of them were, it was
				   a misfire; continue the search.  */

				const char *cp = (const char *)(longword_ptr - 1);

				char_ptr = cp;
				if (cp[0] == 0)
					break;
				char_ptr = cp + 1;
				if (cp[1] == 0)
					break;
				char_ptr = cp + 2;
				if (cp[2] == 0)
					break;
				char_ptr = cp + 3;
				if (cp[3] == 0)
					break;
				if (sizeof(longword) > 4)
				{
					char_ptr = cp + 4;
					if (cp[4] == 0)
						break;
					char_ptr = cp + 5;
					if (cp[5] == 0)
						break;
					char_ptr = cp + 6;
					if (cp[6] == 0)
						break;
					char_ptr = cp + 7;
					if (cp[7] == 0)
						break;
				}
			}
			char_ptr = end_ptr;
		}

		if (char_ptr > end_ptr)
			char_ptr = end_ptr;
		return char_ptr - str;
	}
	__forceinline char *emu_strncpy(char *s1, const char *s2, size_t n)
	{
		size_t size = emu_strnlen(s2, n);
		if (size != n)
			memset(s1 + size, '\0', n - size);
		return (char *)memcpy(s1, s2, size);
	}
	__forceinline char *emu_strcpy(char *dest, const char *src)
	{
		char c;
		char *wcp;
		wcp = dest;

		do
		{
			c = *src++;
			*wcp++ = c;
		} while (c != '\0');

		return dest;
	}
	__forceinline wchar_t *emu_wcscpy(wchar_t *dest, const wchar_t *src)
	{
		unsigned short c;
		wchar_t *wcp;
		wcp = dest;

		do
		{
			c = *src++;
			*wcp++ = c;
		} while (c != L'\0');

		return dest;
	}
	__forceinline int emu_strcmp(const char *p1, const char *p2)
	{
		const unsigned char *s1 = (const unsigned char *)p1;
		const unsigned char *s2 = (const unsigned char *)p2;
		unsigned char c1, c2;

		do
		{
			c1 = (unsigned char)*s1++;
			c2 = (unsigned char)*s2++;
			if (c1 == '\0')
				return c1 - c2;
		} while (c1 == c2);

		return c1 - c2;
	}
	__forceinline int emu_strncmp(const char *s1, const char *s2, size_t n)
	{
		unsigned char c1 = '\0';
		unsigned char c2 = '\0';

		if (n >= 4)
		{
			size_t n4 = n >> 2;
			do
			{
				c1 = (unsigned char)*s1++;
				c2 = (unsigned char)*s2++;
				if (c1 == '\0' || c1 != c2)
					return c1 - c2;
				c1 = (unsigned char)*s1++;
				c2 = (unsigned char)*s2++;
				if (c1 == '\0' || c1 != c2)
					return c1 - c2;
				c1 = (unsigned char)*s1++;
				c2 = (unsigned char)*s2++;
				if (c1 == '\0' || c1 != c2)
					return c1 - c2;
				c1 = (unsigned char)*s1++;
				c2 = (unsigned char)*s2++;
				if (c1 == '\0' || c1 != c2)
					return c1 - c2;
			} while (--n4 > 0);
			n &= 3;
		}

		while (n > 0)
		{
			c1 = (unsigned char)*s1++;
			c2 = (unsigned char)*s2++;
			if (c1 == '\0' || c1 != c2)
				return c1 - c2;
			n--;
		}

		return c1 - c2;
	}

	__forceinline int emu_islower(int c)
	{
		return (c >= 'a' && c <= 'z');
	}

	__forceinline int emu_toupper(int c)
	{
		return ((c >= 'a' && c <= 'z') ? ((c)-0x20) : (c));
	}
	__forceinline int emu_stricmp(const char *str1, const char *str2)
	{
		const unsigned char *ustr1 = (const unsigned char *)str1;
		const unsigned char *ustr2 = (const unsigned char *)str2;

		while (*ustr1 && emu_toupper(*ustr1) == emu_toupper(*ustr2))
		{
			ustr1++;
			ustr2++;
		}
		return emu_toupper(*ustr1) - emu_toupper(*ustr2);
	}
	__forceinline int emu_strnicmp(const char *s1, const char *s2, size_t n)
	{

		if (n == 0)
			return 0;
		do
		{
			if (emu_toupper(*s1) != emu_toupper(*s2++))
				return emu_toupper(*(unsigned const char *)s1) - emu_toupper(*(unsigned const char *)--s2);
			if (*s1++ == 0)
				break;
		} while (--n != 0);
		return 0;
	}

	__forceinline wchar_t __cdecl emu_towupper(wchar_t c)
	{
		if (c >= 'a' && c <= 'z')
			return (c + ('A' - 'a'));
		return (c);
	}
	__forceinline int emu_wcsnicmp(const wchar_t *cs, const wchar_t *ct, size_t count)
	{
		if (count == 0)
			return 0;
		do
		{
			if (emu_towupper(*cs) != emu_towupper(*ct++))
				return emu_towupper(*cs) - emu_towupper(*--ct);
			if (*cs++ == 0)
				break;
		} while (--count != 0);
		return 0;
	}
	__forceinline int emu_wcsicmp(const wchar_t *cs, const wchar_t *ct)
	{
		while (emu_towupper(*cs) == emu_towupper(*ct))
		{
			if (*cs == 0)
				return 0;
			cs++;
			ct++;
		}
		return emu_towupper(*cs) - emu_towupper(*ct);
	}
}
#undef wcsncpy
#undef strlen
#undef wcslen
#undef wcsnlen
#undef strnlen
#undef strncpy
#undef wcscpy
#undef strcmp
#undef strncmp
#undef stricmp
#undef strnicmp
#undef iswctype
#undef towupper
#undef wcsnicmp
#undef islower
#undef wcsicmp
#undef strcpy

#define emu_wcsncpy oldinline::emu_wcsncpy
#define emu_strlen oldinline::emu_strlen
#define emu_wcslen oldinline::emu_wcslen
#define emu_wcsnlen oldinline::emu_wcsnlen
#define emu_strnlen oldinline::emu_strnlen
#define emu_strncpy oldinline::emu_strncpy
#define emu_wcscpy oldinline::emu_wcscpy
#define emu_strcmp oldinline::emu_strcmp
#define emu_strncmp oldinline::emu_strncmp
#define emu_stricmp oldinline::emu_stricmp
#define emu_strnicmp oldinline::emu_strnicmp
#define emu_iswctype oldinline::emu_iswctype
#define emu_towupper oldinline::emu_towupper
#define emu_wcsnicmp oldinline::emu_wcsnicmp
#define emu_islower oldinline::emu_islower
#define emu_wcsicmp oldinline::emu_wcsicmp
#define emu_strcpy oldinline::emu_strcpy

#define wcsncpy emu_wcsncpy
#define strlen emu_strlen
#define wcslen emu_wcslen
#define wcsnlen emu_wcsnlen
#define strnlen emu_strnlen
#define strncpy emu_strncpy
#define wcscpy emu_wcscpy
#define strcmp emu_strcmp
#define strncmp emu_strncmp
#define stricmp emu_stricmp
#define strnicmp emu_strnicmp
#define iswctype emu_iswctype
#define towupper emu_towupper
#define wcsnicmp emu_wcsnicmp
#define islower emu_islower
#define wcsicmp emu_wcsicmp
#define strcpy emu_strcpy

#endif