/* base64.c - decoding PEM encoded DER certificate
 *
 * Copyright (C) 2019 Alexander Koeppe
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <certalize.h>
#include <certalize_base64.h>
#include <certalize_debug.h>

/* globals    */
const gchar decode_matrix[] = {
/* 0  */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
/* 16 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
/* 32 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, 
/* 48 */ 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -2, -1, -1, 
/* 64 */ -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 
/* 80 */ 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, 
/* 96 */ -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 
/* 112 */41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1, 
};

/* prototypes */



/*************/

/*
 * strips PEM header and line feed character reducing input to
 * pure Base64 and store result in output.
 * output must provide enough memory to store the result
 */
gsize pem_strip(const gchar *input, gsize len, gchar *output)
{
   gsize out;
   guint i;
   gchar *outptr;
   gchar *begin_token = "-----BEGIN CERTIFICATE";
   gchar *end_token = "-----END CERTIFICATE";

   /* forward pointer to actual base64 content */
   input = g_strstr_len(input, 100, begin_token);
   input = g_strstr_len(input, 30, "\n");

   /* reset end of string to end of base64 content*/
   outptr = g_strstr_len(input, len, end_token);
   memset(outptr, 0, strlen(end_token));

   /* go through byte for byte */
   for (outptr=output, i=0; i<len && input[i] != 0; i++)
      if (input[i] != '\n' && input[i] != '\r')
         *outptr++ = input[i];

   out = outptr - output;
   return out;
}

/*
 * decode Base64 encoded string and store decoded result in output
 * output must provide sufficient memory to store the result
 */
gint base64_decode(const gchar *input, gsize len, gchar *output)
{
   gsize out;
   gchar *outptr;
   guint i;

   /* santiy check: base64 decoded must always be dividable by 4 */
   if (len == 0 || len % 4) {
      DEBUG_MSG("base64_decode: %d is not an incremental of 4", len);
      return -1;
   }

   /* check for Base64 alphabet */
   for (i=0; i<len; i++) {
      if (input[i] >= 127 || decode_matrix[ (guint8)input[i] ] == -1) {
         DEBUG_MSG("base64_decode: not a valid Base64 character");
         return -1;
      }
   }

   i = 0;
   out = 0;
   while (len > 0) {
      outptr = output + out;

      /* decode first 6 bits decoded by 1. character */
      *(outptr+0) = decode_matrix[ (guint8)input[i] ] << 2;
      len--;
      i++;

      /* decode next 2 bits to complete first byte */
      *(outptr+0) |= (decode_matrix[ (guint8)input[i] ] & 0x30 ) >> 4; 
      out++;

      /* decode remaining 4 bits of the 2. character */
      *(outptr+1) = (decode_matrix[ (guint8)input[i] ] & 0x0f ) << 4;
      len--;
      i++;

      /* check for padding */
      if (len == 2 && input[i] == '=') {
         DEBUG_MSG("base64_decode: padding on second last char");
         break;
      }
      /* decode next 4 bits to complete the second byte */
      *(outptr+1) |= (decode_matrix[ (guint8)input[i] ] & 0x3c ) >> 2;
      out++;

      /* decode remaining 2 bits of the 3. character */
      *(outptr+2) = (decode_matrix[ (guint8)input[i] ] & 0x03 ) << 6;
      len--;
      i++;

      /* check for padding */
      if (len == 1 && input[i] == '=') {
         DEBUG_MSG("base64_decode: padding on last char");
         break;
      }
      /* decode next 6 bits to complete the third byte */
      *(outptr+2) |= decode_matrix[ (guint8)input[i] ];
      len--;
      i++;
      out++;

   }

   return out;
}

/* EOF */

// vim:ts=3:expandtab
