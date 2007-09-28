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
#include <unistd.h>
#include "mccsrcdiskio.h"

#define NR_DATA 2

static void disk_io_read_data(gint64 *ptr);

static void mcc_src_disk_io_class_init(gpointer klass, gpointer class_data);
static void mcc_src_disk_io_set_subidx(MccDataSource *datasrc);
static void mcc_src_disk_io_read(MccDataSourceClass *datasrc_class);
static void mcc_src_disk_io_init(GTypeInstance *obj, gpointer klass);
static void mcc_src_disk_io_finalize(GObject *obj);
static MccValue *mcc_src_disk_io_get(MccDataSource *datasrc);

static gpointer mcc_src_disk_io_parent_class = NULL;

GType mcc_src_disk_io_get_type(void)
{
    static GType type = 0;
    if (type == 0) {
	static GTypeInfo type_info = {
	    .class_size = sizeof(MccSrcDiskIOClass),
	    
	    .base_init = NULL,
	    .base_finalize = NULL,
	    
	    .class_init = mcc_src_disk_io_class_init,
	    .class_finalize = NULL,
	    .class_data = NULL,
	    
	    .instance_size = sizeof(MccSrcDiskIO),
	    .n_preallocs = 0,
	    .instance_init = mcc_src_disk_io_init,
	    
	    .value_table = NULL,
	};
	
	type = g_type_register_static(MCC_TYPE_DATA_SOURCE, "MccSrcDiskIO", &type_info, 0);
    }
    
    return type;
}

static void mcc_src_disk_io_class_init(gpointer klass, gpointer class_data)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    MccDataSourceClass *datasrc_class = MCC_DATA_SOURCE_CLASS(klass);
    MccSrcDiskIOClass *src_class = MCC_SRC_DISK_IO_CLASS(klass);
    
    mcc_src_disk_io_parent_class = g_type_class_peek_parent(klass);
    
    gobject_class->finalize = mcc_src_disk_io_finalize;
    
    datasrc_class->label = g_strdup("Disk I/O");
    datasrc_class->set_subidx = mcc_src_disk_io_set_subidx;
    datasrc_class->read = mcc_src_disk_io_read;
    datasrc_class->get = mcc_src_disk_io_get;
    
    src_class->olddata = g_new0(gint64, NR_DATA);
    src_class->newdata = g_new0(gint64, NR_DATA);
    
    datasrc_class->sublabels = g_new0(gchar *, 2);
    datasrc_class->sublabels[0] = g_strdup("Total");
    
    disk_io_read_data(src_class->newdata);
    memcpy(src_class->olddata, src_class->newdata, sizeof *src_class->olddata * NR_DATA);
}

static void mcc_src_disk_io_read(MccDataSourceClass *datasrc_class)
{
    MccSrcDiskIOClass *src_class = MCC_SRC_DISK_IO_CLASS(datasrc_class);
    
    memcpy(src_class->olddata, src_class->newdata, sizeof *src_class->olddata * NR_DATA);
    disk_io_read_data(src_class->newdata);
}

static void disk_io_read_data(gint64 *ptr)
{
    GDir *dir;
    
    if ((dir = g_dir_open("/sys/block", 0, NULL)) == NULL)
	return;
    
    gint64 totalrd = 0, totalwr = 0;
    while (TRUE) {
	const gchar *name = g_dir_read_name(dir);
	char path[1024];
	
	if (name == NULL)
	    break;
	sprintf(path, "/sys/block/%s/device", name);
	
	if (access(path, F_OK) == -1)
	    continue;
	
	sprintf(path, "/sys/block/%s/stat", name);
	
	FILE *fp;
	
	if ((fp = fopen(path, "rt")) == NULL)
	    continue;
	
	gint64 rd, wr;
	if (fscanf(fp, "%*u %*u %" G_GINT64_FORMAT " %*u %*u %*u %" G_GINT64_FORMAT " %*u %*u %*u %*u", &rd, &wr) == 2) {
	    totalrd += rd;
	    totalwr += wr;
	}
	
	fclose(fp);
    }
    
    g_dir_close(dir);
    
    ptr[0] = totalrd * 512;
    ptr[1] = totalwr * 512;
}

static void mcc_src_disk_io_init(GTypeInstance *obj, gpointer klass)
{
}

static void mcc_src_disk_io_finalize(GObject *object)
{
    (*G_OBJECT_CLASS(mcc_src_disk_io_parent_class)->finalize)(object);
}

static void mcc_src_disk_io_set_subidx(MccDataSource *datasrc)
{
    datasrc->min = 0.0;
    datasrc->max = 16 * 1024 * 1024;
    datasrc->nvalues = NR_DATA;
    
    datasrc->nfg = NR_DATA;
    datasrc->fg_labels = g_new0(gchar *, NR_DATA);
    datasrc->fg_labels[0] = g_strdup("Read");
    datasrc->fg_labels[1] = g_strdup("Write");
    datasrc->default_fg = g_new0(GdkColor, NR_DATA - 1);
    datasrc->default_fg[0].red = 0x0000;
    datasrc->default_fg[0].green = 0xffff;
    datasrc->default_fg[0].blue = 0x0000;
    datasrc->default_fg[1].red = 0xffff;
    datasrc->default_fg[1].green = 0x0000;
    datasrc->default_fg[1].blue = 0x0000;
    
    datasrc->nbg = 1;
    datasrc->bg_labels = g_new0(gchar *, 2);
    datasrc->bg_labels[0] = g_strdup("Background");
    datasrc->default_bg = g_new0(GdkColor, 1);
    datasrc->default_bg[0].red = 0x0000;
    datasrc->default_bg[0].green = 0x0000;
    datasrc->default_bg[0].blue = 0x0000;
    
    datasrc->sublabel = g_strdup(MCC_DATA_SOURCE_GET_CLASS(datasrc)->sublabels[datasrc->subidx]);
}

static MccValue *mcc_src_disk_io_get(MccDataSource *datasrc)
{
    MccSrcDiskIO *src = MCC_SRC_DISK_IO(datasrc);
    MccSrcDiskIOClass *src_class = MCC_SRC_DISK_IO_GET_CLASS(src);
    
    MccValue *value = mcc_value_new(NR_DATA);
    for (gint i = 0; i < NR_DATA; i++) {
	mcc_value_set_value(value, i, src_class->newdata[i] - src_class->olddata[i]);
	mcc_value_set_foreground(value, i, i);
    }
    
    return value;
}
