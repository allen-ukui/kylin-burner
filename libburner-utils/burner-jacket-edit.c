/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/*
 * Libburner-misc
 * Copyright (C) Philippe Rouquier 2005-2009 <bonfire-app@wanadoo.fr>
 * Copyright (C) 2017,Tianjin KYLIN Information Technology Co., Ltd.
 *
 * Libburner-misc is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * The Libburner-misc authors hereby grant permission for non-GPL compatible
 * GStreamer plugins to be used and distributed together with GStreamer
 * and Libburner-misc. This permission is above and beyond the permissions granted
 * by the GPL license by which Libburner-burn is covered. If you modify this code
 * you may extend this exception to your version of the code, but you are not
 * obligated to do so. If you do not wish to do so, delete this exception
 * statement from your version.
 *
 * Libburner-misc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <glib.h>
#include <glib/gi18n-lib.h>
#include <glib-object.h>

#include <gtk/gtk.h>

#include "burner-misc.h"
#include "burner-jacket-edit.h"
#include "burner-jacket-buffer.h"
#include "burner-jacket-view.h"
#include "burner-jacket-font.h"
#include "burner-tool-color-picker.h"

typedef struct _BurnerJacketEditPrivate BurnerJacketEditPrivate;
struct _BurnerJacketEditPrivate
{
	GtkWidget *current_view;
	GtkWidget *front;
	GtkWidget *back;

	GtkWidget *fonts;
	GtkWidget *colours;

	GtkWidget *center;
	GtkWidget *right;
	GtkWidget *left;

	GtkWidget *underline;
	GtkWidget *italic;
	GtkWidget *bold;

	GtkWidget *background;
};

#define BURNER_JACKET_EDIT_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), BURNER_TYPE_JACKET_EDIT, BurnerJacketEditPrivate))

enum {
	SNAP_PIX_COL,
	SNAP_TEXT_COL,
	SNAP_TOOLTIP_COL,
	SNAP_WIDGET_COL,
	SNAP_NUM_COL
};

G_DEFINE_TYPE (BurnerJacketEdit, burner_jacket_edit, GTK_TYPE_BOX);

static void
burner_jacket_edit_print_page (GtkPrintOperation *operation,
				GtkPrintContext *context,
				gint page_num,
				BurnerJacketEdit *self)
{
	BurnerJacketEditPrivate *priv;
	guint y;

	priv = BURNER_JACKET_EDIT_PRIVATE (self);

	y = burner_jacket_view_print (BURNER_JACKET_VIEW (priv->front), context, 0., 0.);
	burner_jacket_view_print (BURNER_JACKET_VIEW (priv->back), context, 0., y + 20.);
}

static void
burner_jacket_edit_print_begin (GtkPrintOperation *operation,
				 GtkPrintContext *context,
				 BurnerJacketEdit *self)
{
	gtk_print_operation_set_n_pages (operation, 1);
}

static void
burner_jacket_edit_print_pressed_cb (GtkButton *button,
				      BurnerJacketEdit *self)
{
	GtkPrintOperationResult res;
	GtkPrintOperation *print;
	GtkPrintSettings *settings;
	GError *error = NULL;
	GtkWidget *toplevel;
	gchar *path;

	print = gtk_print_operation_new ();
	g_signal_connect (print,
			  "draw-page",
			  G_CALLBACK (burner_jacket_edit_print_page),
			  self);
	g_signal_connect (print,
			  "begin-print",
			  G_CALLBACK (burner_jacket_edit_print_begin),
			  self);

	toplevel = gtk_widget_get_toplevel (GTK_WIDGET (self));

	path = g_build_path (G_DIR_SEPARATOR_S, g_get_user_config_dir (), "burner", "print-settings", NULL);
	settings = gtk_print_settings_new_from_file (path, NULL);
	if (settings) {
		gtk_print_operation_set_print_settings (print, settings);
		g_object_unref (settings);
	}

	/* NOTE: when a dialog is hidden while it was run by gtk_dialog_run ()
	 * a response will be sent (GTK_RESPONSE_NONE) */
	gtk_widget_hide (toplevel);
	res = gtk_print_operation_run (print,
				       GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
				       gtk_window_get_transient_for (GTK_WINDOW (toplevel)),
				       &error);

	if (res == GTK_PRINT_OPERATION_RESULT_APPLY) {
		settings = gtk_print_operation_get_print_settings (print);
		gtk_print_settings_to_file (settings, path, NULL);
		g_free (path);
	}

	g_object_unref (print);
}

