/* main.c
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
#include <certalize_ui.h>

int parse_options(int argc _U_, char *argv[] _U_)
{
   return E_SUCCESS;
}

int main(int argc, char *argv[])
{
   int ret = 0;

   /* parse options */
   if ((ret = parse_options(argc, argv))) {
      return -ret;
   }

   /* start UI */
   ret = ui_start(argc, argv);

   return ret;
}



/* EOF */

// vim:ts=3:expandtab
