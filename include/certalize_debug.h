/* certalize_debug.h
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

#ifndef CERTALIZE_DEBUG_H
#define CERTALIZE_DEBUG_H

#include <config.h>

#ifdef DEBUG
extern void debug_msg(const char *message, ...);

#define DEBUG_MSG(x, ...) do {                 \
   debug_msg(x, ## __VA_ARGS__ );              \
} while (0)

#endif


/*
 * if DEBUG Is not defined
 */
#ifndef DEBUG
#define DEBUG_MSG(x, ...)
#endif

#endif   /* CERTALIZE_DEBUG_H */

/* EOF */

// vim:ts=3:expandtab