static void
burner_jacket_edit_underline_pressed_cb (GtkToggleToolButton *button,
					  BurnerJacketEdit *self)
{
	BurnerJacketEditPrivate *priv;
	GtkTextBuffer *buffer;
	GtkTextIter start;
	GtkTextIter end;
	GtkTextTag *tag;

	priv = BURNER_JACKET_EDIT_PRIVATE (self);

	if (!priv->current_view)
		return;

	buffer = burner_jacket_view_get_active_buffer (BURNER_JACKET_VIEW (priv->current_view));
	tag = gtk_text_buffer_create_tag (buffer, NULL,
					  "underline", gtk_toggle_tool_button_get_active (button) ? PANGO_UNDERLINE_SINGLE:PANGO_UNDERLINE_NONE,
					  NULL);

	if (gtk_text_buffer_get_has_selection (buffer)) {
		gtk_text_buffer_get_selection_bounds (buffer, &start, &end);
		gtk_text_buffer_apply_tag (buffer, tag, &start, &end);
	}
	else
		burner_jacket_buffer_add_default_tag (BURNER_JACKET_BUFFER (buffer), tag);
}

static void
burner_jacket_edit_italic_pressed_cb (GtkToggleToolButton *button,
				       BurnerJacketEdit *self)
{
	BurnerJacketEditPrivate *priv;
	GtkTextBuffer *buffer;
	GtkTextIter start;
	GtkTextIter end;
	GtkTextTag *tag;

	priv = BURNER_JACKET_EDIT_PRIVATE (self);

	if (!priv->current_view)
		return;

	buffer = burner_jacket_view_get_active_buffer (BURNER_JACKET_VIEW (priv->current_view));
	tag = gtk_text_buffer_create_tag (buffer, NULL,
					  "style", gtk_toggle_tool_button_get_active (button) ? PANGO_STYLE_ITALIC:PANGO_STYLE_NORMAL,
					  NULL);

	if (gtk_text_buffer_get_has_selection (buffer)) {
		gtk_text_buffer_get_selection_bounds (buffer, &start, &end);
		gtk_text_buffer_apply_tag (buffer, tag, &start, &end);
	}
	else
		burner_jacket_buffer_add_default_tag (BURNER_JACKET_BUFFER (buffer), tag);
}

static void
burner_jacket_edit_bold_pressed_cb (GtkToggleToolButton *button,
				     BurnerJacketEdit *self)
{
	BurnerJacketEditPrivate *priv;
	GtkTextBuffer *buffer;
	GtkTextIter start;
	GtkTextIter end;
	GtkTextTag *tag;

	priv = BURNER_JACKET_EDIT_PRIVATE (self);

	if (!priv->current_view)
		return;

	buffer = burner_jacket_view_get_active_buffer (BURNER_JACKET_VIEW (priv->current_view));
	tag = gtk_text_buffer_create_tag (buffer, NULL,
					  "weight", gtk_toggle_tool_button_get_active (button) ? PANGO_WEIGHT_BOLD:PANGO_WEIGHT_NORMAL,
					  NULL);

	if (gtk_text_buffer_get_has_selection (buffer)) {
		gtk_text_buffer_get_selection_bounds (buffer, &start, &end);
		gtk_text_buffer_apply_tag (buffer, tag, &start, &end);
	}
	else
		burner_jacket_buffer_add_default_tag (BURNER_JACKET_BUFFER (buffer), tag);
}

