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
#include "mccsrccpufreq.h"

static void cpufreq_get_conf(gint *ncpus, gint64 *maxfreq);
static void cpufreq_read_data(gint64 *ptr, gint nr);

static void mcc_src_cpu_freq_class_init(gpointer klass, gpointer class_data);
static void mcc_src_cpu_freq_set_subidx(MccDataSource *datasrc);
static void mcc_src_cpu_freq_read(MccDataSourceClass *datasrc_class);
static void mcc_src_cpu_freq_init(GTypeInstance *obj, gpointer klass);
static void mcc_src_cpu_freq_finalize(GObject *obj);
static MccValue *mcc_src_cpu_freq_get(MccDataSource *datasrc);

static gpointer mcc_src_cpu_freq_parent_class = NULL;

GType mcc_src_cpu_freq_get_type(void)
{
    static GType type = 0;
    if (type == 0) {
	static GTypeInfo type_info = {
	    .class_size = sizeof(MccSrcCpuFreqClass),
	    
	    .base_init = NULL,
	    .base_finalize = NULL,
	    
	    .class_init = mcc_src_cpu_freq_class_init,
	    .class_finalize = NULL,
	    .class_data = NULL,
	    
	    .instance_size = sizeof(MccSrcCpuFreq),
	    .n_preallocs = 0,
	    .instance_init = mcc_src_cpu_freq_init,
	    
	    .value_table = NULL,
	};
	
	type = g_type_register_static(MCC_TYPE_DATA_SOURCE, "MccSrcCpuFreq", &type_info, 0);
    }
    
    return type;
}

static void mcc_src_cpu_freq_class_init(gpointer klass, gpointer class_data)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    MccDataSourceClass *datasrc_class = MCC_DATA_SOURCE_CLASS(klass);
    MccSrcCpuFreqClass *src_class = MCC_SRC_CPU_FREQ_CLASS(klass);
    
    mcc_src_cpu_freq_parent_class = g_type_class_peek_parent(klass);
    
    gobject_class->finalize = mcc_src_cpu_freq_finalize;
    
    datasrc_class->label = g_strdup("CPU Freq");
    datasrc_class->set_subidx = mcc_src_cpu_freq_set_subidx;
    datasrc_class->read = mcc_src_cpu_freq_read;
    datasrc_class->get = mcc_src_cpu_freq_get;
    
    cpufreq_get_conf(&src_class->ncpu, &src_class->maxfreq);
    
    src_class->olddata = g_new0(gint64, src_class->ncpu);
    src_class->newdata = g_new0(gint64, src_class->ncpu);
    
    datasrc_class->sublabels = g_new0(gchar *, src_class->ncpu + 1);
    for (gint i = 0; i < src_class->ncpu; i++)
	datasrc_class->sublabels[i] = g_strdup_printf("CPU %d", i);
    
    cpufreq_read_data(src_class->newdata, src_class->ncpu);
    memcpy(src_class->olddata, src_class->newdata, sizeof *src_class->olddata * src_class->ncpu);
}

static void mcc_src_cpu_freq_read(MccDataSourceClass *datasrc_class)
{
    MccSrcCpuFreqClass *src_class = MCC_SRC_CPU_FREQ_CLASS(datasrc_class);
    
    memcpy(src_class->olddata, src_class->newdata, sizeof *src_class->olddata * src_class->ncpu);
    cpufreq_read_data(src_class->newdata, src_class->ncpu);
}

static void cpufreq_get_conf(gint *ncpus, gint64 *maxfreq)
{
    gint n = 0;
    gint max = 0;
    while (TRUE) {
	FILE *fp;
	char buf[1024];
	sprintf(buf, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_cur_freq", n);
	if ((fp = fopen(buf, "rt")) == NULL)
	    break;
	fclose(fp);
	
	sprintf(buf, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_available_frequencies", n);
	if ((fp = fopen(buf, "rt")) == NULL)
	    break;
	
	gint f;
	while (fscanf(fp, "%d", &f) == 1) {
	    if (f > max)
		max = f;
	}
	fclose(fp);
	
	n++;
    }
    
    *ncpus = n;
    *maxfreq = (gint64) max * 1000;
}

static void cpufreq_read_data(gint64 *ptr, gint nr)
{
    for (gint i = 0; i < nr; i++) {
	FILE *fp;
	char buf[1024];
	sprintf(buf, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_cur_freq", i);
	ptr[i] = 0;
	if ((fp = fopen(buf, "rt")) != NULL) {
	    if (fgets(buf, sizeof buf, fp) != NULL) {
		guint khz;
		if (sscanf(buf, "%u", &khz) == 1)
		    ptr[i] = (gint64) khz * 1000;
	    }
	    fclose(fp);
	}
    }
}

static void mcc_src_cpu_freq_init(GTypeInstance *obj, gpointer klass)
{
}

static void mcc_src_cpu_freq_finalize(GObject *object)
{
    (*G_OBJECT_CLASS(mcc_src_cpu_freq_parent_class)->finalize)(object);
}

static void mcc_src_cpu_freq_set_subidx(MccDataSource *datasrc)
{
    datasrc->min = 0.0;
    datasrc->max = MCC_SRC_CPU_FREQ_GET_CLASS(datasrc)->maxfreq;
    datasrc->nvalues = 1;
    
    datasrc->nfg = 1;
    datasrc->fg_labels = g_new0(gchar *, 2);
    datasrc->fg_labels[0] = g_strdup("Frequency");
    datasrc->default_fg = g_new0(GdkColor, 1);
    datasrc->default_fg[0].red = 0x0000;
    datasrc->default_fg[0].green = 0xb1b1;
    datasrc->default_fg[0].blue = 0xffff;
    
    datasrc->nbg = 1;
    datasrc->bg_labels = g_new0(gchar *, 2);
    datasrc->bg_labels[0] = g_strdup("Background");
    datasrc->default_bg = g_new0(GdkColor, 1);
    datasrc->default_bg[0].red = 0x0000;
    datasrc->default_bg[0].green = 0x0000;
    datasrc->default_bg[0].blue = 0x0000;
    
    datasrc->sublabel = g_strdup(MCC_DATA_SOURCE_GET_CLASS(datasrc)->sublabels[datasrc->subidx]);
}

static MccValue *mcc_src_cpu_freq_get(MccDataSource *datasrc)
{
    MccSrcCpuFreq *src = MCC_SRC_CPU_FREQ(datasrc);
    MccSrcCpuFreqClass *src_class = MCC_SRC_CPU_FREQ_GET_CLASS(src);
    MccValue *value = mcc_value_new(1);
    mcc_value_set_value(value, 0, src_class->newdata[src->data_source.subidx]);
    return value;
}
