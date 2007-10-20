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
#include "opendirat.h"
#include "battalert.h"
#include "mccsrcbattery.h"

static gint battery_read_config(gint *acdirfd, gint32 **fulls, gint **dirfds);
static void battery_read_data(data_per_batt *ptr, gint acdirfd,
	gint nbatt, gint32 *last_full_capacities, gint *dirfds);

static void mcc_src_battery_class_init(gpointer klass, gpointer class_data);
static void mcc_src_battery_set_subidx(MccDataSource *datasrc);
static void mcc_src_battery_read(MccDataSourceClass *datasrc_class);
static void mcc_src_battery_init(GTypeInstance *obj, gpointer klass);
static void mcc_src_battery_finalize(GObject *obj);
static MccValue *mcc_src_battery_get(MccDataSource *datasrc);

static gpointer mcc_src_battery_parent_class = NULL;

GType mcc_src_battery_get_type(void)
{
    static GType type = 0;
    if (type == 0) {
	static GTypeInfo type_info = {
	    .class_size = sizeof(MccSrcBatteryClass),
	    
	    .base_init = NULL,
	    .base_finalize = NULL,
	    
	    .class_init = mcc_src_battery_class_init,
	    .class_finalize = NULL,
	    .class_data = NULL,
	    
	    .instance_size = sizeof(MccSrcBattery),
	    .n_preallocs = 0,
	    .instance_init = mcc_src_battery_init,
	    
	    .value_table = NULL,
	};
	
	type = g_type_register_static(MCC_TYPE_DATA_SOURCE, "MccSrcBattery", &type_info, 0);
    }
    
    return type;
}

static void mcc_src_battery_class_init(gpointer klass, gpointer class_data)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    MccDataSourceClass *datasrc_class = MCC_DATA_SOURCE_CLASS(klass);
    MccSrcBatteryClass *src_class = MCC_SRC_BATTERY_CLASS(klass);
    
    mcc_src_battery_parent_class = g_type_class_peek_parent(klass);
    
    gobject_class->finalize = mcc_src_battery_finalize;
    
    datasrc_class->label = g_strdup("Battery");
    datasrc_class->tick_per_read = 20;
    datasrc_class->set_subidx = mcc_src_battery_set_subidx;
    datasrc_class->read = mcc_src_battery_read;
    datasrc_class->get = mcc_src_battery_get;
    
    src_class->nbatt = battery_read_config(
	    &src_class->acdirfd, &src_class->last_full_capacities, &src_class->dirfds);
    
    datasrc_class->sublabels = g_new0(gchar *, src_class->nbatt + 1);
    for (gint i = 0; i < src_class->nbatt; i++)
	datasrc_class->sublabels[i] = g_strdup_printf("BAT %d", i);
    
    src_class->olddata = g_new0(data_per_batt, src_class->nbatt);
    src_class->newdata = g_new0(data_per_batt, src_class->nbatt);
    
    src_class->battalert = g_new0(struct battalert_t *, src_class->nbatt);
    for (gint i = 0; i < src_class->nbatt; i++)
	src_class->battalert[i] = battalert_new();
    
    battery_read_data(src_class->newdata, src_class->acdirfd,
	    src_class->nbatt, src_class->last_full_capacities, src_class->dirfds);
    memcpy(src_class->olddata, src_class->newdata, sizeof *src_class->olddata * src_class->nbatt);
}

static void mcc_src_battery_read(MccDataSourceClass *datasrc_class)
{
    MccSrcBatteryClass *src_class = MCC_SRC_BATTERY_CLASS(datasrc_class);
    
    memcpy(src_class->olddata, src_class->newdata, sizeof *src_class->olddata * src_class->nbatt);
    battery_read_data(src_class->newdata, src_class->acdirfd,
	    src_class->nbatt, src_class->last_full_capacities, src_class->dirfds);
    
    for (gint i = 0; i < src_class->nbatt; i++) {
	gint ratio = src_class->newdata[i].ratio * 100;
	if (src_class->newdata[i].ac)
	    battalert_clear(src_class->battalert[i]);
	else
	    battalert_alert(src_class->battalert[i], ratio);
    }
}

