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

#include <string.h>
#include "mccsrccpuload.h"

static gint cpuload_count_cpus(void);
static void cpuload_read_data(data_per_cpu *ptr, gint nr);

static void mcc_src_cpu_load_class_init(gpointer klass, gpointer class_data);
static void mcc_src_cpu_load_set_subidx(MccDataSource *datasrc);
static void mcc_src_cpu_load_read(MccDataSourceClass *datasrc_class);
static void mcc_src_cpu_load_init(GTypeInstance *obj, gpointer klass);
static void mcc_src_cpu_load_finalize(GObject *obj);
static MccValue *mcc_src_cpu_load_get(MccDataSource *datasrc);

static gpointer mcc_src_cpu_load_parent_class = NULL;

GType mcc_src_cpu_load_get_type(void)
{
    static GType type = 0;
    if (type == 0) {
	static GTypeInfo type_info = {
	    .class_size = sizeof(MccSrcCpuLoadClass),
	    
	    .base_init = NULL,
	    .base_finalize = NULL,
	    
	    .class_init = mcc_src_cpu_load_class_init,
	    .class_finalize = NULL,
	    .class_data = NULL,
	    
	    .instance_size = sizeof(MccSrcCpuLoad),
	    .n_preallocs = 0,
	    .instance_init = mcc_src_cpu_load_init,
	    
	    .value_table = NULL,
	};
	
	type = g_type_register_static(MCC_TYPE_DATA_SOURCE, "MccSrcCpuLoad", &type_info, 0);
    }
    
    return type;
}

static void mcc_src_cpu_load_class_init(gpointer klass, gpointer class_data)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    MccDataSourceClass *datasrc_class = MCC_DATA_SOURCE_CLASS(klass);
    MccSrcCpuLoadClass *src_class = MCC_SRC_CPU_LOAD_CLASS(klass);
    
    mcc_src_cpu_load_parent_class = g_type_class_peek_parent(klass);
    
    gobject_class->finalize = mcc_src_cpu_load_finalize;
    
    datasrc_class->label = g_strdup("CPU Load");
    datasrc_class->set_subidx = mcc_src_cpu_load_set_subidx;
    datasrc_class->read = mcc_src_cpu_load_read;
    datasrc_class->get = mcc_src_cpu_load_get;
    
    src_class->ncpu = cpuload_count_cpus();
    src_class->olddata = g_new0(data_per_cpu, src_class->ncpu + 1);
    src_class->newdata = g_new0(data_per_cpu, src_class->ncpu + 1);
    
    datasrc_class->sublabels = g_new0(gchar *, src_class->ncpu + 2);
    datasrc_class->sublabels[0] = g_strdup("Total");
    for (gint i = 0; i < src_class->ncpu; i++)
	datasrc_class->sublabels[i + 1] = g_strdup_printf("CPU %d", i);
    
    cpuload_read_data(src_class->newdata, src_class->ncpu + 1);
    memcpy(src_class->olddata, src_class->newdata, sizeof *src_class->olddata * (src_class->ncpu + 1));
}

static void mcc_src_cpu_load_read(MccDataSourceClass *datasrc_class)
{
    MccSrcCpuLoadClass *src_class = MCC_SRC_CPU_LOAD_CLASS(datasrc_class);
    
    memcpy(src_class->olddata, src_class->newdata, sizeof *src_class->olddata * (src_class->ncpu + 1));
    cpuload_read_data(src_class->newdata, src_class->ncpu + 1);
}

static gint cpuload_count_cpus(void)
{
    FILE *fp;
    
    if ((fp = fopen("/proc/stat", "rt")) == NULL)
	return 0;
    
    gint n;
    for (n = 0; ; n++) {
	char buf[1024];
	if (fgets(buf, sizeof buf, fp) == NULL)
	    break;
	
	if (strncmp(buf, "cpu", 3) != 0)
	    break;
    }
    
    fclose(fp);
    
    return n < 2 ? 0 : n - 1;
}

static void cpuload_read_data(data_per_cpu *ptr, gint nr)
{
    FILE *fp;
    
    if ((fp = fopen("/proc/stat", "rt")) == NULL)
	return;
    
    int i;
    for (i = 0; i < nr; i++) {
	char buf[1024];
	if (fgets(buf, sizeof buf, fp) == NULL)
	    break;
	if (sscanf(buf, "%*s %" G_GINT64_FORMAT " %" G_GINT64_FORMAT " %" G_GINT64_FORMAT " %" G_GINT64_FORMAT " %" G_GINT64_FORMAT " %" G_GINT64_FORMAT " %" G_GINT64_FORMAT " %" G_GINT64_FORMAT "",
			&ptr[i][0], &ptr[i][1], &ptr[i][2], &ptr[i][3],
			&ptr[i][4], &ptr[i][5], &ptr[i][6], &ptr[i][7]) != 8)
	    break;
    }
    
    fclose(fp);
}

static void mcc_src_cpu_load_init(GTypeInstance *obj, gpointer klass)
{
}

static void mcc_src_cpu_load_finalize(GObject *object)
{
    (*G_OBJECT_CLASS(mcc_src_cpu_load_parent_class)->finalize)(object);
}

