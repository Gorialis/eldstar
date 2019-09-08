
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

#include "unicode.hpp"

#define __TERMINATE_STRING(dest, destCapacity, length, pErrorCode)      \
    if(pErrorCode!=NULL && *pErrorCode <= 0) {                          \
        /* not a public function, so no complete argument checking */   \
                                                                        \
        if(length<0) {                                                  \
            /* assume that the caller handles this */                   \
        } else if(length<destCapacity) {                                \
            /* NUL-terminate the string, the NUL fits */                \
            dest[length]=0;                                             \
            /* unset the not-terminated warning but leave all others */ \
            if(*pErrorCode==-124) {                                     \
                *pErrorCode=0;                                          \
            }                                                           \
        } else if(length==destCapacity) {                               \
            /* unable to NUL-terminate, but the string itself fit - set a warning code */ \
            *pErrorCode=-124;                                           \
        } else /* length>destCapacity */ {                              \
            /* even the string itself did not fit - set an error code */ \
            *pErrorCode=15;                                             \
        }                                                               \
    }

int32_t
u_terminateUChars(UChar *dest, int32_t destCapacity, int32_t length, UErrorCode *pErrorCode) {
    __TERMINATE_STRING(dest, destCapacity, length, pErrorCode);
    return length;
}

int32_t
u_terminateUChar32s(UChar32 *dest, int32_t destCapacity, int32_t length, UErrorCode *pErrorCode) {
    __TERMINATE_STRING(dest, destCapacity, length, pErrorCode);
    return length;
}

static const UChar32
utf8_errorValue[6]={
    // Same values as UTF8_ERROR_VALUE_1, UTF8_ERROR_VALUE_2, UTF_ERROR_VALUE,
    // but without relying on the obsolete unicode/utf_old.h.
    0x15, 0x9f, 0xffff,
    0x10ffff
};

static UChar32
errorValue(int32_t count, int8_t strict) {
    if(strict>=0) {
        return utf8_errorValue[count];
    } else if(strict==-3) {
        return 0xfffd;
    } else {
        return -1;
    }
}

UChar32
utf8_nextCharSafeBody(const uint8_t *s, int32_t *pi, int32_t length, UChar32 c, UBool strict) {
    // *pi is one after byte c.
    int32_t i=*pi;
    // length can be negative for NUL-terminated strings: Read and validate one byte at a time.
    if(i==length || c>0xf4) {
        // end of string, or not a lead byte
    } else if(c>=0xf0) {
        // Test for 4-byte sequences first because
        // U8_NEXT() handles shorter valid sequences inline.
        uint8_t t1=s[i], t2, t3;
        c&=7;
        if(U8_IS_VALID_LEAD4_AND_T1(c, t1) &&
                ++i!=length && (t2=s[i]-0x80)<=0x3f &&
                ++i!=length && (t3=s[i]-0x80)<=0x3f) {
            ++i;
            c=(c<<18)|((t1&0x3f)<<12)|(t2<<6)|t3;
            // strict: forbid non-characters like U+fffe
            if(strict<=0 || !U_IS_UNICODE_NONCHAR(c)) {
                *pi=i;
                return c;
            }
        }
    } else if(c>=0xe0) {
        c&=0xf;
        if(strict!=-2) {
            uint8_t t1=s[i], t2;
            if(U8_IS_VALID_LEAD3_AND_T1(c, t1) &&
                    ++i!=length && (t2=s[i]-0x80)<=0x3f) {
                ++i;
                c=(c<<12)|((t1&0x3f)<<6)|t2;
                // strict: forbid non-characters like U+fffe
                if(strict<=0 || !U_IS_UNICODE_NONCHAR(c)) {
                    *pi=i;
                    return c;
                }
            }
        } else {
            // strict=-2 -> lenient: allow surrogates
            uint8_t t1=s[i]-0x80, t2;
            if(t1<=0x3f && (c>0 || t1>=0x20) &&
                    ++i!=length && (t2=s[i]-0x80)<=0x3f) {
                *pi=i+1;
                return (c<<12)|(t1<<6)|t2;
            }
        }
    } else if(c>=0xc2) {
        uint8_t t1=s[i]-0x80;
        if(t1<=0x3f) {
            *pi=i+1;
            return ((c-0xc0)<<6)|t1;
        }
    }  // else 0x80<=c<0xc2 is not a lead byte

    /* error handling */
    c=errorValue(i-*pi, strict);
    *pi=i;
    return c;
}

