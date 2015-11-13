/*
 * Copyright (c) 2011, 2015 Ryan Vogt <rvogt@ualberta.ca>
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

#ifndef __MD5_H__
#define __MD5_H__

#include <stdint.h>
#include "common.h"

/*
 * The number of bytes output in a MD5 hash, and the internal block size of
 * MD5
 */
#define MD5_DIGEST_BYTES (16)
#define MD5_BLOCK_BYTES  (64)

struct md5CTX
{
  uint32_t h[4];
  uint64_t bytesProcessed;
  byte_t block[MD5_BLOCK_BYTES];
  uint8_t bytesInBlock;
};

/*
 * Starts a new MD5 operation
 */
void md5Start(void *c);

/*
 * Returns 0 on success or a negative value if the message size has exceeded
 * the maximal MD5 message length.
 */
int md5Add(void *c, const byte_t *bytes, size_t numBytes);

/*
 * Computes the MD5 hash of the message
 */
void md5End(void *c, byte_t *digest);

#endif
