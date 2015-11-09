/*
 * Copyright (c) 2011, 2013, 2015 Ryan Vogt <rvogt@ualberta.ca>
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

#include <stdint.h>
#include <string.h>
#include "common.h"
#include "md5.h"

#define MD5_MAX_MESSAGE_BYTES ((uint64_t)(0x1FFFFFFFFFFFFFFFLLU))

/* The rotation amount in each of the 64 rounds */
static const int ROTATION[64] =
  {7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
   5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
   4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
   6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};

/* The 64 elements in the sine table, T in RFC 1321 */
static const int SINE[64] =
  {0xD76AA478, 0xE8C7B756, 0x242070DB, 0xC1BDCEEE, 0xF57C0FAF, 0x4787C62A,
   0xA8304613, 0xFD469501, 0x698098D8, 0x8B44F7AF, 0xFFFF5BB1, 0x895CD7BE,
   0x6B901122, 0xFD987193, 0xA679438E, 0x49B40821, 0xF61E2562, 0xC040B340,
   0x265E5A51, 0xE9B6C7AA, 0xD62F105D, 0x02441453, 0xD8A1E681, 0xE7D3FBC8,
   0x21E1CDE6, 0xC33707D6, 0xF4D50D87, 0x455A14ED, 0xA9E3E905, 0xFCEFA3F8,
   0x676F02D9, 0x8D2A4C8A, 0xFFFA3942, 0x8771F681, 0x6D9D6122, 0xFDE5380C,
   0xA4BEEA44, 0x4BDECFA9, 0xF6BB4B60, 0xBEBFBC70, 0x289B7EC6, 0xEAA127FA,
   0xD4EF3085, 0x04881D05, 0xD9D4D039, 0xE6DB99E5, 0x1FA27CF8, 0xC4AC5665,
   0xF4292244, 0x432AFF97, 0xAB9423A7, 0xFC93A039, 0x655B59C3, 0x8F0CCC92,
   0xFFEFF47D, 0x85845DD1, 0x6FA87E4F, 0xFE2CE6E0, 0xA3014314, 0x4E0811A1,
   0xF7537E82, 0xBD3AF235, 0x2AD7D2BB, 0xEB86D391};

static void md5ProcessBlock(struct md5CTX *ctx, const byte_t *block);

void md5Start(void *c)
{
  struct md5CTX *ctx = (struct md5CTX *)c;
  ctx->h[0] = (uint32_t)0x67452301;
  ctx->h[1] = (uint32_t)0xEFCDAB89;
  ctx->h[2] = (uint32_t)0x98BADCFE;
  ctx->h[3] = (uint32_t)0x10325476;
  ctx->bytesProcessed = 0;
  ctx->bytesInBlock = 0;
}

int md5Add(void *c, const byte_t *bytes, size_t numBytes)
{
  struct md5CTX *ctx = (struct md5CTX *)c;
  size_t toFillBlock, addToCtx;

  /* Update the number of bytes processed by the context */
  if (sizeIsLargerThan64(numBytes) ||
      UINT64_MAX - (uint64_t)numBytes < ctx->bytesProcessed)
    return -1;
  ctx->bytesProcessed = (uint64_t)numBytes + ctx->bytesProcessed;
  if (ctx->bytesProcessed > MD5_MAX_MESSAGE_BYTES)
    return -1;

  /* Process the data in blocks */
  while (numBytes > 0) {
    if (ctx->bytesInBlock == 0 && numBytes >= MD5_BLOCK_BYTES) {
      md5ProcessBlock(ctx, bytes);
      bytes += MD5_BLOCK_BYTES;
      numBytes -= MD5_BLOCK_BYTES;
    }
    else {
      toFillBlock = MD5_BLOCK_BYTES - ctx->bytesInBlock;
      addToCtx = (toFillBlock < numBytes ? toFillBlock : numBytes);
      memcpy(ctx->block + ctx->bytesInBlock, bytes, addToCtx);
      ctx->bytesInBlock += addToCtx;
      bytes += addToCtx;
      numBytes -= addToCtx;
      if (ctx->bytesInBlock == MD5_BLOCK_BYTES) {
	ctx->bytesInBlock = 0;
	md5ProcessBlock(ctx, ctx->block);
      }
    }
  }

  return 0;
}

void md5End(void *c, byte_t *digest)
{
  struct md5CTX *ctx = (struct md5CTX *)c;
  byte_t toAppend[MD5_BLOCK_BYTES+8];
  uint64_t totalBits;
  size_t numToAppend;
  int i;

  totalBits = ctx->bytesProcessed * 8;

  /*
   * Append 0x80 0x00 0x00 0x00 ... 0x00 so that the resulting amount of data
   * in the context's current block is 56 bytes
   */
  if (ctx->bytesInBlock < 56)
    numToAppend = 56 - ctx->bytesInBlock;
  else
    numToAppend = MD5_BLOCK_BYTES - (ctx->bytesInBlock - 56);
  toAppend[0] = (byte_t)(0x80);
  memset(toAppend+1, 0, numToAppend-1);

  /*
   * Place the number of bits processed prior to this function being called
   * into the context (as a 64-bit integer in little endian order), filling out
   * the current block.
   */
  putLittleEnd64(toAppend + numToAppend, totalBits);

  /* Append and finalize */
  md5Add(ctx, toAppend, numToAppend + 8);
  for (i = 0; i < 4; i++)
    putLittleEnd32(digest + i*4, ctx->h[i]);
}

static void md5ProcessBlock(struct md5CTX *ctx, const byte_t *block)
{
  uint32_t a = ctx->h[0];
  uint32_t b = ctx->h[1];
  uint32_t c = ctx->h[2];
  uint32_t d = ctx->h[3];
  uint32_t w[16];
  uint32_t f, k, temp;
  int i;

  for (i = 0; i < 16; i++)
    w[i] = getLittleEnd32(block + i*4);

  for (i = 0; i < 64; i++) {
    if (i < 16) {
      f = (b & c) | ((~b) & d);
      k = i;
    }
    else if (i < 32) {
      f = (d & b) | ((~d) & c);
      k = ((i << 2) + i + 1) & (0x0F);
    }
    else if (i < 48) {
      f = b ^ c ^ d;
      k = ((i << 1) + i + 5) & (0x0F);
    }
    else {
      f = c ^ (b | (~d));
      k = ((i << 3) - i) & (0x0F);
    }

    temp = d;
    d = c;
    c = b;
    b = b + circShiftLeft32((a + f + SINE[i] + w[k]), ROTATION[i]);
    a = temp;
  }

  ctx->h[0] += a;
  ctx->h[1] += b;
  ctx->h[2] += c;
  ctx->h[3] += d;
}

int main() {
	byte_t * message = "Hello world!";
	struct md5CTX md; 
	memset(md, 0, sizeof(struct md5CTX));
	md5Start(&md);
	md5Add(&md, message, 12);
	char * digest;
	md5End(&md, digest);
	printf("%s", digest);
}

