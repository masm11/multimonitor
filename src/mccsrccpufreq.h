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

#ifndef MCC_SRC_CPU_FREQ_H
#define MCC_SRC_CPU_FREQ_H

#include <glib-object.h>
#include "mccdatasource.h"

#define MCC_TYPE_SRC_CPU_FREQ                  (mcc_src_cpu_freq_get_type ())
#define MCC_SRC_CPU_FREQ(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), MCC_TYPE_SRC_CPU_FREQ, MccSrcCpuFreq))
#define MCC_SRC_CPU_FREQ_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), MCC_TYPE_SRC_CPU_FREQ, MccSrcCpuFreqClass))
#define MCC_IS_SRC_CPU_FREQ(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MCC_TYPE_SRC_CPU_FREQ))
#define MCC_IS_SRC_CPU_FREQ_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), MCC_TYPE_SRC_CPU_FREQ))
#define MCC_SRC_CPU_FREQ_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), MCC_TYPE_SRC_CPU_FREQ, MccSrcCpuFreqClass))

typedef struct _MccSrcCpuFreq       MccSrcCpuFreq;
typedef struct _MccSrcCpuFreqClass  MccSrcCpuFreqClass;

struct _MccSrcCpuFreqPrivate;

struct _MccSrcCpuFreq {
    MccDataSource data_source;
};

struct _MccSrcCpuFreqClass {
    MccDataSourceClass parent_class;
    
    gint ncpu;
    gint64 maxfreq;
    gint64 *olddata, *newdata;
};

GType mcc_src_cpu_freq_get_type(void) G_GNUC_CONST;

#endif