static void
burner_jacket_edit_justify (GtkToggleToolButton *button,
			     BurnerJacketEdit *self,
			     GtkJustification justify)
{
	BurnerJacketEditPrivate *priv;
	GtkTextBuffer *buffer;
	GtkTextIter start;
	GtkTextIter end;
	GtkTextTag *tag;

	priv = BURNER_JACKET_EDIT_PRIVATE (self);

	if (!gtk_toggle_tool_button_get_active (button))
		return;

	if (!priv->current_view)
		return;

	buffer = burner_jacket_view_get_active_buffer (BURNER_JACKET_VIEW (priv->current_view));
	tag = gtk_text_buffer_create_tag (buffer, NULL,
					  "justification", justify,
					  NULL);

	if (!gtk_text_buffer_get_has_selection (buffer)) {
		GtkTextMark *mark;

		mark = gtk_text_buffer_get_insert (buffer);
		gtk_text_buffer_get_iter_at_mark (buffer, &start, mark);
		gtk_text_buffer_get_iter_at_mark (buffer, &end, mark);
		burner_jacket_buffer_add_default_tag (BURNER_JACKET_BUFFER (buffer), tag);
	}
	else
		gtk_text_buffer_get_selection_bounds (buffer, &start, &end);

	gtk_text_iter_set_line_index (&start, 0);
	gtk_text_iter_forward_to_line_end (&end);
	gtk_text_buffer_apply_tag (buffer, tag, &start, &end);
}

static void
burner_jacket_edit_center_pressed_cb (GtkToggleToolButton *button,
				       BurnerJacketEdit *self)
{
	burner_jacket_edit_justify (button, self,  GTK_JUSTIFY_CENTER);
}

static void
burner_jacket_edit_right_pressed_cb (GtkToggleToolButton *button,
				      BurnerJacketEdit *self)
{
	burner_jacket_edit_justify (button, self, GTK_JUSTIFY_RIGHT);
}

static void
burner_jacket_edit_left_pressed_cb (GtkToggleToolButton *button,
				     BurnerJacketEdit *self)
{
	burner_jacket_edit_justify (button, self, GTK_JUSTIFY_LEFT);
}

static void
burner_jacket_edit_colours_changed_cb (GtkColorButton *button,
					BurnerJacketEdit *self)
{
	BurnerJacketEditPrivate *priv;
	GtkTextBuffer *buffer;
	GtkTextIter start;
	GtkTextIter end;
	GdkColor color;
	GtkTextTag *tag;

	priv = BURNER_JACKET_EDIT_PRIVATE (self);
	if (!priv->current_view)
		return;

	burner_tool_color_picker_get_color (BURNER_TOOL_COLOR_PICKER (button), &color);

	buffer = burner_jacket_view_get_active_buffer (BURNER_JACKET_VIEW (priv->current_view));
	tag = gtk_text_buffer_create_tag (buffer, NULL,
					  "foreground-gdk", &color,
					  NULL);
	if (!gtk_text_buffer_get_has_selection (buffer)) {
		burner_jacket_buffer_add_default_tag (BURNER_JACKET_BUFFER (buffer), tag);
		return;
	}

	gtk_text_buffer_get_selection_bounds (buffer, &start, &end);
	gtk_text_buffer_apply_tag (buffer, tag, &start, &end);
}

static void
burner_jacket_edit_font_changed_cb (BurnerJacketFont *button,
				     BurnerJacketEdit *self)
{
	GtkTextTag *tag_family = NULL;
	BurnerJacketEditPrivate *priv;
	gchar *font_family = NULL;
	GtkTextBuffer *buffer;
	GtkTextTag *tag_size;
	GtkTextIter start;
	GtkTextIter end;

	priv = BURNER_JACKET_EDIT_PRIVATE (self);

	if (!priv->current_view)
		return;

	buffer = burner_jacket_view_get_active_buffer (BURNER_JACKET_VIEW (priv->current_view));
	if (!buffer)
		return;

	/* Create two tags, one for the family, one for the size */
	font_family = burner_jacket_font_get_family (button);
	if (font_family) {
		tag_family = gtk_text_buffer_create_tag (buffer, NULL,
							 "family", font_family,
							 NULL);
		g_free (font_family);
	}

	tag_size = gtk_text_buffer_create_tag (buffer, NULL,
					       "size", burner_jacket_font_get_size (button),
					       NULL);

	if (gtk_text_buffer_get_has_selection (buffer)) {
		gtk_text_buffer_get_selection_bounds (buffer, &start, &end);
		gtk_text_buffer_apply_tag (buffer, tag_size, &start, &end);
		gtk_text_buffer_apply_tag (buffer, tag_family, &start, &end);
	}

	burner_jacket_buffer_add_default_tag (BURNER_JACKET_BUFFER (buffer), tag_size);
	burner_jacket_buffer_add_default_tag (BURNER_JACKET_BUFFER (buffer), tag_family);
}

