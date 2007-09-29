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

#ifndef MCC_SRC_MEMORY_H
#define MCC_SRC_MEMORY_H

#include <glib-object.h>
#include "mccdatasource.h"

#define MCC_TYPE_SRC_MEMORY                  (mcc_src_memory_get_type ())
#define MCC_SRC_MEMORY(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), MCC_TYPE_SRC_MEMORY, MccSrcMemory))
#define MCC_SRC_MEMORY_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), MCC_TYPE_SRC_MEMORY, MccSrcMemoryClass))
#define MCC_IS_SRC_MEMORY(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MCC_TYPE_SRC_MEMORY))
#define MCC_IS_SRC_MEMORY_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), MCC_TYPE_SRC_MEMORY))
#define MCC_SRC_MEMORY_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), MCC_TYPE_SRC_MEMORY, MccSrcMemoryClass))

typedef struct _MccSrcMemory       MccSrcMemory;
typedef struct _MccSrcMemoryClass  MccSrcMemoryClass;

struct _MccSrcMemoryPrivate;

struct _MccSrcMemory {
    MccDataSource data_source;
};

struct _MccSrcMemoryClass {
    MccDataSourceClass parent_class;
    
    gint64 max;
    gint64 *olddata, *newdata;
};

GType mcc_src_memory_get_type(void) G_GNUC_CONST;

#endif
