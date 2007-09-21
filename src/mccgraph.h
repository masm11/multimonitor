/* 
 *  Copyright (C) 2007 Yuuki Harano
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
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

#ifndef MCC_GRAPH_H
#define MCC_GRAPH_H

#include <gtk/gtkmisc.h>
#include "mccvalue.h"

#define MCC_TYPE_GRAPH                  (mcc_graph_get_type ())
#define MCC_GRAPH(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), MCC_TYPE_GRAPH, MccGraph))
#define MCC_GRAPH_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), MCC_TYPE_GRAPH, MccGraphClass))
#define MCC_IS_GRAPH(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MCC_TYPE_GRAPH))
#define MCC_IS_GRAPH_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), MCC_TYPE_GRAPH))
#define MCC_GRAPH_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), MCC_TYPE_GRAPH, MccGraphClass))

typedef struct _MccGraph       MccGraph;
typedef struct _MccGraphClass  MccGraphClass;

struct _MccGraphPrivate;

struct _MccGraph {
    GtkMisc misc;
    
    struct _MccGraphPrivate *priv;
};

struct _MccGraphClass {
    GtkMiscClass parent_class;
};

GType mcc_graph_get_type(void) G_GNUC_CONST;
GtkWidget *mcc_graph_new(gint nvalues, gdouble min, gdouble max,
	gint nfg, const GdkColor *fg,
	gint nbg, const GdkColor *bg);
void mcc_graph_add(MccGraph *graph, MccValue *value);
void mcc_graph_get_fg(MccGraph *graph, int i, GdkColor *fg);
void mcc_graph_get_bg(MccGraph *graph, int i, GdkColor *bg);
void mcc_graph_set_fg(MccGraph *graph, int i, const GdkColor *fg);
void mcc_graph_set_bg(MccGraph *graph, int i, const GdkColor *bg);

#endif
