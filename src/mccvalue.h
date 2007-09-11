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

#ifndef MCC_VALUE_H
#define MCC_VALUE_H

#include <glib-object.h>

#define MCC_TYPE_VALUE                  (mcc_value_get_type ())
#define MCC_VALUE(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), MCC_TYPE_VALUE, MccValue))
#define MCC_VALUE_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), MCC_TYPE_VALUE, MccValueClass))
#define MCC_IS_VALUE(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MCC_TYPE_VALUE))
#define MCC_IS_VALUE_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), MCC_TYPE_VALUE))
#define MCC_VALUE_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), MCC_TYPE_VALUE, MccValueClass))

typedef struct _MccValue       MccValue;
typedef struct _MccValueClass  MccValueClass;

struct _MccValuePrivate;

struct _MccValue {
    GObject object;
    
    struct _MccValuePrivate *priv;
};

struct _MccValueClass {
    GObjectClass parent_class;
};

MccValue *mcc_value_new(void);
void mcc_value_set_value(MccValue *value, gint idx, gdouble val);
gdouble mcc_value_get_value(gint idx, MccValue *value);

#endif
