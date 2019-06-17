/* buf.c - certificate buffer handling
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
#include <certalize_buf.h>
#include <certalize_base64.h>
#include <certalize_debug.h>

/* globals    */

/* prototypes */



/*************/

cbuf_t* cbuf_load_file(const gchar *filename)
{
   gchar *content;
   gsize readlen;
   GError *error;
   cbuf_t *cbuf;
   gchar *pemident = "-----BEGIN CERTIFICATE";

   DEBUG_MSG("cbuf_load_file('%s')", filename);
   cbuf = g_malloc0(sizeof(cbuf_t));

   if (cbuf == NULL) {
      g_print("memory allocation failed: %s\n", strerror(errno));
      return NULL;
   }

   if (!g_file_get_contents(filename, &content, &readlen, &error)) {
      g_print("reading file '%s' failed: '%s'\n", filename, error->message);
      g_free(cbuf);
      return NULL;
   }

   /* try to determine if the file is direclty DER or wrapped in PEM */
   if (strncmp(content, pemident, strlen(pemident)) == 0) {
      gsize len = 0;
      gint dlen = 0;
      gchar *base64, *der;
      DEBUG_MSG("cbuf_load_file: PEM endcoded file");

      base64 = g_malloc0(readlen);
      len = pem_strip(content, readlen, base64);
      der = g_malloc0((len/4)*3);
      dlen = base64_decode(base64, len, der);
      g_print("decoded: (%d)\n", dlen);

      g_free(base64);
      g_free(content);

      if (dlen == -1)
         return NULL;

      content = der;
      readlen = dlen;
      
   }
   else if (memcmp(content, "0", 1) == 0) {
      /* this is a very vague determination of DER encoded X.509 cert */
      DEBUG_MSG("cbuf_load_file: DER encoded file");
   }
   else {
      /* something else */
      DEBUG_MSG("cbuf_load_file: Something else");
   }

   cbuf->buffer = content;
   cbuf->length = readlen;
   cbuf->offset = 0;

   return cbuf;
}

/*
 * Returns 16-bits from buffer in host-byte order
 */
guint16 cbuf_get_ntohs(cbuf_t *cbuf, guint offset)
{
   guint16 result;

   if (offset + 2 > cbuf->length) {
      DEBUG_MSG("cbuf_get_ntohs: not enough buffer available");
      return 0;
   }

   result = (guint16)*(cbuf->buffer + offset+0) << 8 |
            (guint16)*(cbuf->buffer + offset+1);
   
   return result;
}

/*
 * Returns 32-bits from buffer in host-byte order
 */
guint32 cbuf_get_ntohl(cbuf_t *cbuf, guint offset)
{
   guint32 result;

   if (offset + 4 > cbuf->length) {
      DEBUG_MSG("cbuf_get_ntohl: not enough buffer available");
      return 0;
   }

   result = (guint32)*(cbuf->buffer + offset+0) << 24 |
            (guint32)*(cbuf->buffer + offset+1) << 16 |
            (guint32)*(cbuf->buffer + offset+2) <<  8 |
            (guint32)*(cbuf->buffer + offset+3);

   return result;
}

/*
 * Copies number of bytes into a externally provided buffer
 * Buffer have to provide enough memory to hold the data
 */
gchar* cbuf_get_bytes(cbuf_t *cbuf, guchar **buffer, guint offset, gsize length)
{

   if(offset + length > cbuf->length) {
      DEBUG_MSG("cbuf_get_bytes: not enough buffer available");
      return NULL;
   }

   memcpy(*buffer, cbuf->buffer + offset, length);

   return *buffer;
}

/*
 * Returns the remaining length relative from offset
 */
gsize cbuf_length_remaining(cbuf_t *cbuf, guint offset)
{
   return cbuf->length - offset;
}


/* EOF */

// vim:ts=3:expandtab