static void
burner_jacket_edit_configure_background_pressed_cb (GtkToolButton *button,
						     BurnerJacketEdit *self)
{
	BurnerJacketEditPrivate *priv;

	priv = BURNER_JACKET_EDIT_PRIVATE (self);
	if (priv->current_view)
		burner_jacket_view_configure_background (BURNER_JACKET_VIEW (priv->current_view));
}

static void
burner_jacket_edit_update_button_state (BurnerJacketEdit *self)
{
	gint pos;
	GtkTextIter iter;
	gchar *font_name;
	GtkTextBuffer *buffer;
	GtkTextAttributes *attributes;
	BurnerJacketEditPrivate *priv;

	priv = BURNER_JACKET_EDIT_PRIVATE (self);

	if (priv->current_view)
		buffer = burner_jacket_view_get_active_buffer (BURNER_JACKET_VIEW (priv->current_view));
	else
		buffer = NULL;

	gtk_widget_set_sensitive (priv->background, buffer != NULL);
	gtk_widget_set_sensitive (priv->bold, buffer != NULL);
	gtk_widget_set_sensitive (priv->italic, buffer != NULL);
	gtk_widget_set_sensitive (priv->underline, buffer != NULL);
	gtk_widget_set_sensitive (priv->right, buffer != NULL);
	gtk_widget_set_sensitive (priv->left, buffer != NULL);
	gtk_widget_set_sensitive (priv->center, buffer != NULL);
	gtk_widget_set_sensitive (priv->fonts, buffer != NULL);
	gtk_widget_set_sensitive (priv->colours, buffer != NULL);

	if (!buffer)
		return;

	g_object_get (buffer,
		      "cursor-position", &pos,
		      NULL);

	if (pos)
		gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (buffer), &iter, pos - 1);
	else
		gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (buffer), &iter, pos);

	attributes = burner_jacket_view_get_attributes (BURNER_JACKET_VIEW (priv->current_view), &iter);

	burner_tool_color_picker_set_color (BURNER_TOOL_COLOR_PICKER (priv->colours), &attributes->appearance.fg_color);
	
	font_name = pango_font_description_to_string (attributes->font);
	burner_jacket_font_set_name (BURNER_JACKET_FONT (priv->fonts), font_name);
	g_free (font_name);

	g_signal_handlers_block_by_func (priv->bold,
					 burner_jacket_edit_bold_pressed_cb,
					 self);
	gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (priv->bold),
					  (pango_font_description_get_weight (attributes->font) != PANGO_WEIGHT_NORMAL));
	g_signal_handlers_unblock_by_func (priv->bold,
					   burner_jacket_edit_bold_pressed_cb,
					   self);

	g_signal_handlers_block_by_func (priv->italic,
					 burner_jacket_edit_italic_pressed_cb,
					 self);
	gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (priv->italic),
					  (pango_font_description_get_style (attributes->font) == PANGO_STYLE_ITALIC));
	g_signal_handlers_unblock_by_func (priv->italic,
					   burner_jacket_edit_italic_pressed_cb,
					   self);

	g_signal_handlers_block_by_func (priv->underline,
					 burner_jacket_edit_underline_pressed_cb,
					 self);
	gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (priv->underline),
					  (attributes->appearance.underline != PANGO_UNDERLINE_NONE));
	g_signal_handlers_unblock_by_func (priv->underline,
					   burner_jacket_edit_underline_pressed_cb,
					   self);

	g_signal_handlers_block_by_func (priv->right,
					 burner_jacket_edit_right_pressed_cb,
					 self);
	g_signal_handlers_block_by_func (priv->left,
					 burner_jacket_edit_left_pressed_cb,
					 self);
	g_signal_handlers_block_by_func (priv->center,
					 burner_jacket_edit_center_pressed_cb,
					 self);
	switch (attributes->justification) {
	case GTK_JUSTIFY_CENTER:
		gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (priv->center), TRUE);
		gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (priv->right), FALSE);
		gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (priv->left), FALSE);
		break;
	case GTK_JUSTIFY_LEFT:
		gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (priv->center), FALSE);
		gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (priv->right), FALSE);
		gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (priv->left), TRUE);
		break;
	case GTK_JUSTIFY_RIGHT:
		gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (priv->center), FALSE);
		gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (priv->right), TRUE);
		gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (priv->left), FALSE);
		break;
	default:
		gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (priv->center), FALSE);
		gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (priv->right), TRUE);
		gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (priv->left), FALSE);
		break;
	};
	g_signal_handlers_unblock_by_func (priv->right,
					   burner_jacket_edit_right_pressed_cb,
					   self);
	g_signal_handlers_unblock_by_func (priv->left,
					   burner_jacket_edit_left_pressed_cb,
					   self);
	g_signal_handlers_unblock_by_func (priv->center,
					   burner_jacket_edit_center_pressed_cb,
					   self);

	gtk_text_attributes_unref (attributes);
}

