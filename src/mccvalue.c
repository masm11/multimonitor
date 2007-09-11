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

#include "mccvalue.h"

typedef struct _MccValuePrivate {
    gdouble value;
    
} MccValuePrivate;

G_DEFINE_TYPE(MccValue, mcc_value, G_TYPE_OBJECT)

static void mcc_value_finalize(GObject *obj);;

static void mcc_value_class_init(MccValueClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    
    gobject_class->finalize = mcc_value_finalize;
}

static void mcc_value_init(MccValue *self)
{
    self->priv = g_new0(struct _MccValuePrivate, 1);
    
    self->priv->value = 0;
}

static void mcc_value_finalize(GObject *object)
{
    MccValue *value = MCC_VALUE(object);
    g_free(value->priv);
    value->priv = NULL;
    
    (*G_OBJECT_CLASS(mcc_value_parent_class)->finalize)(object);
}

void mcc_value_set_value(MccValue *value, gdouble val)
{
    value->priv->value = val;
}

gdouble mcc_value_get_value(MccValue *value)
{
    return value->priv->value;
}

MccValue *mcc_value_new(void)
{
    MccValue *value = g_object_new(MCC_TYPE_VALUE, NULL);
    
    return value;
}
