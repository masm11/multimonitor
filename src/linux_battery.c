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
    gint32 last_full_capacity;
} work;

struct battery_work_t {
    gint idx;
    struct datasrc_context_info_t info;
};

static const gchar * const fg_labels[] = {
    "not charging",
    "charging",
};

static const gchar * const bg_labels[] = {
    "AC off",
    "AC on",
};

static const GdkColor default_fg[] = {
    { .pixel = 0, .red = 0x0000, .green = 0x0000, .blue = 0xffff },
    { .pixel = 0, .red = 0x0000, .green = 0xffff, .blue = 0xffff },
};

static const GdkColor default_bg[] = {
    { .pixel = 0, .red = 0x0000, .green = 0x0000, .blue = 0x0000 },
    { .pixel = 0, .red = 0x8000, .green = 0x0000, .blue = 0x0000 },
};

static const gchar *sublabels[] = {
    "battery",
    NULL
};

static const struct datasrc_info_t sinfo = {
    .src = &linux_battery_datasrc,
    
    .label = "battery",
    .sublabels = sublabels,
};

static const struct datasrc_context_info_t info = {
    .src = &linux_battery_datasrc,
    
    .min = 0.0,
    .max = 1.0,
    .nvalues = 1,
    
    .nfg = 2,
    .fg_labels = fg_labels,
    .default_fg = default_fg,
    
    .nbg = 2,
    .bg_labels = bg_labels,
    .default_bg = default_bg,
    
    .sublabel = NULL,
};

static gint32 battery_read_last_full_capacity(void);
static void battery_read_data(data_per_batt *ptr);

static void battery_init(void)
{
    struct battery_t *ww = &work;
    
    memset(ww, 0, sizeof *ww);
    
    ww->nbatt = 1;
    ww->olddata = g_new0(data_per_batt, ww->nbatt);
    ww->newdata = g_new0(data_per_batt, ww->nbatt);
    ww->last_full_capacity = battery_read_last_full_capacity();
    
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

static gint32 battery_read_last_full_capacity(void)
{
    FILE *fp;
    gint32 full;
    char buf[1024];
    
    if ((fp = fopen("/proc/acpi/battery/BAT0/info", "rt")) != NULL) {
	while (fgets(buf, sizeof buf, fp) != NULL) {
	    gint32 n;
	    if (sscanf(buf, "last full capacity: %" G_GINT32_FORMAT " mWh", &n) == 1)
		full = n;
	}
	
	fclose(fp);
    }
    
    return full;
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
    
    gint32 cur = 0;
    
    if ((fp = fopen("/proc/acpi/battery/BAT0/state", "rt")) != NULL) {
	while (fgets(buf, sizeof buf, fp) != NULL) {
	    gint32 n;
	    char bf[32];
	    if (sscanf(buf, "remaining capacity: %" G_GINT32_FORMAT " mWh", &n) == 1)
		cur = n;
	    else if (sscanf(buf, "charging state: %s", bf) == 1 && strcmp(bf, "charging") == 0) {
		ptr->charging = TRUE;
	    }
	}
	
	fclose(fp);
    }
    
    if (work.last_full_capacity > 0)
	ptr->ratio = (gdouble) cur / work.last_full_capacity;
}

static struct datasrc_context_t *battery_new(gint subidx)
{
    struct battery_work_t *w = g_new0(struct battery_work_t, 1);
    
    w->info = info;
    w->info.sub_idx = subidx;
    w->info.sublabel = sublabels[0];
    
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
    mcc_value_set_foreground(value, 0, work.newdata->charging ? 1 : 0);
    mcc_value_set_background(value, work.newdata->ac ? 1 : 0);
    
    return value;
}

static const struct datasrc_info_t *battery_sinfo(void)
{
    return &sinfo;
}

static const struct datasrc_context_info_t *battery_info(struct datasrc_context_t *w0)
{
    struct battery_work_t *w = datasrc_context_ptr(w0);
    return &w->info;
}

struct datasrc_t linux_battery_datasrc = {
    .sinit = battery_init,
    .sinfo = battery_sinfo,
    .sread = battery_read,
    .sfini = battery_fini,
    
    .new = battery_new,
    .get = battery_get,
    .info = battery_info,
    .destroy = battery_destroy,
};