static void
burner_jacket_edit_tags_changed_cb (BurnerJacketView *view,
				     BurnerJacketEdit *self)
{
	BurnerJacketEditPrivate *priv;

	priv = BURNER_JACKET_EDIT_PRIVATE (self);
	priv->current_view = GTK_WIDGET (view);
	burner_jacket_edit_update_button_state (self);
}

static void
burner_jacket_edit_init (BurnerJacketEdit *object)
{
	BurnerJacketEditPrivate *priv;
	GtkWidget *main_box;
	GtkWidget *toolbar;
	GtkWidget *scroll;
	GtkWidget *vbox;
	GtkWidget *item;
	GtkWidget *view;

	priv = BURNER_JACKET_EDIT_PRIVATE (object);

	gtk_orientable_set_orientation (GTK_ORIENTABLE (object), GTK_ORIENTATION_VERTICAL);

	/* Toolbar */
	toolbar = gtk_toolbar_new ();
	gtk_style_context_add_class (gtk_widget_get_style_context (toolbar),
				     GTK_STYLE_CLASS_PRIMARY_TOOLBAR);
	gtk_widget_show (toolbar);
	gtk_box_pack_start (GTK_BOX (object), toolbar, FALSE, TRUE, 0);

	/* Items */
	item = GTK_WIDGET (gtk_tool_button_new_from_stock (GTK_STOCK_PRINT));
	gtk_widget_set_tooltip_text (item, _("Print"));
	gtk_widget_show (item);
	g_signal_connect (item,
			  "clicked",
			  G_CALLBACK (burner_jacket_edit_print_pressed_cb),
			  object);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (item), 0);

	item = GTK_WIDGET (gtk_separator_tool_item_new ());
	gtk_widget_show (item);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (item), 0);

	item = GTK_WIDGET (gtk_tool_button_new (NULL, _("Bac_kground Properties")));
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (item), "preferences-desktop-wallpaper");
	gtk_tool_button_set_use_underline (GTK_TOOL_BUTTON (item), TRUE);
	gtk_widget_set_tooltip_text (item, _("Background properties"));
	gtk_widget_show (item);
	gtk_widget_set_sensitive (item, FALSE);
	g_signal_connect (item,
			  "clicked",
			  G_CALLBACK (burner_jacket_edit_configure_background_pressed_cb),
			  object);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (item), 0);
	priv->background = item;

	item = GTK_WIDGET (gtk_separator_tool_item_new ());
	gtk_widget_show (item);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (item), 0);

	item = GTK_WIDGET (gtk_radio_tool_button_new_from_stock (NULL, GTK_STOCK_JUSTIFY_RIGHT));
	gtk_widget_set_tooltip_text (item, _("Align right"));
	gtk_widget_show (item);
	g_signal_connect (item,
			  "clicked",
			  G_CALLBACK (burner_jacket_edit_right_pressed_cb),
			  object);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (item), 0);
	priv->right = item;

	item = GTK_WIDGET (gtk_radio_tool_button_new_from_stock (gtk_radio_tool_button_get_group (GTK_RADIO_TOOL_BUTTON (priv->right)), GTK_STOCK_JUSTIFY_CENTER));
	gtk_widget_set_tooltip_text (item, _("Center"));
	gtk_widget_show (item);
	g_signal_connect (item,
			  "clicked",
			  G_CALLBACK (burner_jacket_edit_center_pressed_cb),
			  object);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (item), 0);
	priv->center = item;

	item = GTK_WIDGET (gtk_radio_tool_button_new_from_stock (gtk_radio_tool_button_get_group (GTK_RADIO_TOOL_BUTTON (priv->right)), GTK_STOCK_JUSTIFY_LEFT));
	gtk_widget_set_tooltip_text (item, _("Align left"));
	gtk_widget_show (item);
	g_signal_connect (item,
			  "clicked",
			  G_CALLBACK (burner_jacket_edit_left_pressed_cb),
			  object);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (item), 0);
	priv->left = item;

	item = GTK_WIDGET (gtk_separator_tool_item_new ());
	gtk_widget_show (item);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (item), 0);

	item = GTK_WIDGET (gtk_toggle_tool_button_new_from_stock (GTK_STOCK_UNDERLINE));
	gtk_widget_set_tooltip_text (item, _("Underline"));
	gtk_widget_show (item);
	g_signal_connect (item,
			  "clicked",
			  G_CALLBACK (burner_jacket_edit_underline_pressed_cb),
			  object);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (item), 0);
	priv->underline = item;

	item = GTK_WIDGET (gtk_toggle_tool_button_new_from_stock (GTK_STOCK_ITALIC));
	gtk_widget_set_tooltip_text (item, _("Italic"));
	gtk_widget_show (item);
	g_signal_connect (item,
			  "clicked",
			  G_CALLBACK (burner_jacket_edit_italic_pressed_cb),
			  object);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (item), 0);
	priv->italic = item;

	item = GTK_WIDGET (gtk_toggle_tool_button_new_from_stock (GTK_STOCK_BOLD));
	gtk_widget_set_tooltip_text (item, _("Bold"));
	gtk_widget_show (item);
	g_signal_connect (item,
			  "clicked",
			  G_CALLBACK (burner_jacket_edit_bold_pressed_cb),
			  object);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (item), 0);
	priv->bold = item;

	item = GTK_WIDGET (gtk_separator_tool_item_new ());
	gtk_widget_show (item);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (item), 0);

	priv->fonts = burner_jacket_font_new ();
	gtk_widget_show (priv->fonts);
	g_signal_connect (priv->fonts,
			  "font-changed",
			  G_CALLBACK (burner_jacket_edit_font_changed_cb),
			  object);
	g_signal_connect (priv->fonts,
			  "size-changed",
			  G_CALLBACK (burner_jacket_edit_font_changed_cb),
			  object);

	item = GTK_WIDGET (gtk_tool_item_new ());
	gtk_widget_show (item);
	gtk_widget_set_tooltip_text (item, _("Font family and size"));
	gtk_container_add (GTK_CONTAINER (item), priv->fonts);
	gtk_tool_item_set_expand (GTK_TOOL_ITEM (item), FALSE);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (item), 0);

	priv->colours = burner_tool_color_picker_new ();
	burner_tool_color_picker_set_text (BURNER_TOOL_COLOR_PICKER (priv->colours),
					    _("_Text Color"));
	gtk_widget_set_tooltip_text (priv->colours, _("Text color"));
	gtk_widget_show (priv->colours);
	g_signal_connect (priv->colours,
			  "color-set",
			  G_CALLBACK (burner_jacket_edit_colours_changed_cb),
			  object);

	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (priv->colours), 1);

	/* contents */
	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 8);
	gtk_widget_show (vbox);
	gtk_box_pack_start (GTK_BOX (object), vbox, TRUE, TRUE, 0);

	scroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gtk_widget_show (scroll);
	gtk_box_pack_start (GTK_BOX (vbox), scroll, TRUE, TRUE, 0);

	main_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_show (main_box);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scroll), main_box);

	view = burner_jacket_view_new ();
	priv->front = view;
	gtk_widget_show (view);
	burner_jacket_view_set_side (BURNER_JACKET_VIEW (view), BURNER_JACKET_FRONT);
	g_signal_connect (view,
			  "tags-changed",
			  G_CALLBACK (burner_jacket_edit_tags_changed_cb),
			  object);

	gtk_box_pack_start (GTK_BOX (main_box), view, FALSE, FALSE, 0);

	view = burner_jacket_view_new ();
	priv->back = view;
	gtk_widget_show (view);
	burner_jacket_view_set_side (BURNER_JACKET_VIEW (view), BURNER_JACKET_BACK);

	g_signal_connect (view,
			  "tags-changed",
			  G_CALLBACK (burner_jacket_edit_tags_changed_cb),
			  object);

	gtk_box_pack_start (GTK_BOX (main_box), view, FALSE, FALSE, 0);

	burner_jacket_font_set_name (BURNER_JACKET_FONT (priv->fonts), "Sans 12");
}

