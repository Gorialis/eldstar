
// This code is taken in large portion from ICU 64.2, see their license at:
// https://github.com/unicode-org/icu/blob/release-64-2/icu4c/LICENSE

// © 2016 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html
/*
******************************************************************************
*
*   Copyright (C) 2001-2016, International Business Machines
*   Corporation and others.  All Rights Reserved.
*
******************************************************************************
*
* File ustrtrns.cpp
*
* Modification History:
*
*   Date        Name        Description
*   9/10/2001    Ram    Creation.
******************************************************************************
*/

/*******************************************************************************
 *
 * u_strTo* and u_strFrom* APIs
 * WCS functions moved to ustr_wcs.c for better modularization
 *
 *******************************************************************************
 */

#include <cstddef>
#include <cstdint>

#define U_IS_SURROGATE(c) (((c)&0xfffff800)==0xd800)
#define U8_IS_SINGLE(c) (((c)&0x80)==0)
#define U8_LEAD3_T1_BITS "\x20\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x10\x30\x30"
#define U8_LEAD4_T1_BITS "\x00\x00\x00\x00\x00\x00\x00\x00\x1E\x0F\x0F\x0F\x00\x00\x00\x00"
#define U8_IS_VALID_LEAD3_AND_T1(lead, t1) (U8_LEAD3_T1_BITS[(lead)&0xf]&(1<<((uint8_t)(t1)>>5)))
#define U8_IS_VALID_LEAD4_AND_T1(lead, t1) (U8_LEAD4_T1_BITS[(uint8_t)(t1)>>4]&(1<<((lead)&7)))
#define U16_LEAD(supplementary) (UChar)(((supplementary)>>10)+0xd7c0)
#define U16_TRAIL(supplementary) (UChar)(((supplementary)&0x3ff)|0xdc00)
#define U16_LENGTH(c) ((uint32_t)(c)<=0xffff ? 1 : 2)
#define U16_SURROGATE_OFFSET ((0xd800<<10UL)+0xdc00-0x10000)
#define U16_IS_SURROGATE_LEAD(c) (((c)&0x400)==0)
#define U16_IS_TRAIL(c) (((c)&0xfffffc00)==0xdc00)
#define U16_GET_SUPPLEMENTARY(lead, trail) \
    (((UChar32)(lead)<<10UL)+(UChar32)(trail)-U16_SURROGATE_OFFSET)
#define U_IS_UNICODE_NONCHAR(c) \
    ((c)>=0xfdd0 && \
     ((c)<=0xfdef || ((c)&0xfffe)==0xfffe) && (c)<=0x10ffff)

typedef char16_t UChar;
typedef int32_t UChar32;
typedef int UErrorCode;
typedef int8_t UBool;

UChar* u_strFromUTF8(UChar *dest, int32_t destCapacity, int32_t *pDestLength, const char* src, int32_t srcLength, UErrorCode *pErrorCode);
UChar32* u_strToUTF32(UChar32 *dest, int32_t destCapacity, int32_t *pDestLength, const UChar *src, int32_t srcLength, UErrorCode *pErrorCode);
