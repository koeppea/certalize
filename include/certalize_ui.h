/* certalize_ui.h
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

#ifndef CERTALIZE_UI_H
#define CERTALIZE_UI_H

#include <gtk/gtk.h>
#include <glib.h>

typedef struct gtk_accel_map {
   /* detailed action name */
   gchar *action;
   /*
    * NULL terminated accelerator string-array
    */
   const char * const accel[2];
} ui_accel_map_t;

/* prototypes */
extern void ui_start(void);

#endif   /* CERTALIZE_UI_H */

/* EOF */

// vim:ts=3:expandtab
