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
#include <certalize_debug.h>
#include <certalize_ui.h>
#include <certalize_buf.h>

/* globals    */
GObject *window = NULL;
GObject *detailsview = NULL;
GObject *offsetview = NULL;
GObject *bytesview = NULL;
GObject *asciiview = NULL;

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
int ui_start(void)
{
   GtkApplication *app = NULL;
   int status;

   app = gtk_application_new("org.gnome.Certalize", G_APPLICATION_FLAGS_NONE);

   /* main signal handlers */
   g_signal_connect(app, "activate", G_CALLBACK(cb_activate), NULL);
   g_signal_connect(app, "shutdown", G_CALLBACK(cb_shutdown), NULL);

   status = g_application_run(G_APPLICATION(app), 0, NULL);

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
   cbuf_t *cbuf = NULL;

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
   detailsview = gtk_builder_get_object(widgets, "details-view");
   offsetview = gtk_builder_get_object(widgets, "offset-view");
   bytesview = gtk_builder_get_object(widgets, "bytes-view");
   asciiview = gtk_builder_get_object(widgets, "ascii-view");


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

   if (global_filename) {
      cbuf_load_file(global_filename);
      ui_analyze_certificate(cbuf);
   }
}

/*
 * callback when application shuts down
 *   - do all cleanup work here
 */
static void cb_shutdown(GApplication *app _U_, gpointer data _U_)
{
   DEBUG_MSG("cb_shutdown");
}

/*
 * request to shutdown application
 */
static void ui_shutdown(GSimpleAction *action _U_, GVariant *value _U_, gpointer app)
{
   DEBUG_MSG("ui_shutdown");
   g_application_quit(app);
}

/*
 * New instance
 */
static void ui_new(GSimpleAction *action _U_, GVariant *value _U_, gpointer data _U_)
{
   DEBUG_MSG("ui_new");
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

   DEBUG_MSG("ui_open");

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

      /* TODO Infobar for error message */
      if (cbuf == NULL) {
         DEBUG_MSG("ui_open: error parsing file");
      }
      else {
         ui_analyze_certificate(cbuf);
      }
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
   DEBUG_MSG("ui_prefs");
}

/*
 * This is the main routing to dissect the certificate
 */
static void ui_analyze_certificate(cbuf_t *cbuf)
{
   DEBUG_MSG("ui_analyze_certificate");

   ui_dump_bytes(cbuf);
}

/*
 * printing bytes in the byte text view pane
 */
