#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include "mccgraph.h"
#include "mccvalue.h"
#define DATASRC_CONTEXT_T battery_work_t
#include "datasrc.h"

typedef struct {
    gboolean ac;
    gboolean charging;
    gdouble ratio;
} data_per_batt;

static struct battery_t {
    gint nbatt;
    data_per_batt *olddata;
    data_per_batt *newdata;
} work;

struct battery_work_t {
    gint idx;
};

static const gchar * const labels[] = {
    "ratio",
};

static const GdkColor default_fg[] = {
    { .pixel = 0, .red = 0x0000, .green = 0x0000, .blue = 0xffff },
};

static const GdkColor default_bg[] = {
    { .pixel = 0, .red = 0x0000, .green = 0x0000, .blue = 0x0000 },
};

static const struct datasrc_info_t info = {
    .min = 0.0,
    .max = 1.0,
    .nvalues = 1,
    
    .nfg = 1,
    .value_labels = labels,
    .default_fg = default_fg,
    
    .nbg = 1,
    .default_bg = default_bg,
};

static void battery_read_data(data_per_batt *ptr);

static void battery_init(void)
{
    struct battery_t *ww = &work;
    
    memset(ww, 0, sizeof *ww);
    
    ww->nbatt = 1;
    ww->olddata = g_new0(data_per_batt, ww->nbatt);
    ww->newdata = g_new0(data_per_batt, ww->nbatt);
    
    battery_read_data(ww->newdata);
    memcpy(ww->olddata, ww->newdata, sizeof *ww->olddata);
}

static void battery_read(void)
{
    struct battery_t *ww = &work;
    
    memcpy(ww->olddata, ww->newdata, sizeof *ww->olddata);
    battery_read_data(ww->newdata);
}

static void battery_fini(void)
{
    struct battery_t *ww = &work;
    
    free(ww->olddata);
    ww->olddata = NULL;
    free(ww->newdata);
    ww->newdata = NULL;
}

static void battery_read_data(data_per_batt *ptr)
{
    char buf[1024];
    FILE *fp;
    
    ptr->ac = FALSE;
    ptr->charging = FALSE;
    ptr->ratio = 0.0;
    
    if ((fp = fopen("/proc/acpi/ac_adapter/AC/state", "rt")) != NULL) {
	while (fgets(buf, sizeof buf, fp) != NULL) {
	    if (strncmp(buf, "state:", 6) == 0) {
		if (strstr(buf, "on-line") != NULL)
		    ptr->ac = TRUE;
	    }
	}
	
	fclose(fp);
    }
    
    gint32 full = 0, cur = 0;
    
    if ((fp = fopen("/proc/acpi/battery/BAT0/info", "rt")) != NULL) {
	while (fgets(buf, sizeof buf, fp) != NULL) {
	    gint32 n;
	    if (sscanf(buf, "last full capacity: %" G_GINT32_FORMAT " mWh", &n) == 1)
		full = n;
	}
	
	fclose(fp);
    }
    
    if ((fp = fopen("/proc/acpi/battery/BAT0/state", "rt")) != NULL) {
	while (fgets(buf, sizeof buf, fp) != NULL) {
	    gint32 n;
	    if (sscanf(buf, "remaining capacity: %" G_GINT32_FORMAT " mWh", &n) == 1)
		cur = n;
	    else if (strncmp(buf, "charging state:", 15) == 0) {
		if (strstr(buf, "charging") != NULL)
		    ptr->charging = TRUE;
	    }
	}
	
	fclose(fp);
    }
    
    if (full > 0)
	ptr->ratio = (gdouble) cur / full;
}

static struct datasrc_context_t *battery_new(void)
{
    struct battery_work_t *w = g_new0(struct battery_work_t, 1);
    
    return datasrc_context_base_ptr(w);
}

static void battery_destroy(struct datasrc_context_t *w0)
{
    struct battery_work_t *w = datasrc_context_ptr(w0);
}

static MccValue *battery_get(struct datasrc_context_t *w0)
{
    struct battery_t *ww = &work;
    struct battery_work_t *w = datasrc_context_ptr(w0);
    
    MccValue *value = mcc_value_new(1);
    mcc_value_set_value(value, 0, work.newdata->ratio);
    
    return value;
}

static const struct datasrc_info_t *battery_info(struct datasrc_context_t *w0)
{
    return &info;
}

struct datasrc_t linux_battery_datasrc = {
    .sinit = battery_init,
    .sread = battery_read,
    .sfini = battery_fini,
    
    .new = battery_new,
    .get = battery_get,
    .info = battery_info,
    .destroy = battery_destroy,
};
