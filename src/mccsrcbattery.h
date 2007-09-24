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

#ifndef MCC_SRC_BATTERY_H
#define MCC_SRC_BATTERY_H

#include <glib-object.h>
#include "mccdatasource.h"

#define MCC_TYPE_SRC_BATTERY                  (mcc_src_battery_get_type ())
#define MCC_SRC_BATTERY(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), MCC_TYPE_SRC_BATTERY, MccSrcBattery))
#define MCC_SRC_BATTERY_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), MCC_TYPE_SRC_BATTERY, MccSrcBatteryClass))
#define MCC_IS_SRC_BATTERY(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MCC_TYPE_SRC_BATTERY))
#define MCC_IS_SRC_BATTERY_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), MCC_TYPE_SRC_BATTERY))
#define MCC_SRC_BATTERY_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), MCC_TYPE_SRC_BATTERY, MccSrcBatteryClass))

typedef struct _MccSrcBattery       MccSrcBattery;
typedef struct _MccSrcBatteryClass  MccSrcBatteryClass;

struct _MccSrcBatteryPrivate;

struct _MccSrcBattery {
    MccDataSource data_source;
};

typedef struct {
    gboolean ac;
    gboolean charging;
    gdouble ratio;
} data_per_batt;

struct _MccSrcBatteryClass {
    MccDataSourceClass parent_class;
    
    gint nbatt;
    data_per_batt *olddata, *newdata;
    gint32 last_full_capacity;
};

GType mcc_src_battery_get_type(void) G_GNUC_CONST;

#endif
