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

#ifndef MCC_SRC_CPU_LOAD_H
#define MCC_SRC_CPU_LOAD_H

#include <glib-object.h>
#include "mccdatasource.h"

#define MCC_TYPE_SRC_CPU_LOAD                  (mcc_src_cpu_load_get_type ())
#define MCC_SRC_CPU_LOAD(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), MCC_TYPE_SRC_CPU_LOAD, MccSrcCpuLoad))
#define MCC_SRC_CPU_LOAD_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), MCC_TYPE_SRC_CPU_LOAD, MccSrcCpuLoadClass))
#define MCC_IS_SRC_CPU_LOAD(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MCC_TYPE_SRC_CPU_LOAD))
#define MCC_IS_SRC_CPU_LOAD_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), MCC_TYPE_SRC_CPU_LOAD))
#define MCC_SRC_CPU_LOAD_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), MCC_TYPE_SRC_CPU_LOAD, MccSrcCpuLoadClass))

typedef struct _MccSrcCpuLoad       MccSrcCpuLoad;
typedef struct _MccSrcCpuLoadClass  MccSrcCpuLoadClass;

struct _MccSrcCpuLoadPrivate;

struct _MccSrcCpuLoad {
    MccDataSource data_source;
};

#define NR_DATA 8

typedef gint64 data_per_cpu[NR_DATA];

struct _MccSrcCpuLoadClass {
    MccDataSourceClass parent_class;
    
    gint ncpu;
    data_per_cpu *olddata, *newdata;
};

GType mcc_src_cpu_load_get_type(void) G_GNUC_CONST;

#endif
