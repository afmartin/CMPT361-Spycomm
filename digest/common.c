/*
 * Copyright (c) 2008, 2011, 2015 Ryan Vogt <rvogt@ualberta.ca>
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

#include <sys/types.h>
#include <stdio.h>
#include <stdint.h>
#include "common.h"

/*
 * To keep cryptographic operations sane, we assume 8-bit bytes. This software
 * was never meant to be run somewhere without 8-bit bytes.
 */
#if CHAR_BIT != 8
#error This library is not compatible with non-octet-byte machines.
#endif

/*
 * Tests if the given bytes are in little-endian or big-endian order.
 * Returns 1 and prints the appropriate compiler flag if they are; otherwise,
 * returns 0.
 */
static int testEndian(byte_t *b, size_t len, int spaceBefore);

static int testEndian(byte_t *b, size_t len, int spaceBefore)
{
  size_t i;
  int isLE, isBE;

  isLE = isBE = 1;
  for (i = 0; i < len; i++) {
    if (b[i] != (byte_t)(i + 1))
      isBE = 0;
    if (b[i] != (byte_t)(len - i))
      isLE = 0;
  }

  if (isLE)
    printf("%s-DLITTLE_ENDIAN_%d", (spaceBefore ? " " : ""), (int)(len*8));
  else if (isBE)
    printf("%s-DBIG_ENDIAN_%d", (spaceBefore ? " " : ""), (int)(len*8));
  return (isLE | isBE);
}