void
burner_jacket_edit_freeze (BurnerJacketEdit *self)
{
	BurnerJacketEditPrivate *priv;

	priv = BURNER_JACKET_EDIT_PRIVATE (self);

	g_signal_handlers_block_by_func (priv->front, burner_jacket_edit_tags_changed_cb, self);
	g_signal_handlers_block_by_func (priv->back, burner_jacket_edit_tags_changed_cb, self);
}

void
burner_jacket_edit_thaw (BurnerJacketEdit *self)
{
	BurnerJacketEditPrivate *priv;

	priv = BURNER_JACKET_EDIT_PRIVATE (self);

	g_signal_handlers_unblock_by_func (priv->front, burner_jacket_edit_tags_changed_cb, self);
	g_signal_handlers_unblock_by_func (priv->back, burner_jacket_edit_tags_changed_cb, self);
}

BurnerJacketView *
burner_jacket_edit_get_front (BurnerJacketEdit *self)
{
	BurnerJacketEditPrivate *priv;

	priv = BURNER_JACKET_EDIT_PRIVATE (self);

	return BURNER_JACKET_VIEW (priv->front);
}

BurnerJacketView *
burner_jacket_edit_get_back (BurnerJacketEdit *self)
{
	BurnerJacketEditPrivate *priv;

	priv = BURNER_JACKET_EDIT_PRIVATE (self);

	return BURNER_JACKET_VIEW (priv->back);
}

