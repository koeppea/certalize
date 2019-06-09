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
#include <certalize_buf.h>

/* globals    */
GObject *window = NULL;
GObject *details = NULL;
GObject *bytes = NULL;

/* prototypes */
static void cb_activate(GApplication *app, gpointer data);
static void cb_shutdown(GApplication *app, gpointer data);
static void ui_shutdown(GSimpleAction *action, GVariant *value, gpointer data);
static void ui_new(GSimpleAction *action, GVariant *value, gpointer data);
static void ui_open(GSimpleAction *action, GVariant *value, gpointer data);
static void ui_prefs(GSimpleAction *action, GVariant *value, gpointer data);
static void ui_analyze_certificate(cbuf_t *cbuf);
static void ui_dump_bytes(cbuf_t *cbuf);


/*************/

/*
 * startup GTK main loop
 */
int ui_start(int argc, char *argv[])
{
   GtkApplication *app = NULL;
   int status;

   app = gtk_application_new("org.gnome.Certalize", G_APPLICATION_FLAGS_NONE);

   g_signal_connect(app, "activate", G_CALLBACK(cb_activate), NULL);
   g_signal_connect(app, "shutdown", G_CALLBACK(cb_shutdown), NULL);

   status = g_application_run(G_APPLICATION(app), argc, argv);

   g_object_unref(G_OBJECT(app));

   return status;
}

/*
 * build widgets from resource file
 */
static void cb_activate(GApplication *app, gpointer data _U_)
{
   GObject *menu;
   GError *err = NULL;
   GtkBuilder *widgets, *menus;
   gchar *widgets_filename = INSTALL_UIDIR "/widgets.ui";
   gchar *menus_filename = INSTALL_UIDIR "/menus.ui";
   guint i;

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

   /* show application window */
   window = gtk_builder_get_object(widgets, "main-window");
   gtk_application_add_window(GTK_APPLICATION(app), GTK_WINDOW(window));

   /* set app menu */
   menu = gtk_builder_get_object(menus, "app-menu");
   gtk_application_set_app_menu(GTK_APPLICATION(app), G_MENU_MODEL(menu));

   /* get main widgets to be used later */
   details = gtk_builder_get_object(widgets, "details-view");
   bytes = gtk_builder_get_object(widgets, "bytes-view");

   /* define accelerators */
   static ui_accel_map_t accels[] = {
      {"app.new", {"<Primary>n", NULL}},
      {"app.open", {"<Primary>o", NULL}},
      {"app.quit", {"<Primary>q", NULL}}
   };

   /* define actions */
   static GActionEntry action_entries[] = {
      {"new", ui_new, NULL, NULL, NULL, {}},
      {"open", ui_open, NULL, NULL, NULL, {}},
      {"prefs", ui_prefs, NULL, NULL, NULL, {}},
      {"quit", ui_shutdown, NULL, NULL, NULL, {}}
   };

   /* add action to the application */
   g_action_map_add_action_entries(G_ACTION_MAP(app), action_entries,
         G_N_ELEMENTS(action_entries), app);

   /* map accelerators to actions */
   for (i = 0; i < G_N_ELEMENTS(accels); i++)
      gtk_application_set_accels_for_action(GTK_APPLICATION(app),
            accels[i].action, accels[i].accel);
   
   /* show all widgets */
   gtk_widget_show_all(GTK_WIDGET(window));

   g_object_unref(widgets);
   g_object_unref(menus);

}

/*
 * request to shutdown application
 */
static void ui_shutdown(GSimpleAction *action _U_, GVariant *value _U_, gpointer app)
{
   g_print("ui_shutdown\n");
   g_application_quit(app);
}

/*
 * callback when application shuts down
 *   - do all cleanup work here
 */
static void cb_shutdown(GApplication *app _U_, gpointer data _U_)
{
   g_print("cb_shutdown\n");
}

/*
 * New instance
 */
static void ui_new(GSimpleAction *action _U_, GVariant *value _U_, gpointer data _U_)
{
   g_print("ui_new\n");
}

/*
 * Open file dialog
 */
static void ui_open(GSimpleAction *action _U_, GVariant *value _U_, gpointer data _U_)
{
   GtkWidget *dialog, *chooser, *content;
   gchar *filename;
   gint response = 0;
   cbuf_t *cbuf;

   g_print("ui_open\n");

   dialog = gtk_dialog_new_with_buttons("Select a Certificate file",
         GTK_WINDOW(window),
         GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_USE_HEADER_BAR,
         "_Cancel", GTK_RESPONSE_CANCEL,
         "_OK",     GTK_RESPONSE_OK,
         NULL);
   gtk_container_set_border_width(GTK_CONTAINER(dialog), 10);

   content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
   chooser = gtk_file_chooser_widget_new(GTK_FILE_CHOOSER_ACTION_OPEN);
   gtk_container_add(GTK_CONTAINER(content), chooser);
   gtk_widget_show(chooser);

   response = gtk_dialog_run(GTK_DIALOG(dialog));

   if (response == GTK_RESPONSE_OK) {
      gtk_widget_hide(dialog);
      filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser));
      gtk_widget_destroy(dialog);
      cbuf = cbuf_load_file(filename);
      g_free(filename);
      ui_analyze_certificate(cbuf);
   }
   else {
      gtk_widget_destroy(dialog);
   }
}

/*
 * Preferences dialog
 */
static void ui_prefs(GSimpleAction *action _U_, GVariant *value _U_, gpointer data _U_)
{
   g_print("ui_prefs\n");
}

/*
 * This is the main routing to dissect the certificate
 */
static void ui_analyze_certificate(cbuf_t *cbuf)
{
   g_print("ui_analyze_certificate\n");

   ui_dump_bytes(cbuf);
}

/*
 * printing bytes in the byte text view pane
 */
static void ui_dump_bytes(cbuf_t *cbuf)
{
   GtkTextView *textview;
   GtkTextBuffer *buffer;
   gchar buf[16], *ptr;
   guint offset = 0, remain, i;
   g_print("ui_dump_bytes\n");

   textview = GTK_TEXT_VIEW(bytes);


   while (cbuf_length_remaining(cbuf, offset) < 16) {

      ptr = buf;
      cbuf_get_bytes(cbuf, &ptr, offset, 16);

      g_print("%08x  ", offset);

      for (i = 0; i < 8; i++)
         g_print("%02x ", buf[i]);
      
      g_print(" ");

      for (i = 8; i < 16; i++)
         g_print("%02x ", buf[i]);

      g_print("|");
      for (i = 0; i < 16; i++)
         g_print("%c", g_ascii_isprint(buf[i]) ? buf[i] : '.');
      g_print("|\n");

      offset += 16;
   }

   remain = cbuf_length_remaining(cbuf, offset);

   if (remain > 0) {
      g_print("%08x  ", offset);

      for (i = 0; i < (remain > 8 ? 8 : remain); i++)
         g_print("%02x ", buf[i]);
      
      if (remain > 8) {
         g_print(" ");

         for (i = 8; i < remain; i++)
            g_print("%02x ", buf[i]);
      }

      g_print(" ");
      g_print("|");
      for (i = 0; i < remain; i++)
         g_print("%c", g_ascii_isprint(buf[i]) ? buf[i] : '.');
      g_print("|\n");
   }


}
/* EOF */

// vim:ts=3:expandtab
