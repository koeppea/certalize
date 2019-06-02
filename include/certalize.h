/* certalize.h
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

#ifndef CERTALIZE_H
#define CERTALIZE_H

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>


enum {
   E_SUCCESS    = 0,
   E_NOTFOUND   = 1,
   E_NOMATCH    = 2,
   E_NOTHANDLED = 3,
   E_INVALID    = 4,
   E_NOADDRESS  = 5,
   E_DUPLICATE  = 6,
   E_TIMEOUT    = 7,
   E_INITFAIL   = 8,
   E_FOUND      = 128,
   E_BRIDGE     = 129,
   E_VERSION    = 254,
   E_FATAL      = 255,
};



#endif   /* CERTALIZE_H */

/* EOF */

// vim:ts=3:expandtab
