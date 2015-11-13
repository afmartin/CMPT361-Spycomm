/*
 * Copyright (c) 2008, 2011, 2013, 2015 Ryan Vogt <rvogt@ualberta.ca>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef __COMMON_H__
#define __COMMON_H__

#include <sys/types.h>
#include <limits.h>
#include <stdint.h>

#ifndef __BYTE_T_DEFINED__
#define __BYTE_T_DEFINED__
typedef unsigned char byte_t;
#endif

/*
 * If debugging is defined, include the file name and line number of the
 * invocation.  Otherwise, leave that information out.
 */
#ifndef DEBUGOUTPUT
#define DEBUGOUTPUT stderr
#endif
#ifdef DEBUGGING
#define ERROR_CODE(target, wasErr, code)                      \
    do {                                                      \
      fprintf(DEBUGOUTPUT, "Error code %d", (code));          \
      fprintf(DEBUGOUTPUT, " [%s:%d]\n", __FILE__, __LINE__); \
      fflush(DEBUGOUTPUT);                                    \
      wasErr = (code);                                        \
      goto target;                                            \
    } while (0)
#else
#define ERROR_CODE(target, wasErr, code)                      \
    do {                                                      \
      wasErr = (code);                                        \
      goto target;                                            \
    } while (0)
#endif

/*
 * The smaller of the upper bounds of a size_t and an ssize_t.
 */
#if SIZE_MAX < SSIZE_MAX
#define SIZE_MAX_SSIZE ((size_t)(SIZE_MAX))
#else
#define SIZE_MAX_SSIZE ((size_t)(SSIZE_MAX))
#endif

/*
 * Functions for working with size_t values that could possibly be larger than
 * 64 bits:
 *
 * sizeRightShift64: returns the value of the given size_t right-shifted
 * (non-circular) by 64 bits.
 *
 * sizeIsLargerThan64: returns whether the given size_t has a value that is
 * larger than the largest 64-bit unsigned value (i.e., 2^{64}-1).
 */
static inline size_t sizeRightShift64(size_t inputSize)
{
#if SIZE_MAX > UINT64_MAX
  return inputSize >> 64;
#else
  return 0;
#endif
}

static inline int sizeIsLargerThan64(size_t inputSize)
{
#if SIZE_MAX > UINT64_MAX
  return (inputSize > UINT64_MAX);
#else
  return 0;
#endif
}

/*
 * Write the given integer value into the given address in memory in big-endian
 * order.  The BIG_ENDIAN_32 and BIG_ENDIAN_64 variables can be defined on a
 * big-endian system to improve performance.
 */
static inline void putBigEnd32(byte_t *location, uint32_t value)
{
#ifdef BIG_ENDIAN_32
  *((uint32_t *)location) = value;
#else
  location[0] = (byte_t)((value >> 24) & 0xFF);
  location[1] = (byte_t)((value >> 16) & 0xFF);
  location[2] = (byte_t)((value >>  8) & 0xFF);
  location[3] = (byte_t)((value      ) & 0xFF);
#endif
}

static inline void putBigEnd64(byte_t *location, uint64_t value)
{
#ifdef BIG_ENDIAN_64
  *((uint64_t *)location) = value;
#else
  location[0] = (byte_t)((value >> 56) & 0xFF);
  location[1] = (byte_t)((value >> 48) & 0xFF);
  location[2] = (byte_t)((value >> 40) & 0xFF);
  location[3] = (byte_t)((value >> 32) & 0xFF);
  location[4] = (byte_t)((value >> 24) & 0xFF);
  location[5] = (byte_t)((value >> 16) & 0xFF);
  location[6] = (byte_t)((value >>  8) & 0xFF);
  location[7] = (byte_t)((value      ) & 0xFF);
#endif
}

/*
 * Write the given integer value into the given address in memory in
 * little-endian order.  The LITTLE_ENDIAN_32 and LITTLE_ENDIAN_64 variables
 * can be defined on a little-endian system to improve performance.
 */
static inline void putLittleEnd32(byte_t *location, uint32_t value)
{
#ifdef LITTLE_ENDIAN_32
  *((uint32_t *)location) = value;
#else
  location[0] = (byte_t)((value      ) & 0xFF);
  location[1] = (byte_t)((value >>  8) & 0xFF);
  location[2] = (byte_t)((value >> 16) & 0xFF);
  location[3] = (byte_t)((value >> 24) & 0xFF);
#endif
}

