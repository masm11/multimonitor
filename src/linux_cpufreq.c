#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include "mccgraph.h"
#include "mccvalue.h"
#define DATASRC_CONTEXT_T cpufreq_work_t
#include "datasrc.h"

#define NR_CPU 2

typedef gint64 data_per_cpu;

static struct cpufreq_t {
    gint ncpu;
    data_per_cpu *olddata;
    data_per_cpu *newdata;
} work;

struct cpufreq_work_t {
    gint idx;
};

static const gchar *labels[] = {
    "frequency",
};

static GdkColor default_fg[] = {
    { .pixel = 0, .red = 0xffff, .green = 0xffff, .blue = 0xffff },
};

static GdkColor default_bg[] = {
    { .pixel = 0, .red = 0x0000, .green = 0x0000, .blue = 0x0000 },
};

static const struct datasrc_info_t info = {
    .min = 0.0,
    .max = 2001000000,
    .nvalues = 1,
    .value_labels = labels,
    .default_fg = default_fg,
    .nbg = 1,
    .default_bg = default_bg,
};

static void cpufreq_read_data(data_per_cpu *ptr, gint nr);

static void cpufreq_init(void)
{
    struct cpufreq_t *ww = &work;
    
    memset(ww, 0, sizeof *ww);
    
    ww->ncpu = 2;
    ww->olddata = g_new0(data_per_cpu, ww->ncpu);
    ww->newdata = g_new0(data_per_cpu, ww->ncpu);
    
    cpufreq_read_data(ww->newdata, ww->ncpu);
    memcpy(ww->olddata, ww->newdata, sizeof *ww->olddata * ww->ncpu);
}

static void cpufreq_read(void)
{
    struct cpufreq_t *ww = &work;
    
    memcpy(ww->olddata, ww->newdata, sizeof *ww->olddata * ww->ncpu);
    cpufreq_read_data(ww->newdata, ww->ncpu);
}

static void cpufreq_fini(void)
{
    struct cpufreq_t *ww = &work;
    
    ww->ncpu = 0;
    free(ww->olddata);
    ww->olddata = NULL;
    free(ww->newdata);
    ww->newdata = NULL;
}

static void cpufreq_read_data(data_per_cpu *ptr, gint nr)
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

static struct datasrc_context_t *cpufreq_new(void)
{
    struct cpufreq_work_t *w = g_new0(struct cpufreq_work_t, 1);
    
    static gint idx = 0;
    
    w->idx = idx++ % (work.ncpu);
    
    return datasrc_context_base_ptr(w);
}

static void cpufreq_destroy(struct datasrc_context_t *w0)
{
    struct cpufreq_work_t *w = datasrc_context_ptr(w0);
}

static MccValue *cpufreq_get(struct datasrc_context_t *w0)
{
    struct cpufreq_t *ww = &work;
    struct cpufreq_work_t *w = datasrc_context_ptr(w0);
    
    MccValue *value = mcc_value_new(1);
    mcc_value_set_value(value, 0, ww->newdata[w->idx]);
    
    return value;
}

static const struct datasrc_info_t *cpufreq_info(struct datasrc_context_t *w0)
{
    return &info;
}

struct datasrc_t linux_cpufreq_datasrc = {
    .sinit = cpufreq_init,
    .sread = cpufreq_read,
    .sfini = cpufreq_fini,
    
    .new = cpufreq_new,
    .get = cpufreq_get,
    .info = cpufreq_info,
    .destroy = cpufreq_destroy,
};
