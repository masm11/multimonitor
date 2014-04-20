/* Multi Monitor Plugin for Xfce
 *  Copyright (C) 2007,2014 Yuuki Harano
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "../config.h"
#include <gtk/gtk.h>
#include <libxfcegui4/xfce_aboutdialog.h>
#include "about.h"

static GtkWidget *dialog = NULL;

static void clicked(GtkDialog *w, gint arg, gpointer userdata)
{
    gtk_widget_destroy(dialog);
    dialog = NULL;
}

void about(void)
{
    if (dialog == NULL) {
	XfceAboutInfo *info = xfce_about_info_new(PACKAGE_NAME, PACKAGE_VERSION,
		"A Graphical monitor of multiple values.",
		XFCE_COPYRIGHT_TEXT("2007,2014", "Yuuki Harano"),
		XFCE_LICENSE_GPL);
	
	dialog = xfce_about_dialog_new_with_values(NULL, info, NULL);
	g_signal_connect(dialog, "response", G_CALLBACK(clicked), NULL);
	
	xfce_about_info_free(info);
    }
    
    gtk_window_present(GTK_WINDOW(dialog));
}
