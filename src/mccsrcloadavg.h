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

#ifndef MCC_SRC_LOAD_AVG_H
#define MCC_SRC_LOAD_AVG_H

#include <glib-object.h>
#include "mccdatasource.h"

#define MCC_TYPE_SRC_LOAD_AVG                  (mcc_src_load_avg_get_type ())
#define MCC_SRC_LOAD_AVG(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), MCC_TYPE_SRC_LOAD_AVG, MccSrcLoadAvg))
#define MCC_SRC_LOAD_AVG_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), MCC_TYPE_SRC_LOAD_AVG, MccSrcLoadAvgClass))
#define MCC_IS_SRC_LOAD_AVG(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MCC_TYPE_SRC_LOAD_AVG))
#define MCC_IS_SRC_LOAD_AVG_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), MCC_TYPE_SRC_LOAD_AVG))
#define MCC_SRC_LOAD_AVG_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), MCC_TYPE_SRC_LOAD_AVG, MccSrcLoadAvgClass))

typedef struct _MccSrcLoadAvg       MccSrcLoadAvg;
typedef struct _MccSrcLoadAvgClass  MccSrcLoadAvgClass;

struct _MccSrcLoadAvgPrivate;

struct _MccSrcLoadAvg {
    MccDataSource data_source;
};

struct _MccSrcLoadAvgClass {
    MccDataSourceClass parent_class;
    
    gdouble *olddata, *newdata;	// 1min, 5min, 15min
};

GType mcc_src_load_avg_get_type(void) G_GNUC_CONST;

#endif
