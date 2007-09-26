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
    gint nvalues;
    gdouble *values;
    gint *fgs;
    gint bg;
} MccValuePrivate;

G_DEFINE_TYPE(MccValue, mcc_value, G_TYPE_OBJECT)

static void mcc_value_finalize(GObject *obj);

static inline MccValuePrivate *mcc_value_get_private(MccValue *value)
{
    return G_TYPE_INSTANCE_GET_PRIVATE(value, MCC_TYPE_VALUE, MccValuePrivate);
}

static void mcc_value_class_init(MccValueClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    
    gobject_class->finalize = mcc_value_finalize;
    
    g_type_class_add_private(klass, sizeof (MccValuePrivate));
}

static void mcc_value_init(MccValue *self)
{
    MccValuePrivate *priv = mcc_value_get_private(self);
    
    priv->values = NULL;
    priv->fgs = NULL;
}

static void mcc_value_finalize(GObject *object)
{
    MccValue *value = MCC_VALUE(object);
    MccValuePrivate *priv = mcc_value_get_private(value);
    
    g_free(priv->values);
    g_free(priv->fgs);
    
    (*G_OBJECT_CLASS(mcc_value_parent_class)->finalize)(object);
}

void mcc_value_set_value(MccValue *value, gint idx, gdouble val)
{
    MccValuePrivate *priv = mcc_value_get_private(value);
    g_assert((guint) idx < priv->nvalues);
    priv->values[idx] = val;
}

gdouble mcc_value_get_value(MccValue *value, gint idx)
{
    MccValuePrivate *priv = mcc_value_get_private(value);
    g_assert((guint) idx < priv->nvalues);
    return priv->values[idx];
}

void mcc_value_set_foreground(MccValue *value, gint idx, gint col)
{
    MccValuePrivate *priv = mcc_value_get_private(value);
    g_assert((guint) idx < priv->nvalues);
    priv->fgs[idx] = col;
}

gint mcc_value_get_foreground(MccValue *value, gint idx)
{
    MccValuePrivate *priv = mcc_value_get_private(value);
    g_assert((guint) idx < priv->nvalues);
    return priv->fgs[idx];
}

void mcc_value_set_background(MccValue *value, gint col)
{
    MccValuePrivate *priv = mcc_value_get_private(value);
    priv->bg = col;
}

gint mcc_value_get_background(MccValue *value)
{
    MccValuePrivate *priv = mcc_value_get_private(value);
    return priv->bg;
}

// fixme: メモリ管理
MccValue *mcc_value_new(gint nvalues)
{
    MccValue *value = g_object_new(MCC_TYPE_VALUE, NULL);
    
    MccValuePrivate *priv = mcc_value_get_private(value);
    priv->nvalues = nvalues;
    priv->values = g_new0(gdouble, nvalues);
    priv->fgs = g_new0(gint, nvalues);
    
    return value;
}
