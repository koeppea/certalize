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

void print_usage(void)
{
   g_print("\nUsage: %s [OPTIONS] [FILE]\n", PROGRAM_NAME);
   g_print("\nOptions:\n");
   g_print("   -v, --version      prints the version and exits\n");
   g_print("   -h, --help         this help screen\n");
   g_print("\n\n");

   exit(0);
}

int parse_options(int argc, char *argv[])
{
   int c;
   int option_index = 0;

   static struct option long_options[] = {
      { "version", no_argument, NULL, 'v' },
      { "help", no_argument, NULL, 'h' },
      { "help", no_argument, NULL, '?' },
      { 0, 0, 0, 0 }
   };

   while ((c = getopt_long(argc, argv, "vh?", long_options, &option_index)) != EOF) {
      switch (c) {
         case 'v':
            g_print("%s's version is %s\n", PROGRAM_NAME, PROGRAM_VERSION);
            exit(0);
            break;
         case '?':
         case 'h':
            print_usage();
            exit(0);
            break;
         default:
            break;
      }
   }

   /* if argument left parse it as filename */
   if (argv[optind]) {
      global_filename = g_strdup(argv[optind]);
   }

   return E_SUCCESS;
}

int main(int argc, char *argv[])
{
   int ret = 0;

   global_filename = NULL;

   ret = parse_options(argc, argv);
   if (ret != E_SUCCESS) {
      return ret;
   }

   /* start UI */
   ret = ui_start();

   return ret;
}



/* EOF */

// vim:ts=3:expandtab
