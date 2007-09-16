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

static void mcc_value_finalize(GObject *obj);;

static void mcc_value_class_init(MccValueClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    
    gobject_class->finalize = mcc_value_finalize;
}

static void mcc_value_init(MccValue *self)
{
    self->priv = g_new0(struct _MccValuePrivate, 1);
    
    self->priv->values = NULL;
    self->priv->fgs = NULL;
}

static void mcc_value_finalize(GObject *object)
{
    MccValue *value = MCC_VALUE(object);
    
    g_free(value->priv->values);
    g_free(value->priv->fgs);
    
    g_free(value->priv);
    value->priv = NULL;
    
    (*G_OBJECT_CLASS(mcc_value_parent_class)->finalize)(object);
}

void mcc_value_set_value(MccValue *value, gint idx, gdouble val)
{
    g_assert((guint) idx < value->priv->nvalues);
    value->priv->values[idx] = val;
}

gdouble mcc_value_get_value(MccValue *value, gint idx)
{
    g_assert((guint) idx < value->priv->nvalues);
    return value->priv->values[idx];
}

void mcc_value_set_foreground(MccValue *value, gint idx, gint col)
{
    g_assert((guint) idx < value->priv->nvalues);
    value->priv->fgs[idx] = col;
}

gint mcc_value_get_foreground(MccValue *value, gint idx)
{
    g_assert((guint) idx < value->priv->nvalues);
    return value->priv->fgs[idx];
}

void mcc_value_set_background(MccValue *value, gint col)
{
    value->priv->bg = col;
}

gint mcc_value_get_background(MccValue *value)
{
    return value->priv->bg;
}

// fixme: メモリ管理
MccValue *mcc_value_new(gint nvalues)
{
    MccValue *value = g_object_new(MCC_TYPE_VALUE, NULL);
    
    value->priv->nvalues = nvalues;
    value->priv->values = g_new0(gdouble, nvalues);
    value->priv->fgs = g_new0(gint, nvalues);
    
    return value;
}
