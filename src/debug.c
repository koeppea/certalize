/* debug.c - debugging
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

#ifdef DEBUG

/* globals    */

/* prototypes */


/*************/

/*
 * pring debug message
 */
void debug_msg(const char *message, ...)
{
   va_list ap;
   char debug_message[strlen(message) + 2];

   strncpy(debug_message, message, sizeof(debug_message));
   strncat(debug_message, "\n", sizeof(debug_message));

   va_start(ap, message);
   vfprintf(stderr, debug_message, ap);
   va_end(ap);

   fflush(stderr);
}
#endif

/* EOF */

// vim:ts=3:expandtab