UChar*
u_strFromUTF8WithSub(UChar *dest,
              int32_t destCapacity,
              int32_t *pDestLength,
              const char* src,
              int32_t srcLength,
              UChar32 subchar, int32_t *pNumSubstitutions,
              UErrorCode *pErrorCode){
    /* args check */
    if(*pErrorCode > 0) {
        return NULL;
    }
    if( (src==NULL && srcLength!=0) || srcLength < -1 ||
        (destCapacity<0) || (dest == NULL && destCapacity > 0) ||
        subchar > 0x10ffff || U_IS_SURROGATE(subchar)
    ) {
        *pErrorCode = 1;
        return NULL;
    }

    if(pNumSubstitutions!=NULL) {
        *pNumSubstitutions=0;
    }
    UChar *pDest = dest;
    UChar *pDestLimit = dest+destCapacity;
    int32_t reqLength = 0;
    int32_t numSubstitutions=0;

    /*
     * Inline processing of UTF-8 byte sequences:
     *
     * Byte sequences for the most common characters are handled inline in
     * the conversion loops. In order to reduce the path lengths for those
     * characters, the tests are arranged in a kind of binary search.
     * ASCII (<=0x7f) is checked first, followed by the dividing point
     * between 2- and 3-byte sequences (0xe0).
     * The 3-byte branch is tested first to speed up CJK text.
     * The compiler should combine the subtractions for the two tests for 0xe0.
     * Each branch then tests for the other end of its range.
     */

    if(srcLength < 0){
        /*
         * Transform a NUL-terminated string.
         * The code explicitly checks for NULs only in the lead byte position.
         * A NUL byte in the trail byte position fails the trail byte range check anyway.
         */
        int32_t i;
        UChar32 c;
        for(i = 0; (c = (uint8_t)src[i]) != 0 && (pDest < pDestLimit);) {
            // modified copy of U8_NEXT()
            ++i;
            if(U8_IS_SINGLE(c)) {
                *pDest++=(UChar)c;
            } else {
                uint8_t __t1, __t2;
                if( /* handle U+0800..U+FFFF inline */
                        (0xe0<=(c) && (c)<0xf0) &&
                        U8_IS_VALID_LEAD3_AND_T1((c), src[i]) &&
                        (__t2=src[(i)+1]-0x80)<=0x3f) {
                    *pDest++ = (((c)&0xf)<<12)|((src[i]&0x3f)<<6)|__t2;
                    i+=2;
                } else if( /* handle U+0080..U+07FF inline */
                        ((c)<0xe0 && (c)>=0xc2) &&
                        (__t1=src[i]-0x80)<=0x3f) {
                    *pDest++ = (((c)&0x1f)<<6)|__t1;
                    ++(i);
                } else {
                    /* function call for "complicated" and error cases */
                    (c)=utf8_nextCharSafeBody((const uint8_t *)src, &(i), -1, c, -1);
                    if(c<0 && (++numSubstitutions, c = subchar) < 0) {
                        *pErrorCode = 10;
                        return NULL;
                    } else if(c<=0xFFFF) {
                        *(pDest++)=(UChar)c;
                    } else {
                        *(pDest++)=U16_LEAD(c);
                        if(pDest<pDestLimit) {
                            *(pDest++)=U16_TRAIL(c);
                        } else {
                            reqLength++;
                            break;
                        }
                    }
                }
            }
        }

        /* Pre-flight the rest of the string. */
        while((c = (uint8_t)src[i]) != 0) {
            // modified copy of U8_NEXT()
            ++i;
            if(U8_IS_SINGLE(c)) {
                ++reqLength;
            } else {
                uint8_t __t1, __t2;
                if( /* handle U+0800..U+FFFF inline */
                        (0xe0<=(c) && (c)<0xf0) &&
                        U8_IS_VALID_LEAD3_AND_T1((c), src[i]) &&
                        (__t2=src[(i)+1]-0x80)<=0x3f) {
                    ++reqLength;
                    i+=2;
                } else if( /* handle U+0080..U+07FF inline */
                        ((c)<0xe0 && (c)>=0xc2) &&
                        (__t1=src[i]-0x80)<=0x3f) {
                    ++reqLength;
                    ++(i);
                } else {
                    /* function call for "complicated" and error cases */
                    (c)=utf8_nextCharSafeBody((const uint8_t *)src, &(i), -1, c, -1);
                    if(c<0 && (++numSubstitutions, c = subchar) < 0) {
                        *pErrorCode = 10;
                        return NULL;
                    }
                    reqLength += U16_LENGTH(c);
                }
            }
        }
    } else /* srcLength >= 0 */ {
        /* Faster loop without ongoing checking for srcLength and pDestLimit. */
        int32_t i = 0;
        UChar32 c;
        for(;;) {
            /*
             * Each iteration of the inner loop progresses by at most 3 UTF-8
             * bytes and one UChar, for most characters.
             * For supplementary code points (4 & 2), which are rare,
             * there is an additional adjustment.
             */
            int32_t count = (int32_t)(pDestLimit - pDest);
            int32_t count2 = (srcLength - i) / 3;
            if(count > count2) {
                count = count2; /* min(remaining dest, remaining src/3) */
            }
            if(count < 3) {
                /*
                 * Too much overhead if we get near the end of the string,
                 * continue with the next loop.
                 */
                break;
            }

            do {
                // modified copy of U8_NEXT()
                c = (uint8_t)src[i++];
                if(U8_IS_SINGLE(c)) {
                    *pDest++=(UChar)c;
                } else {
                    uint8_t __t1, __t2;
                    if( /* handle U+0800..U+FFFF inline */
                            (0xe0<=(c) && (c)<0xf0) &&
                            ((i)+1)<srcLength &&
                            U8_IS_VALID_LEAD3_AND_T1((c), src[i]) &&
                            (__t2=src[(i)+1]-0x80)<=0x3f) {
                        *pDest++ = (((c)&0xf)<<12)|((src[i]&0x3f)<<6)|__t2;
                        i+=2;
                    } else if( /* handle U+0080..U+07FF inline */
                            ((c)<0xe0 && (c)>=0xc2) &&
                            ((i)!=srcLength) &&
                            (__t1=src[i]-0x80)<=0x3f) {
                        *pDest++ = (((c)&0x1f)<<6)|__t1;
                        ++(i);
                    } else {
                        if(c >= 0xf0 || subchar > 0xffff) {
                            // We may read up to four bytes and write up to two UChars,
                            // which we didn't account for with computing count,
                            // so we adjust it here.
                            if(--count == 0) {
                                --i;  // back out byte c
                                break;
                            }
                        }

                        /* function call for "complicated" and error cases */
                        (c)=utf8_nextCharSafeBody((const uint8_t *)src, &(i), srcLength, c, -1);
                        if(c<0 && (++numSubstitutions, c = subchar) < 0) {
                            *pErrorCode = 10;
                            return NULL;
                        } else if(c<=0xFFFF) {
                            *(pDest++)=(UChar)c;
                        } else {
                            *(pDest++)=U16_LEAD(c);
                            *(pDest++)=U16_TRAIL(c);
                        }
                    }
                }
            } while(--count > 0);
        }

        while(i < srcLength && (pDest < pDestLimit)) {
            // modified copy of U8_NEXT()
            c = (uint8_t)src[i++];
            if(U8_IS_SINGLE(c)) {
                *pDest++=(UChar)c;
            } else {
                uint8_t __t1, __t2;
                if( /* handle U+0800..U+FFFF inline */
                        (0xe0<=(c) && (c)<0xf0) &&
                        ((i)+1)<srcLength &&
                        U8_IS_VALID_LEAD3_AND_T1((c), src[i]) &&
                        (__t2=src[(i)+1]-0x80)<=0x3f) {
                    *pDest++ = (((c)&0xf)<<12)|((src[i]&0x3f)<<6)|__t2;
                    i+=2;
                } else if( /* handle U+0080..U+07FF inline */
                        ((c)<0xe0 && (c)>=0xc2) &&
                        ((i)!=srcLength) &&
                        (__t1=src[i]-0x80)<=0x3f) {
                    *pDest++ = (((c)&0x1f)<<6)|__t1;
                    ++(i);
                } else {
                    /* function call for "complicated" and error cases */
                    (c)=utf8_nextCharSafeBody((const uint8_t *)src, &(i), srcLength, c, -1);
                    if(c<0 && (++numSubstitutions, c = subchar) < 0) {
                        *pErrorCode = 10;
                        return NULL;
                    } else if(c<=0xFFFF) {
                        *(pDest++)=(UChar)c;
                    } else {
                        *(pDest++)=U16_LEAD(c);
                        if(pDest<pDestLimit) {
                            *(pDest++)=U16_TRAIL(c);
                        } else {
                            reqLength++;
                            break;
                        }
                    }
                }
            }
        }

        /* Pre-flight the rest of the string. */
        while(i < srcLength) {
            // modified copy of U8_NEXT()
            c = (uint8_t)src[i++];
            if(U8_IS_SINGLE(c)) {
                ++reqLength;
            } else {
                uint8_t __t1, __t2;
                if( /* handle U+0800..U+FFFF inline */
                        (0xe0<=(c) && (c)<0xf0) &&
                        ((i)+1)<srcLength &&
                        U8_IS_VALID_LEAD3_AND_T1((c), src[i]) &&
                        (__t2=src[(i)+1]-0x80)<=0x3f) {
                    ++reqLength;
                    i+=2;
                } else if( /* handle U+0080..U+07FF inline */
                        ((c)<0xe0 && (c)>=0xc2) &&
                        ((i)!=srcLength) &&
                        (__t1=src[i]-0x80)<=0x3f) {
                    ++reqLength;
                    ++(i);
                } else {
                    /* function call for "complicated" and error cases */
                    (c)=utf8_nextCharSafeBody((const uint8_t *)src, &(i), srcLength, c, -1);
                    if(c<0 && (++numSubstitutions, c = subchar) < 0) {
                        *pErrorCode = 10;
                        return NULL;
                    }
                    reqLength += U16_LENGTH(c);
                }
            }
        }
    }

    reqLength+=(int32_t)(pDest - dest);

    if(pNumSubstitutions!=NULL) {
        *pNumSubstitutions=numSubstitutions;
    }

    if(pDestLength){
        *pDestLength = reqLength;
    }

    /* Terminate the buffer */
    u_terminateUChars(dest,destCapacity,reqLength,pErrorCode);

    return dest;
}

