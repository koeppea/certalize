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

/* globals    */

/* prototypes */



/*************/

cbuf_t cbuf_load_file(const gchar *filename)
{
   cbuf_t *cbuf;

   g_print("cbuf_load_file('%s')\n", filename);

   return E_SUCCESS;
}

/* EOF */

// vim:ts=3:expandtab
