#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include "mccgraph.h"
#include "mccvalue.h"
#define DATASRC_CONTEXT_T cpuload_work_t
#include "datasrc.h"

#define NR_DATA 8

typedef gint64 data_per_cpu[NR_DATA];

static struct cpuload_t {
    gint ncpu;
    data_per_cpu *olddata;
    data_per_cpu *newdata;
} work;

struct cpuload_work_t {
    gint idx;
    struct datasrc_context_info_t info;
};

static const gchar * const fg_labels[] = {
    "user",
    "nice",
    "sys",
    "iowait",
    "irq",
    "softirq",
    "steal",
};

static const gchar * const bg_labels[] = {
    "bg",
};

static const GdkColor default_fg[] = {
    { .pixel = 0, .red = 0x0000, .green = 0x0000, .blue = 0xffff },	// user
    { .pixel = 0, .red = 0xffff, .green = 0x0000, .blue = 0x0000 },	// nice
    { .pixel = 0, .red = 0xffff, .green = 0x0000, .blue = 0xffff },	// sys
    { .pixel = 0, .red = 0x0000, .green = 0xffff, .blue = 0x0000 },	// iowait
    { .pixel = 0, .red = 0x0000, .green = 0xffff, .blue = 0xffff },	// irq
    { .pixel = 0, .red = 0xffff, .green = 0xffff, .blue = 0x0000 },	// softirq
    { .pixel = 0, .red = 0xffff, .green = 0xffff, .blue = 0xffff },	// steal
};

static const GdkColor default_bg[] = {
    { .pixel = 0, .red = 0x0000, .green = 0x0000, .blue = 0x0000 },
};

static const gchar *sublabels[] = {
    "total",
    "cpu0",
    "cpu1",
    NULL
};

static const struct datasrc_info_t sinfo = {
    .src = &linux_cpuload_datasrc,
    
    .label = "cpuload",
    .sublabels = sublabels,
};

static const struct datasrc_context_info_t info = {
    .src = &linux_cpuload_datasrc,
    
    .min = 0.0,
    .max = 1.0,
    .nvalues = 7,
    
    .nfg = 7,
    .fg_labels = fg_labels,
    .default_fg = default_fg,
    
    .nbg = 1,
    .bg_labels = bg_labels,
    .default_bg = default_bg,
    
    .sublabel = NULL,
};

static void cpuload_read_data(data_per_cpu *ptr, gint nr);

static void cpuload_init(void)
{
    struct cpuload_t *ww = &work;
    
    memset(ww, 0, sizeof *ww);
    
    ww->ncpu = 2;
    ww->olddata = g_new0(data_per_cpu, ww->ncpu + 1);
    ww->newdata = g_new0(data_per_cpu, ww->ncpu + 1);
    
    cpuload_read_data(ww->newdata, ww->ncpu + 1);
    memcpy(ww->olddata, ww->newdata, sizeof *ww->olddata * (ww->ncpu + 1));
}

static void cpuload_read(void)
{
    struct cpuload_t *ww = &work;
    
    memcpy(ww->olddata, ww->newdata, sizeof *ww->olddata * (ww->ncpu + 1));
    cpuload_read_data(ww->newdata, ww->ncpu + 1);
}

static void cpuload_fini(void)
{
    struct cpuload_t *ww = &work;
    
    ww->ncpu = 0;
    free(ww->olddata);
    ww->olddata = NULL;
    free(ww->newdata);
    ww->newdata = NULL;
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

static struct datasrc_context_t *cpuload_new(gint subidx)
{
    struct cpuload_work_t *w = g_new0(struct cpuload_work_t, 1);
    
    w->idx = subidx;
    w->info = info;
    w->info.sublabel = sublabels[w->idx];
    
    return datasrc_context_base_ptr(w);
}

static void cpuload_destroy(struct datasrc_context_t *w0)
{
    struct cpuload_work_t *w = datasrc_context_ptr(w0);
}

static MccValue *cpuload_get(struct datasrc_context_t *w0)
{
    struct cpuload_t *ww = &work;
    struct cpuload_work_t *w = datasrc_context_ptr(w0);
    gdouble vals[NR_DATA];
    
    for (gint i = 0; i < NR_DATA; i++)
	vals[i] = 0;
    
    gint64 total
	    = ww->newdata[w->idx][0] - ww->olddata[w->idx][0]
	    + ww->newdata[w->idx][1] - ww->olddata[w->idx][1]
	    + ww->newdata[w->idx][2] - ww->olddata[w->idx][2]
	    + ww->newdata[w->idx][3] - ww->olddata[w->idx][3]
	    + ww->newdata[w->idx][4] - ww->olddata[w->idx][4]
	    + ww->newdata[w->idx][5] - ww->olddata[w->idx][5]
	    + ww->newdata[w->idx][6] - ww->olddata[w->idx][6]
	    + ww->newdata[w->idx][7] - ww->olddata[w->idx][7];
    // fixme: if total==0.
    vals[0] = (ww->newdata[w->idx][0] - ww->olddata[w->idx][0]) / (gdouble) total;	// user
    vals[1] = (ww->newdata[w->idx][1] - ww->olddata[w->idx][1]) / (gdouble) total;	// nice
    vals[2] = (ww->newdata[w->idx][2] - ww->olddata[w->idx][2]) / (gdouble) total;	// sys
    vals[3] = (ww->newdata[w->idx][4] - ww->olddata[w->idx][4]) / (gdouble) total;	// iowait
    vals[4] = (ww->newdata[w->idx][5] - ww->olddata[w->idx][5]) / (gdouble) total;	// irq
    vals[5] = (ww->newdata[w->idx][6] - ww->olddata[w->idx][6]) / (gdouble) total;	// softirq
    vals[6] = (ww->newdata[w->idx][7] - ww->olddata[w->idx][7]) / (gdouble) total;	// steal
    
    MccValue *value = mcc_value_new(NR_DATA - 1);
    for (gint i = 0; i < NR_DATA - 1; i++) {
	mcc_value_set_value(value, i, vals[i]);
	mcc_value_set_foreground(value, i, i);
    }
    
    return value;
}

static const struct datasrc_info_t *cpuload_sinfo(void)
{
    return &sinfo;
}

static const struct datasrc_context_info_t *cpuload_info(struct datasrc_context_t *w0)
{
    struct cpuload_work_t *w = datasrc_context_ptr(w0);
    return &w->info;
}

struct datasrc_t linux_cpuload_datasrc = {
    .sinit = cpuload_init,
    .sinfo = cpuload_sinfo,
    .sread = cpuload_read,
    .sfini = cpuload_fini,
    
    .new = cpuload_new,
    .get = cpuload_get,
    .info = cpuload_info,
    .destroy = cpuload_destroy,
};
