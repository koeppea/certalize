/* certalize_buf.h
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

#ifndef CERTALIZE_BUF_H
#define CERTALIZE_BUF_H

#include <certalize.h>

typedef struct cbuf {
   guchar *buffer;
   gsize length;
   guint offset;
} cbuf_t;

extern cbuf_t* cbuf_load_file(const gchar *filename);
extern guint16 cbuf_get_ntohs(cbuf_t *cbuf, guint offset);
extern guint32 cbuf_get_ntohl(cbuf_t *cbuf, guint offset);
extern gchar*  cbuf_get_bytes(cbuf_t *cbuf, guchar **buffer, guint offset, gsize length);
extern gsize   cbuf_length_remaining(cbuf_t *cbuf, guint offset);


#endif   /* CERTALIZE_BUF_H */

/* EOF */

// vim:ts=3:expandtab