static gint battery_read_config(gint *acdirfd, gint32 **fulls, gint **dirfds)
{
    *acdirfd = open_dir("/proc/acpi/ac_adapter/AC/");
    
    gint num = 0;
    
    *fulls = g_new0(gint32, 1);
    *dirfds = g_new0(gint, 1);
    
    while (TRUE) {
	FILE *fp;
	char buf[1024];
	
	sprintf(buf, "/proc/acpi/battery/BAT%d/", num);
	gint dirfd;
	if ((dirfd = open_dir(buf)) == -1)
	    break;
	
	gint32 full = 0;
	if ((fp = fopenat(dirfd, "info")) == NULL)
	    break;
	while (fgets(buf, sizeof buf, fp) != NULL) {
	    gint32 n;
	    if (sscanf(buf, "last full capacity: %" G_GINT32_FORMAT " mWh", &n) == 1)
		full = n;
	}
	fclose(fp);
	
	if (full == 0)
	    break;
	
	*fulls = g_renew(gint32, *fulls, num + 1);
	*dirfds = g_renew(gint32, *dirfds, num + 1);
	(*fulls)[num] = full;
	(*dirfds)[num] = dirfd;
	num++;
    }
    
    return num;
}

static void battery_read_data(data_per_batt *ptr, gint acdirfd,
	gint nbatt, gint32 *last_full_capacities, gint *dirfds)
{
    char buf[1024];
    FILE *fp;
    
    gboolean ac = FALSE;
    if ((fp = fopenat(acdirfd, "state")) != NULL) {
	while (fgets(buf, sizeof buf, fp) != NULL) {
	    if (strncmp(buf, "state:", 6) == 0) {
		if (strstr(buf, "on-line") != NULL)
		    ac = TRUE;
	    }
	}
	
	fclose(fp);
    }
    
    for (gint batt = 0; batt < nbatt; batt++) {
	gint32 cur = 0;
	
	ptr[batt].ac = ac;
	ptr[batt].charging = FALSE;
	ptr[batt].ratio = 0.0;
	
	if ((fp = fopenat(dirfds[batt], "state")) != NULL) {
	    while (fgets(buf, sizeof buf, fp) != NULL) {
		gint32 n;
		char bf[32];
		if (sscanf(buf, "remaining capacity: %" G_GINT32_FORMAT " mWh", &n) == 1)
		    cur = n;
		else if (sscanf(buf, "charging state: %s", bf) == 1 && strcmp(bf, "charging") == 0) {
		    ptr[batt].charging = TRUE;
		}
	    }
	    
	    fclose(fp);
	}
	
	if (last_full_capacities[batt] > 0)
	    ptr[batt].ratio = (gdouble) cur / last_full_capacities[batt];
    }
}

static void mcc_src_battery_init(GTypeInstance *obj, gpointer klass)
{
}

static void mcc_src_battery_finalize(GObject *object)
{
    (*G_OBJECT_CLASS(mcc_src_battery_parent_class)->finalize)(object);
}

static void mcc_src_battery_set_subidx(MccDataSource *datasrc)
{
    datasrc->min = 0.0;
    datasrc->max = 1.0;
    datasrc->nvalues = 1;
    
    datasrc->nfg = 2;
    datasrc->fg_labels = g_new0(gchar *, 2);
    datasrc->fg_labels[0] = g_strdup("Not Charging");
    datasrc->fg_labels[1] = g_strdup("Charging");
    datasrc->default_fg = g_new0(GdkColor, 2);
    datasrc->default_fg[0].red = 0x0000;
    datasrc->default_fg[0].green = 0x0000;
    datasrc->default_fg[0].blue = 0xffff;
    datasrc->default_fg[1].red = 0x0000;
    datasrc->default_fg[1].green = 0xffff;
    datasrc->default_fg[1].blue = 0xffff;
    
    datasrc->nbg = 2;
    datasrc->bg_labels = g_new0(gchar *, 2);
    datasrc->bg_labels[0] = g_strdup("AC Off");
    datasrc->bg_labels[1] = g_strdup("AC On");
    datasrc->default_bg = g_new0(GdkColor, 2);
    datasrc->default_bg[0].red = 0x0000;
    datasrc->default_bg[0].green = 0x0000;
    datasrc->default_bg[0].blue = 0x0000;
    datasrc->default_bg[1].red = 0x0000;
    datasrc->default_bg[1].green = 0x4040;
    datasrc->default_bg[1].blue = 0x4040;
    
    datasrc->add_on_tick = FALSE;
    
    datasrc->sublabel = g_strdup(MCC_DATA_SOURCE_GET_CLASS(datasrc)->sublabels[datasrc->subidx]);
}

static MccValue *mcc_src_battery_get(MccDataSource *datasrc)
{
    MccSrcBattery *src = MCC_SRC_BATTERY(datasrc);
    MccSrcBatteryClass *src_class = MCC_SRC_BATTERY_GET_CLASS(src);
    
    MccValue *value = mcc_value_new(1);
    mcc_value_set_value(value, 0, src_class->newdata[src->data_source.subidx].ratio);
    mcc_value_set_foreground(value, 0, src_class->newdata[src->data_source.subidx].charging ? 1 : 0);
    mcc_value_set_background(value, src_class->newdata[src->data_source.subidx].ac ? 1 : 0);
    
    return value;
}
