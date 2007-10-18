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
#include <string.h>
#include "mccsrcmemory.h"

#define NR_DATA 4

static void memory_read_data(gint64 *ptr);

static void mcc_src_memory_class_init(gpointer klass, gpointer class_data);
static void mcc_src_memory_set_subidx(MccDataSource *datasrc);
static void mcc_src_memory_read(MccDataSourceClass *datasrc_class);
static void mcc_src_memory_init(GTypeInstance *obj, gpointer klass);
static void mcc_src_memory_finalize(GObject *obj);
static MccValue *mcc_src_memory_get(MccDataSource *datasrc);

static gpointer mcc_src_memory_parent_class = NULL;

GType mcc_src_memory_get_type(void)
{
    static GType type = 0;
    if (type == 0) {
	static GTypeInfo type_info = {
	    .class_size = sizeof(MccSrcMemoryClass),
	    
	    .base_init = NULL,
	    .base_finalize = NULL,
	    
	    .class_init = mcc_src_memory_class_init,
	    .class_finalize = NULL,
	    .class_data = NULL,
	    
	    .instance_size = sizeof(MccSrcMemory),
	    .n_preallocs = 0,
	    .instance_init = mcc_src_memory_init,
	    
	    .value_table = NULL,
	};
	
	type = g_type_register_static(MCC_TYPE_DATA_SOURCE, "MccSrcMemory", &type_info, 0);
    }
    
    return type;
}

static void mcc_src_memory_class_init(gpointer klass, gpointer class_data)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    MccDataSourceClass *datasrc_class = MCC_DATA_SOURCE_CLASS(klass);
    MccSrcMemoryClass *src_class = MCC_SRC_MEMORY_CLASS(klass);
    
    mcc_src_memory_parent_class = g_type_class_peek_parent(klass);
    
    gobject_class->finalize = mcc_src_memory_finalize;
    
    datasrc_class->label = g_strdup("Memory");
    datasrc_class->set_subidx = mcc_src_memory_set_subidx;
    datasrc_class->read = mcc_src_memory_read;
    datasrc_class->get = mcc_src_memory_get;
    
    src_class->olddata = g_new0(gint64, NR_DATA);
    src_class->newdata = g_new0(gint64, NR_DATA);
    
    datasrc_class->sublabels = g_new0(gchar *, 2);
    datasrc_class->sublabels[0] = g_strdup("Usage");
    
    memory_read_data(src_class->newdata);
    memcpy(src_class->olddata, src_class->newdata, sizeof *src_class->olddata * NR_DATA);
    src_class->max = src_class->newdata[0];
}

static void mcc_src_memory_read(MccDataSourceClass *datasrc_class)
{
    MccSrcMemoryClass *src_class = MCC_SRC_MEMORY_CLASS(datasrc_class);
    
    memcpy(src_class->olddata, src_class->newdata, sizeof *src_class->olddata * NR_DATA);
    memory_read_data(src_class->newdata);
}

static void memory_read_data(gint64 *ptr)
{
    FILE *fp;
    gint64 total = -1, free = -1, buff = -1, cache = -1;
    
    ptr[0] = ptr[1] = ptr[2] = ptr[3] = 0;
    
    if ((fp = fopen("/proc/meminfo", "rt")) == NULL)
	return;
    
    while (TRUE) {
	char buf[1024];
	if (fgets(buf, sizeof buf, fp) == NULL)
	    break;
	gint v;
	if (strncmp(buf, "MemTotal:", 9) == 0) {
	    if (sscanf(buf, "%*s %d kB", &v) == 1)
		total = (gint64) v * 1024;
	} else if (strncmp(buf, "MemFree:", 8) == 0) {
	    if (sscanf(buf, "%*s %d kB", &v) == 1)
		free = (gint64) v * 1024;
	} else if (strncmp(buf, "Buffers:", 8) == 0) {
	    if (sscanf(buf, "%*s %d kB", &v) == 1)
		buff = (gint64) v * 1024;
	} else if (strncmp(buf, "Cached:", 7) == 0) {
	    if (sscanf(buf, "%*s %d kB", &v) == 1)
		cache = (gint64) v * 1024;
	}
    }
    
    fclose(fp);
    
    if (total < 0 || free < 0 || buff < 0 || cache < 0)
	return;
    
    ptr[0] = total;
    ptr[1] = total - free - buff - cache;
    ptr[2] = buff;
    ptr[3] = cache;
}

static void mcc_src_memory_init(GTypeInstance *obj, gpointer klass)
{
}

static void mcc_src_memory_finalize(GObject *object)
{
    (*G_OBJECT_CLASS(mcc_src_memory_parent_class)->finalize)(object);
}

static void mcc_src_memory_set_subidx(MccDataSource *datasrc)
{
    datasrc->min = 0.0;
    datasrc->max = MCC_SRC_MEMORY_GET_CLASS(datasrc)->max;
    datasrc->nvalues = NR_DATA - 1;
    
    datasrc->nfg = NR_DATA - 1;
    datasrc->fg_labels = g_new0(gchar *, NR_DATA - 1);
    datasrc->fg_labels[0] = g_strdup("Used");
    datasrc->fg_labels[1] = g_strdup("Buffer");
    datasrc->fg_labels[2] = g_strdup("Cache");
    datasrc->default_fg = g_new0(GdkColor, NR_DATA - 1);
    datasrc->default_fg[0].red = 0x0000;	// user
    datasrc->default_fg[0].green = 0x0000;
    datasrc->default_fg[0].blue = 0xffff;
    datasrc->default_fg[1].red = 0xffff;	// nice
    datasrc->default_fg[1].green = 0x0000;
    datasrc->default_fg[1].blue = 0x0000;
    datasrc->default_fg[2].red = 0xffff;	// sys
    datasrc->default_fg[2].green = 0x0000;
    datasrc->default_fg[2].blue = 0xffff;
    
    datasrc->nbg = 1;
    datasrc->bg_labels = g_new0(gchar *, 2);
    datasrc->bg_labels[0] = g_strdup("Background");
    datasrc->default_bg = g_new0(GdkColor, 1);
    datasrc->default_bg[0].red = 0x0000;
    datasrc->default_bg[0].green = 0x0000;
    datasrc->default_bg[0].blue = 0x0000;
    
    datasrc->sublabel = g_strdup(MCC_DATA_SOURCE_GET_CLASS(datasrc)->sublabels[datasrc->subidx]);
}

static MccValue *mcc_src_memory_get(MccDataSource *datasrc)
{
    MccSrcMemory *src = MCC_SRC_MEMORY(datasrc);
    MccSrcMemoryClass *src_class = MCC_SRC_MEMORY_GET_CLASS(src);
    
    MccValue *value = mcc_value_new(NR_DATA - 1);
    for (gint i = 0; i < NR_DATA - 1; i++) {
	mcc_value_set_value(value, i, src_class->newdata[i + 1]);
	mcc_value_set_foreground(value, i, i);
    }
    
    return value;
}
