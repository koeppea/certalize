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
#include <certalize_asn1.h>

/* globals    */
GObject *window = NULL;
GObject *detailsview = NULL;
GObject *offsetgrid = NULL;
GObject *bytesgrid = NULL;
GObject *asciigrid = NULL;

/* prototypes */
static void cb_activate(GApplication *app, gpointer data);
static void cb_shutdown(GApplication *app, gpointer data);
static gboolean cb_tree_view_buttonpressed(GtkWidget *widget, 
      GdkEvent *event, gpointer data);
static gboolean cb_tree_selected(GtkTreeSelection *selection, 
      GtkTreeModel *model, GtkTreePath *path, gboolean selected, 
      gpointer data);

static void ui_shutdown(GSimpleAction *action, GVariant *value, gpointer data);
static void ui_new(GSimpleAction *action, GVariant *value, gpointer data);
static void ui_open(GSimpleAction *action, GVariant *value, gpointer data);
static void ui_prefs(GSimpleAction *action, GVariant *value, gpointer data);

static void ui_analyze_certificate(cbuf_t *cbuf);
static void ui_dump_bytes(cbuf_t *cbuf);
static void ui_byteselect(bytepointer_t *bp);
static void ui_dissect_signed_certificate(GtkTreeStore *store, cbuf_t *cbuf);


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
   GFile *file;
   GtkBuilder *widgets, *menus;
   GtkTreeSelection *selection;
   GtkCssProvider *provider;
   gchar *widgets_filename = INSTALL_UIDIR "/widgets.ui";
   gchar *menus_filename = INSTALL_UIDIR "/menus.ui";
   gchar *style_filename = INSTALL_UIDIR "/style.css";
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
   offsetgrid = gtk_builder_get_object(widgets, "offset-grid");
   bytesgrid = gtk_builder_get_object(widgets, "bytes-grid");
   asciigrid = gtk_builder_get_object(widgets, "ascii-grid");

   /* set selection function to select bytes */
   selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(detailsview));
   gtk_tree_selection_set_select_function(selection, cb_tree_selected, 
         NULL, NULL);

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

   /* load custom style */
   provider = gtk_css_provider_new();
   file = g_file_new_for_path(style_filename);
   if (gtk_css_provider_load_from_file(provider, file, &err)) {
      gtk_style_context_add_provider_for_screen(
            gdk_screen_get_default(), GTK_STYLE_PROVIDER(provider),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
   }
   else {
      g_warning("Could not load CSS file: %s", err->message);
      g_error_free(err);
   }
   
   /* show all widgets */
   gtk_widget_show_all(GTK_WIDGET(window));

   g_object_unref(widgets);
   g_object_unref(menus);
   g_object_unref(file);

   if (global_filename) {
      cbuf = cbuf_load_file(global_filename);
      /* TODO Infobar for error message */
      if (cbuf == NULL) {
         DEBUG_MSG("cb_activate: error parsing file");
      }
      else {
         ui_analyze_certificate(cbuf);
      }
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
 * callback when tree view is clicked
 *   - (future: context menu)
 */
static gboolean cb_tree_view_buttonpressed(GtkWidget *widget, GdkEvent *event,
      gpointer data _U_)
{
   /*
   GdkEventButton *button_event;

   if (event->type == GDK_BUTTON_PRESS) {
      button_event = (GdkEventButton*)event;
      if (0 && button_event->button == 1)
         ui_byteselect(GTK_TREE_VIEW(widget));
   }
   */

   return FALSE;
}
   
/*
 * callback when tree view is selcted 
 *   - used to select bytes in byteview
 */
static gboolean cb_tree_selected(GtkTreeSelection *selection _U_, 
      GtkTreeModel *model, GtkTreePath *path, gboolean selected _U_, 
      gpointer data _U_)
{
   GtkTreeIter iter;
   bytepointer_t *bp;
   gchar *title;

   if (gtk_tree_model_get_iter(model, &iter, path)) {
      gtk_tree_model_get(model, &iter, 0, &title, 1, &bp, -1);
      g_print("'%s' selected\n", title);
      g_free(title);
      ui_byteselect(bp);
   }

   return TRUE;
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
   GtkTreeView *tree;
   GtkTreeStore *store;
   GtkTreeIter iter;
   GtkTreeViewColumn *column;
   GtkCellRenderer *renderer;
   guint offset;

   DEBUG_MSG("ui_analyze_certificate");

   ui_dump_bytes(cbuf);

   offset = 0;
   tree = GTK_TREE_VIEW(detailsview);


   renderer = gtk_cell_renderer_text_new();
   column = gtk_tree_view_column_new();
   gtk_tree_view_column_pack_start(column, renderer, FALSE);
   gtk_tree_view_column_add_attribute(column, renderer, "text", 0);
   gtk_tree_view_append_column(tree, column);
   gtk_tree_view_set_headers_visible(tree, FALSE);

   store = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);
   ui_dissect_signed_certificate(store, cbuf);

   gtk_tree_view_set_model(tree, GTK_TREE_MODEL(store));

   g_object_unref(store);

   g_signal_connect(tree, "button-press-event", 
         G_CALLBACK(cb_tree_view_buttonpressed), NULL);

   gtk_widget_show_all(GTK_WIDGET(tree));

}

/*
 * printing bytes in the byte text view pane
 */
static void ui_dump_bytes(cbuf_t *cbuf)
{
   GtkTextBuffer *offsetbuf, *asciibuf;
   GtkTextIter offsetiter, asciiiter;
   GtkTextIter startiter, enditer;
   GtkGrid *grid;
   GtkWidget *label;
   guchar buf[16], *ptr, *fstr;
   guint offset = 0, remain, i, row = 0;

   DEBUG_MSG("ui_dump_bytes");

   ptr = buf;

   /* clear offset grid */
   gtk_grid_remove_column(GTK_GRID(offsetgrid), 0);

   /* clear bytes grid */
   for (i = 0; i < 17; i++)
      gtk_grid_remove_column(GTK_GRID(bytesgrid), i);

   /* clear ascii grid */
   for (i = 0; i < 8; i++)
      gtk_grid_remove_column(GTK_GRID(asciigrid), i);

   /* dump 16 bytes each line */
   while (cbuf_length_remaining(cbuf, offset) > 16) {

      cbuf_get_bytes(cbuf, &ptr, offset, 16);

      /* offset */
      fstr = g_strdup_printf("%08x", offset);
      label = gtk_label_new(fstr);
      gtk_grid_attach(GTK_GRID(offsetgrid), label, 0, row, 1, 1);
      g_free(fstr);

      /* bytes */
      for (i = 0; i < 8; i++) {
         fstr = g_strdup_printf("%02x", buf[i]);
         label = gtk_label_new(fstr);
         gtk_grid_attach(GTK_GRID(bytesgrid), label, i, row, 1, 1);
         g_free(fstr);
      }
      
      label = gtk_label_new(" ");
      gtk_grid_attach(GTK_GRID(bytesgrid), label, 8, row, 1, 1);

      for (i = 8; i < 16; i++) {
         fstr = g_strdup_printf("%02x", buf[i]);
         label = gtk_label_new(fstr);
         gtk_grid_attach(GTK_GRID(bytesgrid), label, i+1, row, 1, 1);
         g_free(fstr);
      }

      /* ascii */
      for (i = 0; i < 16; i++) {
         fstr = g_strdup_printf("%c", g_ascii_isprint(buf[i]) ? buf[i] : '.');
         label = gtk_label_new(fstr);
         gtk_grid_attach(GTK_GRID(asciigrid), label, i, row, 1, 1);
         g_free(fstr);
      }

      row++;
      offset += 16;
   }

   remain = cbuf_length_remaining(cbuf, offset);

   /* last line */
   if (remain > 0) {
      cbuf_get_bytes(cbuf, &ptr, offset, remain);

      /* offset */
      fstr = g_strdup_printf("%08x\n", offset);
      label = gtk_label_new(fstr);
      gtk_grid_attach(GTK_GRID(offsetgrid), label, 0, row, 1, 1);
      g_free(fstr);

      /* bytes */
      for (i = 0; i < (remain > 8 ? 8 : remain); i++) {
         fstr = g_strdup_printf("%02x", buf[i]);
         label = gtk_label_new(fstr);
         gtk_grid_attach(GTK_GRID(bytesgrid), label, i, row, 1, 1);
         g_free(fstr);
      }
      
      if (remain > 8) {
         label = gtk_label_new(" ");
         gtk_grid_attach(GTK_GRID(bytesgrid), label, 8, row, 1, 1);

         for (i = 8; i < remain; i++) {
            fstr = g_strdup_printf("%02x", buf[i]);
            label = gtk_label_new(fstr);
            gtk_grid_attach(GTK_GRID(bytesgrid), label, i+1, row, 1, 1);
            g_free(fstr);
         }
      }

      /* ascii */
      for (i = 0; i < remain; i++) {
         fstr = g_strdup_printf("%c", g_ascii_isprint(buf[i]) ? buf[i] : '.');
         label = gtk_label_new(fstr);
         gtk_grid_attach(GTK_GRID(asciigrid), label, i, row, 1, 1);
         g_free(fstr);
      }
   }

   gtk_widget_show_all(GTK_WIDGET(offsetgrid));
   gtk_widget_show_all(GTK_WIDGET(bytesgrid));
   gtk_widget_show_all(GTK_WIDGET(asciigrid));

}

static void ui_byteselect(bytepointer_t *bp)
{
   GtkTextBuffer *bytesbuf, *asciibuf, *offsetbuf;
   GtkTextIter startiter, enditer;
   GtkGrid *grid;
   GtkWidget *label;
   guint startoffset, endoffset;
   guint whitespaces, top, left;
   gridcoord_t startcoords, endcoords;

   DEBUG_MSG("ui_byteselect");

   grid = GTK_GRID(bytesgrid);

   bp->offset = 3;
   bp->length = 32;

   /* 
    * calculate the number of characters for the start of the selection 
    * in the bytesview
    */
   g_print("bytes_view:\n");
   whitespaces = bp->offset % 16 > 8 ? 1 : 0;
   startcoords.top = bp->offset / 16;
   startcoords.left = bp->offset % 16 + whitespaces;

   g_print("\tstart: top: %d, left: %d (whitespace: %d)\n", 
         startcoords.top, startcoords.left, whitespaces);

   whitespaces = (bp->offset + bp->length) % 16 > 8 ? 1 : 0;
   endcoords.top = (bp->offset + bp->length) / 16;
   endcoords.left = (bp->offset + bp->length) % 16 - 1 + whitespaces;

   g_print("\tend: top: %d, left: %d (whitespace: %d)\n", 
         endcoords.top, endcoords.left, whitespaces);

   label = gtk_grid_get_child_at(grid, startcoords.left, startcoords.top);
   g_print("Label at %d/%d has name %s\n",
         startcoords.top, startcoords.left, gtk_widget_get_name(label));

   gtk_widget_set_name(label, "selected");
   
   label = gtk_grid_get_child_at(grid, endcoords.left, endcoords.top);
   gtk_widget_set_name(label, "selected");



   /* 
    * calculate the number of characters for the start of the selection 
    * in the asciigrid
    */
   g_print("ascii_view:\n");
   whitespaces = bp->offset > 0 ? (bp->offset - 1) / 16 : 0;
   startoffset = bp->offset + whitespaces;
   g_print("\tstartoffset: %d (whitespaces: %d)\n", startoffset, whitespaces);

   whitespaces = bp->length > 0 ? (bp->length - 1) / 16 : 0;
   if (bp->offset % 16) // if offset begins withing a line ...
      whitespaces++; // add one whitespace
   endoffset = startoffset + bp->length + whitespaces;
   g_print("\tendoffset: %d (whitespaces: %d)\n", endoffset, whitespaces);

   // FIXME: scoll may to be done on the scrollable

}

void ui_dissect_signed_certificate(GtkTreeStore *store, cbuf_t *cbuf)
{
   GtkTreeIter iter, child;
   int res;
   asn1_hdr_t *asn1;
   bytepointer_t *bptr;

   DEBUG_MSG("ui_dissect_signed_certificate\n");

   asn1 = g_malloc0(sizeof(asn1_hdr_t));

   if ((res = asn1_parse_hdr(cbuf, asn1)) < 0) {
      DEBUG_MSG("Error in parsing ASN1 header\n");
      return;
   }

   if (!asn1->constructed && asn1->tag != ASN1_TAG_SEQUENCE) {
      DEBUG_MSG("X.509 certificate must start with a Sequence\n");
      return;
   }

   /* set bytepointer for byte selection */
   bptr = g_malloc0(sizeof(bytepointer_t));
   bptr->offset = cbuf->offset;
   bptr->length = asn1->length + res;

   /* create top-level */
   gtk_tree_store_append(store, &iter, NULL);
   gtk_tree_store_set(store, &iter, 
         0, "X.509 signed certificate", 1, bptr, -1);

   /* forward offset to already parsed bytes */
   cbuf->offset += res;

   /* prepare sublevels and start parsing */

   g_free(asn1);

}

/* EOF */

// vim:ts=3:expandtab
