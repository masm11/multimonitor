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

#ifndef MCC_SRC_DISK_IO_H
#define MCC_SRC_DISK_IO_H

#include <glib-object.h>
#include "mccdatasource.h"

#define MCC_TYPE_SRC_DISK_IO                  (mcc_src_disk_io_get_type ())
#define MCC_SRC_DISK_IO(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), MCC_TYPE_SRC_DISK_IO, MccSrcDiskIO))
#define MCC_SRC_DISK_IO_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), MCC_TYPE_SRC_DISK_IO, MccSrcDiskIOClass))
#define MCC_IS_SRC_DISK_IO(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MCC_TYPE_SRC_DISK_IO))
#define MCC_IS_SRC_DISK_IO_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), MCC_TYPE_SRC_DISK_IO))
#define MCC_SRC_DISK_IO_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), MCC_TYPE_SRC_DISK_IO, MccSrcDiskIOClass))

typedef struct _MccSrcDiskIO       MccSrcDiskIO;
typedef struct _MccSrcDiskIOClass  MccSrcDiskIOClass;

struct _MccSrcDiskIOPrivate;

struct _MccSrcDiskIO {
    MccDataSource data_source;
};

struct _MccSrcDiskIOClass {
    MccDataSourceClass parent_class;
    
    gint64 *olddata, *newdata;
};

GType mcc_src_disk_io_get_type(void) G_GNUC_CONST;

#endif