static void ui_dump_bytes(cbuf_t *cbuf)
{
   GtkTextBuffer *offsetbuf, *bytesbuf, *asciibuf;
   GtkTextIter offsetiter, bytesiter, asciiiter;
   GtkTextIter startiter, enditer;
   GtkCssProvider *provider;
   GtkStyleContext *offsetcontext, *bytescontext, *asciicontext;
   guchar buf[16], *ptr, *fstr;
   guint offset = 0, remain, i;

   DEBUG_MSG("ui_dump_bytes");

   ptr = buf;

   offsetbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(offsetview));
   bytesbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(bytesview));
   asciibuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(asciiview));

   /* clear buffers */
   gtk_text_buffer_get_start_iter(offsetbuf, &startiter);
   gtk_text_buffer_get_end_iter(offsetbuf, &enditer);
   gtk_text_buffer_delete(offsetbuf, &startiter, &enditer);

   gtk_text_buffer_get_start_iter(bytesbuf, &startiter);
   gtk_text_buffer_get_end_iter(bytesbuf, &enditer);
   gtk_text_buffer_delete(bytesbuf, &startiter, &enditer);

   gtk_text_buffer_get_start_iter(asciibuf, &startiter);
   gtk_text_buffer_get_end_iter(asciibuf, &enditer);
   gtk_text_buffer_delete(asciibuf, &startiter, &enditer);

   /* set fixed font */
   offsetcontext = gtk_widget_get_style_context(GTK_WIDGET(offsetview));
   bytescontext = gtk_widget_get_style_context(GTK_WIDGET(bytesview));
   asciicontext = gtk_widget_get_style_context(GTK_WIDGET(asciiview));

   provider = gtk_css_provider_new();
   gtk_css_provider_load_from_data(provider,
         "textview {"
         "   font: monospace;"
         "}",
         -1, NULL);

   gtk_style_context_add_provider(offsetcontext, GTK_STYLE_PROVIDER(provider),
         GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
   gtk_style_context_add_provider(bytescontext, GTK_STYLE_PROVIDER(provider),
         GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
   gtk_style_context_add_provider(asciicontext, GTK_STYLE_PROVIDER(provider),
         GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);



   while (cbuf_length_remaining(cbuf, offset) > 16) {

      cbuf_get_bytes(cbuf, &ptr, offset, 16);

      fstr = g_strdup_printf("%08x\n", offset);
      gtk_text_buffer_get_end_iter(offsetbuf, &offsetiter);
      gtk_text_buffer_insert(offsetbuf, &offsetiter, fstr, -1);
      g_free(fstr);

      for (i = 0; i < 8; i++) {
         fstr = g_strdup_printf("%02x ", buf[i]);
         gtk_text_buffer_get_end_iter(bytesbuf, &bytesiter);
         gtk_text_buffer_insert(bytesbuf, &bytesiter, fstr, -1);
         g_free(fstr);
      }
      
      gtk_text_buffer_get_end_iter(bytesbuf, &bytesiter);
      gtk_text_buffer_insert(bytesbuf, &bytesiter, " ", -1);

      for (i = 8; i < 16; i++) {
         fstr = g_strdup_printf("%02x ", buf[i]);
         gtk_text_buffer_get_end_iter(bytesbuf, &bytesiter);
         gtk_text_buffer_insert(bytesbuf, &bytesiter, fstr, -1);
         g_free(fstr);
      }

      gtk_text_buffer_get_end_iter(bytesbuf, &bytesiter);
      gtk_text_buffer_insert(bytesbuf, &bytesiter, "\n", -1);

      for (i = 0; i < 16; i++) {
         fstr = g_strdup_printf("%c", g_ascii_isprint(buf[i]) ? buf[i] : '.');
         gtk_text_buffer_get_end_iter(asciibuf, &asciiiter);
         gtk_text_buffer_insert(asciibuf, &asciiiter, fstr, -1);
      }

      gtk_text_buffer_get_end_iter(asciibuf, &asciiiter);
      gtk_text_buffer_insert(asciibuf, &asciiiter, "\n", -1);

      offset += 16;
   }

   remain = cbuf_length_remaining(cbuf, offset);

   if (remain > 0) {
      cbuf_get_bytes(cbuf, &ptr, offset, remain);

      fstr = g_strdup_printf("%08x\n", offset);
      gtk_text_buffer_get_end_iter(offsetbuf, &offsetiter);
      gtk_text_buffer_insert(offsetbuf, &offsetiter, fstr, -1);
      g_free(fstr);

      for (i = 0; i < (remain > 8 ? 8 : remain); i++) {
         fstr = g_strdup_printf("%02x ", buf[i]);
         gtk_text_buffer_get_end_iter(bytesbuf, &bytesiter);
         gtk_text_buffer_insert(bytesbuf, &bytesiter, fstr, -1);
         g_free(fstr);
      }
      
      if (remain > 8) {
         gtk_text_buffer_get_end_iter(bytesbuf, &bytesiter);
         gtk_text_buffer_insert(bytesbuf, &bytesiter, " ", -1);

         for (i = 8; i < remain; i++) {
            fstr = g_strdup_printf("%02x ", buf[i]);
            gtk_text_buffer_get_end_iter(bytesbuf, &bytesiter);
            gtk_text_buffer_insert(bytesbuf, &bytesiter, fstr, -1);
            g_free(fstr);
         }
      }

      for (i = 0; i < remain; i++) {
         fstr = g_strdup_printf("%c", g_ascii_isprint(buf[i]) ? buf[i] : '.');
         gtk_text_buffer_get_end_iter(asciibuf, &asciiiter);
         gtk_text_buffer_insert(asciibuf, &asciiiter, fstr, -1);
         g_free(fstr);
      }
   }


}
/* EOF */

// vim:ts=3:expandtab