static inline void putLittleEnd64(byte_t *location, uint64_t value)
{
#ifdef LITTLE_ENDIAN_64
  *((uint64_t *)location) = value;
#else
  location[0] = (byte_t)((value      ) & 0xFF);
  location[1] = (byte_t)((value >>  8) & 0xFF);
  location[2] = (byte_t)((value >> 16) & 0xFF);
  location[3] = (byte_t)((value >> 24) & 0xFF);
  location[4] = (byte_t)((value >> 32) & 0xFF);
  location[5] = (byte_t)((value >> 40) & 0xFF);
  location[6] = (byte_t)((value >> 48) & 0xFF);
  location[7] = (byte_t)((value >> 56) & 0xFF);
#endif
}

/*
 * Interpret the four or eight bytes at the given memory address as a
 * big-endian integer and return its value.  The BIG_ENDIAN_32 and
 * BIG_ENDIAN_64 variables can be defined on a big-endian system to improve
 * performance.
 */
static inline uint32_t getBigEnd32(const byte_t *location)
{
#ifdef BIG_ENDIAN_32
  return *((uint32_t *)location);
#else
  return
    (((uint32_t)(location[0])) << 24) |
    (((uint32_t)(location[1])) << 16) |
    (((uint32_t)(location[2])) <<  8) |
    (((uint32_t)(location[3]))      );
#endif
}

static inline uint64_t getBigEnd64(const byte_t *location)
{
#ifdef BIG_ENDIAN_64
  return *((uint64_t *)location);
#else
  return
    (((uint64_t)(location[0])) << 56) |
    (((uint64_t)(location[1])) << 48) |
    (((uint64_t)(location[2])) << 40) |
    (((uint64_t)(location[3])) << 32) |
    (((uint64_t)(location[4])) << 24) |
    (((uint64_t)(location[5])) << 16) |
    (((uint64_t)(location[6])) <<  8) |
    (((uint64_t)(location[7]))      );
#endif
}

/*
 * Interpret the four or eight bytes at the given memory address as a
 * little-endian integer and return its value.  The LITTLE_ENDIAN_32 and
 * LITTLE_ENDIAN_64 variables can be defined on a little-endian system to
 * improve performance.
 */
static inline uint32_t getLittleEnd32(const byte_t *location)
{
#ifdef LITTLE_ENDIAN_32
  return *((uint32_t *)location);
#else
  return
    (((uint32_t)(location[0]))      ) |
    (((uint32_t)(location[1])) <<  8) |
    (((uint32_t)(location[2])) << 16) |
    (((uint32_t)(location[3])) << 24);
#endif
}

static inline uint64_t getLittleEnd64(const byte_t *location)
{
#ifdef LITTLE_ENDIAN_64
  return *((uint64_t *)location);
#else
  return
    (((uint64_t)(location[0]))      ) |
    (((uint64_t)(location[1])) <<  8) |
    (((uint64_t)(location[2])) << 16) |
    (((uint64_t)(location[3])) << 24) |
    (((uint64_t)(location[4])) << 32) |
    (((uint64_t)(location[5])) << 40) |
    (((uint64_t)(location[6])) << 48) |
    (((uint64_t)(location[7])) << 56);
#endif
}

/*
 * Compute and return the given circular bit shifts.
 */
static inline uint32_t circShiftLeft32(uint32_t value, int amount)
{
  if ((amount &= 31) == 0)
    return value;
  return (value << amount) | (value >> (32 - amount));
}

static inline uint64_t circShiftLeft64(uint64_t value, int amount)
{
  if ((amount &= 63) == 0)
    return value;
  return (value << amount) | (value >> (64 - amount));
}

static inline uint32_t circShiftRight32(uint32_t value, int amount)
{
  if ((amount &= 31) == 0)
    return value;
  return (value >> amount) | (value << (32 - amount));
}

static inline uint64_t circShiftRight64(uint64_t value, int amount)
{
  if ((amount &= 63) == 0)
    return value;
  return (value >> amount) | (value << (64 - amount));
}

#endif