static void
burner_jacket_edit_finalize (GObject *object)
{
	G_OBJECT_CLASS (burner_jacket_edit_parent_class)->finalize (object);
}

static void
burner_jacket_edit_class_init (BurnerJacketEditClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (BurnerJacketEditPrivate));

	object_class->finalize = burner_jacket_edit_finalize;
}

GtkWidget *
burner_jacket_edit_new (void)
{
	return g_object_new (BURNER_TYPE_JACKET_EDIT, NULL);
}

static void
on_exit_cb (GtkWidget *button, GtkWidget *dialog)
{
	if (dialog)
		gtk_dialog_response (dialog, GTK_RESPONSE_CANCEL);
}

GtkWidget *
burner_jacket_edit_dialog_new (GtkWidget *toplevel,
				BurnerJacketEdit **contents_ret)
{
	GtkWidget *window;
	GtkWidget *contents;
	GtkWidget *action_area;
	GtkWidget *box;
	GtkWidget *vbox;
	GtkWidget *title;
	GtkWidget *close_bt;
	GtkWidget *label;
	GtkWidget *alignment;

	window = gtk_dialog_new_with_buttons (_("Cover Editor"),
					      GTK_WINDOW (toplevel),
					      GTK_DIALOG_MODAL|
					      GTK_DIALOG_DESTROY_WITH_PARENT,
					      GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
					      NULL);

	gtk_window_set_type_hint (GTK_WINDOW (window), GDK_WINDOW_TYPE_HINT_NORMAL);
	gtk_window_set_default_size (GTK_WINDOW (window), 680, 640);
	contents = burner_jacket_edit_new ();
	gtk_widget_show (contents);

	gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (window))), contents, TRUE, TRUE, 0);
	if (contents_ret)
		*contents_ret = BURNER_JACKET_EDIT (contents);

	burner_jacket_edit_update_button_state (BURNER_JACKET_EDIT (contents));

	return window;
}