static void mcc_src_cpu_load_set_subidx(MccDataSource *datasrc)
{
    datasrc->min = 0.0;
    datasrc->max = 1.0;
    datasrc->nvalues = 7;
    
    datasrc->nfg = 7;
    datasrc->fg_labels = g_new0(gchar *, 7);
    datasrc->fg_labels[0] = g_strdup("user");
    datasrc->fg_labels[1] = g_strdup("nice");
    datasrc->fg_labels[2] = g_strdup("sys");
    datasrc->fg_labels[3] = g_strdup("iowait");
    datasrc->fg_labels[4] = g_strdup("irq");
    datasrc->fg_labels[5] = g_strdup("softirq");
    datasrc->fg_labels[6] = g_strdup("steal");
    datasrc->default_fg = g_new0(GdkColor, 7);
    datasrc->default_fg[0].red = 0x0000;	// user
    datasrc->default_fg[0].green = 0x0000;
    datasrc->default_fg[0].blue = 0xffff;
    datasrc->default_fg[1].red = 0x8080;	// nice
    datasrc->default_fg[1].green = 0x8080;
    datasrc->default_fg[1].blue = 0xffff;
    datasrc->default_fg[2].red = 0xffff;	// sys
    datasrc->default_fg[2].green = 0x0000;
    datasrc->default_fg[2].blue = 0xffff;
    datasrc->default_fg[3].red = 0x0000;	// iowait
    datasrc->default_fg[3].green = 0xffff;
    datasrc->default_fg[3].blue = 0x0000;
    datasrc->default_fg[4].red = 0x0000;	// irq
    datasrc->default_fg[4].green = 0xffff;
    datasrc->default_fg[4].blue = 0x0000;
    datasrc->default_fg[5].red = 0x0000;	// softirq
    datasrc->default_fg[5].green = 0xffff;
    datasrc->default_fg[5].blue = 0x0000;
    datasrc->default_fg[6].red = 0x0000;	// steal
    datasrc->default_fg[6].green = 0xffff;
    datasrc->default_fg[6].blue = 0x0000;
    
    datasrc->nbg = 1;
    datasrc->bg_labels = g_new0(gchar *, 2);
    datasrc->bg_labels[0] = g_strdup("Background");
    datasrc->default_bg = g_new0(GdkColor, 1);
    datasrc->default_bg[0].red = 0x0000;
    datasrc->default_bg[0].green = 0x0000;
    datasrc->default_bg[0].blue = 0x0000;
    
    datasrc->sublabel = g_strdup(MCC_DATA_SOURCE_GET_CLASS(datasrc)->sublabels[datasrc->subidx]);
}

static MccValue *mcc_src_cpu_load_get(MccDataSource *datasrc)
{
    MccSrcCpuLoad *src = MCC_SRC_CPU_LOAD(datasrc);
    MccSrcCpuLoadClass *src_class = MCC_SRC_CPU_LOAD_GET_CLASS(src);
    
    gdouble vals[NR_DATA];
    
    for (gint i = 0; i < NR_DATA; i++)
	vals[i] = 0;
    
    gint64 total
	    = src_class->newdata[datasrc->subidx][0] - src_class->olddata[datasrc->subidx][0]
	    + src_class->newdata[datasrc->subidx][1] - src_class->olddata[datasrc->subidx][1]
	    + src_class->newdata[datasrc->subidx][2] - src_class->olddata[datasrc->subidx][2]
	    + src_class->newdata[datasrc->subidx][3] - src_class->olddata[datasrc->subidx][3]
	    + src_class->newdata[datasrc->subidx][4] - src_class->olddata[datasrc->subidx][4]
	    + src_class->newdata[datasrc->subidx][5] - src_class->olddata[datasrc->subidx][5]
	    + src_class->newdata[datasrc->subidx][6] - src_class->olddata[datasrc->subidx][6]
	    + src_class->newdata[datasrc->subidx][7] - src_class->olddata[datasrc->subidx][7];
    // fixme: if total==0.
    vals[0] = (src_class->newdata[datasrc->subidx][0] - src_class->olddata[datasrc->subidx][0]) / (gdouble) total;	// user
    vals[1] = (src_class->newdata[datasrc->subidx][1] - src_class->olddata[datasrc->subidx][1]) / (gdouble) total;	// nice
    vals[2] = (src_class->newdata[datasrc->subidx][2] - src_class->olddata[datasrc->subidx][2]) / (gdouble) total;	// sys
    vals[3] = (src_class->newdata[datasrc->subidx][4] - src_class->olddata[datasrc->subidx][4]) / (gdouble) total;	// iowait
    vals[4] = (src_class->newdata[datasrc->subidx][5] - src_class->olddata[datasrc->subidx][5]) / (gdouble) total;	// irq
    vals[5] = (src_class->newdata[datasrc->subidx][6] - src_class->olddata[datasrc->subidx][6]) / (gdouble) total;	// softirq
    vals[6] = (src_class->newdata[datasrc->subidx][7] - src_class->olddata[datasrc->subidx][7]) / (gdouble) total;	// steal
    
    MccValue *value = mcc_value_new(NR_DATA - 1);
    for (gint i = 0; i < NR_DATA - 1; i++) {
	mcc_value_set_value(value, i, vals[i]);
	mcc_value_set_foreground(value, i, i);
    }
    
    return value;
}
