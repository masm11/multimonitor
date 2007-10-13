/* Multi Monitor Plugin for Xfce
 *  Copyright (C) 2007 Yuuki Harano
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

#include <gtk/gtk.h>
#include <string.h>

/*
 *              critical
 *    +--------------------------------------------------+
 *    |                  ^                               |
 *    |       low        |     close         critical    v
 * [normal] -------> [level 1] -----> [close] -----> [level 2]
 *    ^                  |               |               |
 *    |                  v               v               |
 *    +--------------------------------------------------+
 *                clear
 */

#define CRITICAL_LEVEL	1
#define LOW_LEVEL	10

enum {
    STEP_NORMAL,
    STEP_LEVEL1,
    STEP_CLOSE,
    STEP_LEVEL2,
};

static struct {
    GtkWidget *dialog;
    gint step;
    gint prev_ratio;
} work;

void battalert_init(void)
{
    memset(&work, 0, sizeof work);
}

static void update_dialog(gint ratio)
{
    if (work.prev_ratio != ratio) {
	gchar *msg;
	
	msg = g_markup_printf_escaped("Battery level is %s.", (ratio <= CRITICAL_LEVEL ? "critical" : "low"));
	gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(work.dialog), msg);
	
	if (ratio <= CRITICAL_LEVEL) {
	    gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(work.dialog),
		    "<span size='xx-large' weight='heavy' foreground='red'>%d%%</span>", ratio);
	} else {
	    gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(work.dialog),
		    "<span size='xx-large' weight='heavy'>%d%%</span>", ratio);
	}
	
	g_free(msg);
    }
}

static void clicked(void)
{
    if (work.step == STEP_LEVEL1)
	work.step = STEP_CLOSE;
    gtk_widget_destroy(work.dialog);
    work.dialog = NULL;
}

static void create_dialog(gint ratio)
{
    work.dialog = gtk_message_dialog_new(
	    NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_CLOSE,
	    NULL);
    g_signal_connect_swapped(work.dialog, "response",
	    G_CALLBACK(clicked), work.dialog);
    
    work.prev_ratio = -1;
    update_dialog(ratio);
    
    gtk_widget_show_all(work.dialog);
}

void battalert_alert(gint ratio)
{
    switch (work.step) {
    case STEP_NORMAL:
	if (ratio <= CRITICAL_LEVEL) {
	    work.step = STEP_LEVEL2;
	    create_dialog(ratio);
	} else if (ratio <= LOW_LEVEL) {
	    work.step = STEP_LEVEL1;
	    create_dialog(ratio);
	}
	break;
	
    case STEP_LEVEL1:
	if (ratio <= CRITICAL_LEVEL) {
	    work.step = STEP_LEVEL2;
	    update_dialog(ratio);
	} else {
	    update_dialog(ratio);
	}
	break;
	
    case STEP_CLOSE:
	if (ratio <= CRITICAL_LEVEL) {
	    work.step = STEP_LEVEL2;
	    create_dialog(ratio);
	}
	break;
	
    case STEP_LEVEL2:
	update_dialog(ratio);
	break;
    }
}

void battalert_clear(void)
{
    if (work.dialog != NULL) {
	gtk_widget_destroy(work.dialog);
	work.dialog = NULL;
    }
    work.step = STEP_NORMAL;
}
