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

#ifndef MCC_DATASOURCE_H
#define MCC_DATASOURCE_H

#include "../config.h"
#include <glib-object.h>
#include <gdk/gdkcolor.h>
#include "mccvalue.h"

#define MCC_TYPE_DATA_SOURCE                  (mcc_data_source_get_type ())
#define MCC_DATA_SOURCE(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), MCC_TYPE_DATA_SOURCE, MccDataSource))
#define MCC_DATA_SOURCE_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), MCC_TYPE_DATA_SOURCE, MccDataSourceClass))
#define MCC_IS_DATA_SOURCE(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MCC_TYPE_DATA_SOURCE))
#define MCC_IS_DATA_SOURCE_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), MCC_TYPE_DATA_SOURCE))
#define MCC_DATA_SOURCE_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), MCC_TYPE_DATA_SOURCE, MccDataSourceClass))

typedef struct _MccDataSource       MccDataSource;
typedef struct _MccDataSourceClass  MccDataSourceClass;

struct _MccDataSourcePrivate;

struct _MccDataSource {
    GObject object;
    
    gint subidx;
    
    gdouble min, max;
    gint nvalues;
    
    gint nfg;
    gchar **fg_labels;		// [nfg]
    GdkColor *default_fg;	// [nfg]
    
    gint nbg;
    gchar **bg_labels;		// [nbg]
    GdkColor *default_bg;	// [nbg]
    
    gboolean dynamic_scaling;
    gboolean add_on_tick;
    
    gchar *sublabel;
};

struct _MccDataSourceClass {
    GObjectClass parent_class;
    
    gchar *label;
    gchar **sublabels;
    gint tick_per_read;
    
    gint tick_for_read;
    gboolean has_new_data;
    
    gint proc_dirfd, sys_dirfd;
    
    void (*read)(MccDataSourceClass *datasrc_class);
    void (*set_subidx)(MccDataSource *src);
    MccValue *(*get)(MccDataSource *src);
};

GType mcc_data_source_get_type(void) G_GNUC_CONST;
void mcc_data_source_read(GType type);
gboolean mcc_data_source_has_new_data(MccDataSource *datasrc);
MccValue *mcc_data_source_get(MccDataSource *datasrc);
MccDataSource *mcc_data_source_new(GType type, gint subidx);

#endif
