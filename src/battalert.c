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

#include "../config.h"
#include <gtk/gtk.h>
#include <string.h>
#include "battalert.h"

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

struct battalert_t {
    GtkWidget *dialog;
    gint step;
    gint prev_ratio;
};

struct battalert_t *battalert_new(void)
{
    struct battalert_t *w = g_new0(struct battalert_t, 1);
    return w;
}

static void update_dialog(struct battalert_t *w, gint ratio)
{
    if (w->prev_ratio != ratio && w->dialog != NULL) {
	gchar *msg;
	
	msg = g_markup_printf_escaped("Battery level is %s.", (ratio <= CRITICAL_LEVEL ? "critical" : "low"));
	gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(w->dialog), msg);
	
	if (ratio <= CRITICAL_LEVEL) {
	    gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(w->dialog),
		    "<span size='xx-large' weight='heavy' foreground='red'>%d%%</span>", ratio);
	} else {
	    gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(w->dialog),
		    "<span size='xx-large' weight='heavy'>%d%%</span>", ratio);
	}
	
	g_free(msg);
    }
}

static void clicked(GtkDialog *dialog, gint arg, gpointer userdata)
{
    struct battalert_t *w = userdata;
    if (w->step == STEP_LEVEL1)
	w->step = STEP_CLOSE;
    gtk_widget_destroy(w->dialog);
    w->dialog = NULL;
}

static void create_dialog(struct battalert_t *w, gint ratio)
{
    w->dialog = gtk_message_dialog_new(
	    NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_CLOSE,
	    NULL);
    g_signal_connect(w->dialog, "response", G_CALLBACK(clicked), w);
    
    w->prev_ratio = -1;
    update_dialog(w, ratio);
    
    gtk_widget_show_all(w->dialog);
}

void battalert_alert(struct battalert_t *w, gint ratio)
{
    switch (w->step) {
    case STEP_NORMAL:
	if (ratio <= CRITICAL_LEVEL) {
	    w->step = STEP_LEVEL2;
	    create_dialog(w, ratio);
	} else if (ratio <= LOW_LEVEL) {
	    w->step = STEP_LEVEL1;
	    create_dialog(w, ratio);
	}
	break;
	
    case STEP_LEVEL1:
	if (ratio <= CRITICAL_LEVEL) {
	    w->step = STEP_LEVEL2;
	    update_dialog(w, ratio);
	} else {
	    update_dialog(w, ratio);
	}
	break;
	
    case STEP_CLOSE:
	if (ratio <= CRITICAL_LEVEL) {
	    w->step = STEP_LEVEL2;
	    create_dialog(w, ratio);
	}
	break;
	
    case STEP_LEVEL2:
	update_dialog(w, ratio);
	break;
    }
}

void battalert_clear(struct battalert_t *w)
{
    if (w->dialog != NULL) {
	gtk_widget_destroy(w->dialog);
	w->dialog = NULL;
    }
    w->step = STEP_NORMAL;
}