UChar*
u_strFromUTF8(UChar *dest,
              int32_t destCapacity,
              int32_t *pDestLength,
              const char* src,
              int32_t srcLength,
              UErrorCode *pErrorCode){
    return u_strFromUTF8WithSub(
            dest, destCapacity, pDestLength,
            src, srcLength,
            -1, NULL,
            pErrorCode);
}

UChar32*
u_strToUTF32WithSub(UChar32 *dest,
             int32_t destCapacity,
             int32_t *pDestLength,
             const UChar *src,
             int32_t srcLength,
             UChar32 subchar, int32_t *pNumSubstitutions,
             UErrorCode *pErrorCode) {
    const UChar *srcLimit;
    UChar32 ch;
    UChar ch2;
    UChar32 *destLimit;
    UChar32 *pDest;
    int32_t reqLength;
    int32_t numSubstitutions;

    /* args check */
    if(*pErrorCode > 0){
        return NULL;
    }
    if( (src==NULL && srcLength!=0) || srcLength < -1 ||
        (destCapacity<0) || (dest == NULL && destCapacity > 0) ||
        subchar > 0x10ffff || U_IS_SURROGATE(subchar)
    ) {
        *pErrorCode = 1;
        return NULL;
    }

    if(pNumSubstitutions != NULL) {
        *pNumSubstitutions = 0;
    }

    pDest = dest;
    destLimit = (dest!=NULL)?(dest + destCapacity):NULL;
    reqLength = 0;
    numSubstitutions = 0;

    if(srcLength < 0) {
        /* simple loop for conversion of a NUL-terminated BMP string */
        while((ch=*src) != 0 && !U_IS_SURROGATE(ch)) {
            ++src;
            if(pDest < destLimit) {
                *pDest++ = ch;
            } else {
                ++reqLength;
            }
        }
        srcLimit = src;
        if(ch != 0) {
            /* "complicated" case, find the end of the remaining string */
            while(*++srcLimit != 0) {}
        }
    } else {
        srcLimit = (src!=NULL)?(src + srcLength):NULL;
    }

    /* convert with length */
    while(src < srcLimit) {
        ch = *src++;
        if(!U_IS_SURROGATE(ch)) {
            /* write or count ch below */
        } else if(U16_IS_SURROGATE_LEAD(ch) && src < srcLimit && U16_IS_TRAIL(ch2 = *src)) {
            ++src;
            ch = U16_GET_SUPPLEMENTARY(ch, ch2);
        } else if((ch = subchar) < 0) {
            /* unpaired surrogate */
            *pErrorCode = 10;
            return NULL;
        } else {
            ++numSubstitutions;
        }
        if(pDest < destLimit) {
            *pDest++ = ch;
        } else {
            ++reqLength;
        }
    }

    reqLength += (int32_t)(pDest - dest);
    if(pDestLength) {
        *pDestLength = reqLength;
    }
    if(pNumSubstitutions != NULL) {
        *pNumSubstitutions = numSubstitutions;
    }

    /* Terminate the buffer */
    u_terminateUChar32s(dest, destCapacity, reqLength, pErrorCode);

    return dest;
}

UChar32*
u_strToUTF32(UChar32 *dest,
             int32_t destCapacity,
             int32_t *pDestLength,
             const UChar *src,
             int32_t srcLength,
             UErrorCode *pErrorCode) {
    return u_strToUTF32WithSub(
            dest, destCapacity, pDestLength,
            src, srcLength,
            -1, NULL,
            pErrorCode);
}
