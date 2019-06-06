/* ui.c - graphical user interface
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

/* globals    */

/* prototypes */
static void ui_activate(GApplication* app, gpointer data);
static void ui_shutdown(GApplication* app, gpointer data);


/*************/

/*
 * startup GTK main loop
 */
int ui_start(int argc, char *argv[])
{
   GtkApplication *app = NULL;
   GtkWidget *window = NULL;
   int status;

   app = gtk_application_new("org.gnome.Certalize",
         G_APPLICATION_FLAGS_NONE);

   g_signal_connect(app, "activate", G_CALLBACK(ui_activate), window);
   g_signal_connect(app, "shutdown", G_CALLBACK(ui_shutdown), window);

   status = g_application_run(G_APPLICATION(app), argc, argv);

   g_object_unref(G_OBJECT(app));

   return status;
}

/*
 * build widgets from resource file
 */
static void ui_activate(GApplication *app, gpointer data)
{
   GObject *window;
   GError *err = NULL;
   GtkBuilder *widgets, *menus;
   gchar *widgets_filename = INSTALL_UIDIR "/widgets.ui";
   gchar *menus_filename = INSTALL_UIDIR "/menus.ui";
   gint i;

   (void) data;

   /* Load UI from XML file */
   widgets = gtk_builder_new();
   if (!gtk_builder_add_from_file(widgets, widgets_filename, &err)) {
      g_print("GtkBuilder could not load from file '%s': %s\n",
            widgets_filename, err->message);
      g_error_free(err);
      return;
   }

   menus = gtk_builder_new();
   if (!gtk_builder_add_from_file(menus, menus_filename, &err)) {
      g_print("GtkBuilder could not load from file '%s': %s\n",
            menus_filename, err->message);
      g_error_free(err);
      return;
   }


   /* define accelerators */
   static ui_accel_map_t accels[] = {
      {"app.quit", {"<Primary>q", NULL}}
   };

   /* map accelerators to actions */
   for (i = 0; i < G_N_ELEMENTS(accels); i++)
      gtk_application_set_accels_for_action(GTK_APPLICATION(app),
            accels[i].action, accels[i].accel);
   
   
   /* show application window */
   window = gtk_builder_get_object(widgets, "main-window");
   gtk_application_add_window(GTK_APPLICATION(app), GTK_WINDOW(window));

   /* set app menu */
   gtk_application_set_app_menu(GTK_APPLICATION(app),
         G_MENU_MODEL(gtk_builder_get_object(menus, "app-menu")));

   /* show all widgets */
   gtk_widget_show_all(GTK_WIDGET(window));
}

/*
 * cleanup when application is shut down
 */
static void ui_shutdown(GApplication *app, gpointer data)
{
}


/* EOF */

// vim:ts=3:expandtab
