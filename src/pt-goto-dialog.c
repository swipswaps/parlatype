/* Copyright (C) Gabor Karsay 2016 <gabor.karsay@gmx.at>
 *
 * Based on Totem, string_to_time() almost unchanged, rest modified:
 * https://git.gnome.org/browse/totem/tree/src/plugins/skipto/totem-time-entry.c?h=gnome-3-14
 *
 * Copyright (C) 2008 Philip Withnall <philip@tecnocode.co.uk>
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "config.h"
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include "pt-player.h"
#include "pt-goto-dialog.h"


static GtkWidget *goto_dialog = NULL;

struct _PtGotoDialogPrivate
{
	GtkWidget *spin;
	gint	   max;
};

G_DEFINE_TYPE_WITH_PRIVATE (PtGotoDialog, pt_goto_dialog, GTK_TYPE_DIALOG)


static gint
string_to_time (const char *time_string)
{
	gint sec, min, hour, args;

	args = sscanf (time_string, C_("long time format", "%d:%02d:%02d"), &hour, &min, &sec);

	if (args == 3) {
		/* Parsed all three arguments successfully */
		return (hour * (60 * 60) + min * 60 + sec) * 1000;
	} else if (args == 2) {
		/* Only parsed the first two arguments; treat hour and min as min and sec, respectively */
		return (hour * 60 + min) * 1000;
	} else if (args == 1) {
		/* Only parsed the first argument; treat hour as sec */
		return hour * 1000;
	} else {
		/* Error! */
		return -1;
	}
}

gboolean
output_cb (GtkSpinButton *spin,
	   PtGotoDialog  *dlg)
{
	gchar *text;

	if (dlg->priv->max == 0)
		return GTK_INPUT_ERROR;
	
	text = pt_player_get_time_string (
			gtk_spin_button_get_value_as_int (spin) * 1000,
			dlg->priv->max, 0);

	gtk_entry_set_text (GTK_ENTRY (spin), text);
	g_free (text);

	return TRUE;
}

gint
input_cb (GtkSpinButton *self,
	  gdouble	*new_value,
	  gpointer	 user_data)
{
	gint64 val;

	val = string_to_time (gtk_entry_get_text (GTK_ENTRY (self)));
	if (val == -1)
		return GTK_INPUT_ERROR;

	*new_value = val / 1000;
	return TRUE;
}

static void
pt_goto_dialog_init (PtGotoDialog *dlg)
{
	dlg->priv = pt_goto_dialog_get_instance_private (dlg);
	gtk_widget_init_template (GTK_WIDGET (dlg));

	dlg->priv->max = 0;
}

static void
pt_goto_dialog_dispose (GObject *object)
{
	PtGotoDialog *dlg = PT_GOTO_DIALOG (object);

	G_OBJECT_CLASS (pt_goto_dialog_parent_class)->dispose (object);
}

static void
pt_goto_dialog_class_init (PtGotoDialogClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	object_class->dispose = pt_goto_dialog_dispose;

	/* Bind class to template */
	gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/parlatype/goto-dialog.ui");
	gtk_widget_class_bind_template_child_private (widget_class, PtGotoDialog, spin);
}

gint
pt_goto_dialog_get_pos (PtGotoDialog *dlg)
{
	return gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (dlg->priv->spin));
}

void
pt_goto_dialog_set_pos (PtGotoDialog *dlg,
			gint	      seconds)
{
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (dlg->priv->spin), (gdouble) seconds);
}

void
pt_goto_dialog_set_max (PtGotoDialog *dlg,
			gint	      seconds)
{
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dlg->priv->spin), 0, (gdouble) seconds);
	dlg->priv->max = seconds * 1000;
}

PtGotoDialog *
pt_goto_dialog_new (GtkWindow *win)
{
	return g_object_new (
			PT_TYPE_GOTO_DIALOG,
			"use-header-bar", TRUE,
			"transient-for", win,
			NULL);
}
