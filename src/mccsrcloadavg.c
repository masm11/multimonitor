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

#include <string.h>
#include "mccsrcloadavg.h"

static void loadavg_read_data(gdouble *ptr);

static void mcc_src_load_avg_class_init(gpointer klass, gpointer class_data);
static void mcc_src_load_avg_set_subidx(MccDataSource *datasrc);
static void mcc_src_load_avg_read(MccDataSourceClass *datasrc_class);
static void mcc_src_load_avg_init(GTypeInstance *obj, gpointer klass);
static void mcc_src_load_avg_finalize(GObject *obj);
static MccValue *mcc_src_load_avg_get(MccDataSource *datasrc);

static gpointer mcc_src_load_avg_parent_class = NULL;

GType mcc_src_load_avg_get_type(void)
{
    static GType type = 0;
    if (type == 0) {
	static GTypeInfo type_info = {
	    .class_size = sizeof(MccSrcLoadAvgClass),
	    
	    .base_init = NULL,
	    .base_finalize = NULL,
	    
	    .class_init = mcc_src_load_avg_class_init,
	    .class_finalize = NULL,
	    .class_data = NULL,
	    
	    .instance_size = sizeof(MccSrcLoadAvg),
	    .n_preallocs = 0,
	    .instance_init = mcc_src_load_avg_init,
	    
	    .value_table = NULL,
	};
	
	type = g_type_register_static(MCC_TYPE_DATA_SOURCE, "MccSrcLoadAvg", &type_info, 0);
    }
    
    return type;
}

static void mcc_src_load_avg_class_init(gpointer klass, gpointer class_data)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    MccDataSourceClass *datasrc_class = MCC_DATA_SOURCE_CLASS(klass);
    MccSrcLoadAvgClass *src_class = MCC_SRC_LOAD_AVG_CLASS(klass);
    
    mcc_src_load_avg_parent_class = g_type_class_peek_parent(klass);
    
    gobject_class->finalize = mcc_src_load_avg_finalize;
    
    datasrc_class->label = g_strdup("Load AVG");
    datasrc_class->sublabels = g_new0(gchar *, 4);
    datasrc_class->sublabels[0] = g_strdup("1 min");
    datasrc_class->sublabels[1] = g_strdup("5 min");
    datasrc_class->sublabels[2] = g_strdup("15 min");
    datasrc_class->set_subidx = mcc_src_load_avg_set_subidx;
    datasrc_class->read = mcc_src_load_avg_read;
    datasrc_class->get = mcc_src_load_avg_get;
    
    src_class->olddata = g_new0(gdouble, 3);
    src_class->newdata = g_new0(gdouble, 3);
    
    loadavg_read_data(src_class->newdata);
    memcpy(src_class->olddata, src_class->newdata, sizeof *src_class->olddata * 3);
}

static void mcc_src_load_avg_read(MccDataSourceClass *datasrc_class)
{
    MccSrcLoadAvgClass *src_class = MCC_SRC_LOAD_AVG_CLASS(datasrc_class);
    
    memcpy(src_class->olddata, src_class->newdata, sizeof *src_class->olddata * 3);
    loadavg_read_data(src_class->newdata);
}

static void loadavg_read_data(gdouble *ptr)
{
    FILE *fp;
    
    if ((fp = fopen("/proc/loadavg", "rt")) == NULL)
	return;
    
    if (fscanf(fp, "%lf %lf %lf", &ptr[0], &ptr[1], &ptr[2]) != 3)
	ptr[0] = ptr[1] = ptr[2] = 0;
    
    fclose(fp);
}

static void mcc_src_load_avg_init(GTypeInstance *obj, gpointer klass)
{
}

static void mcc_src_load_avg_finalize(GObject *object)
{
    (*G_OBJECT_CLASS(mcc_src_load_avg_parent_class)->finalize)(object);
}

static void mcc_src_load_avg_set_subidx(MccDataSource *datasrc)
{
    datasrc->min = 0.0;
    datasrc->max = 1.0;
    datasrc->nvalues = 1;
    
    datasrc->nfg = 1;
    datasrc->fg_labels = g_new0(gchar *, 1);
    datasrc->fg_labels[0] = g_strdup("Load");
    datasrc->default_fg = g_new0(GdkColor, 1);
    datasrc->default_fg[0].red = 0xffff;
    datasrc->default_fg[0].green = 0x0000;
    datasrc->default_fg[0].blue = 0x0000;
    
    datasrc->nbg = 1;
    datasrc->bg_labels = g_new0(gchar *, 1);
    datasrc->bg_labels[0] = g_strdup("Background");
    datasrc->default_bg = g_new0(GdkColor, 1);
    datasrc->default_bg[0].red = 0x0000;
    datasrc->default_bg[0].green = 0x0000;
    datasrc->default_bg[0].blue = 0x0000;
    
    datasrc->dynamic_scaling = TRUE;
    
    datasrc->sublabel = g_strdup(MCC_DATA_SOURCE_GET_CLASS(datasrc)->sublabels[datasrc->subidx]);
}

static MccValue *mcc_src_load_avg_get(MccDataSource *datasrc)
{
    MccSrcLoadAvg *src = MCC_SRC_LOAD_AVG(datasrc);
    MccSrcLoadAvgClass *src_class = MCC_SRC_LOAD_AVG_GET_CLASS(src);
    
    MccValue *value = mcc_value_new(1);
    mcc_value_set_value(value, 0, src_class->newdata[datasrc->subidx]);
    
    return value;
}
