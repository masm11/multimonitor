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

#include "mccdatasource.h"

G_DEFINE_ABSTRACT_TYPE(MccDataSource, mcc_data_source, G_TYPE_OBJECT)

static void mcc_data_source_finalize(GObject *obj);

static void mcc_data_source_class_init(MccDataSourceClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    MccDataSourceClass *datasrc_class = MCC_DATA_SOURCE_CLASS(klass);
    
    gobject_class->finalize = mcc_data_source_finalize;
}

static void mcc_data_source_init(MccDataSource *self)
{
    self->max = 1.0;
    self->nvalues = 0;
}

static void mcc_data_source_finalize(GObject *object)
{
    MccDataSource *src = MCC_DATA_SOURCE(object);
    
#define FREE(p) do { if (p != NULL) { g_free(p); p = NULL; } } while (FALSE);
    
    for (int i = 0; i < src->nfg; i++)
	FREE(src->fg_labels[i]);
    FREE(src->fg_labels);
    FREE(src->default_fg);
    src->nfg = 0;
    
    for (int i = 0; i < src->nbg; i++)
	FREE(src->bg_labels[i]);
    FREE(src->bg_labels);
    FREE(src->default_bg);
    src->nbg = 0;
    
    FREE(src->sublabel);
    
#undef FREE
    
    (*G_OBJECT_CLASS(mcc_data_source_parent_class)->finalize)(object);
}

void mcc_data_source_read(GType type)
{
    GObjectClass *klass = g_type_class_peek(type);
    if (klass != NULL && MCC_IS_DATA_SOURCE_CLASS(klass)) {
	MccDataSourceClass *data_src_class = (MccDataSourceClass *) klass;
	(*data_src_class->read)(data_src_class);
    }
}

MccValue *mcc_data_source_get(MccDataSource *datasrc)
{
    MccDataSourceClass *datasrc_class = MCC_DATA_SOURCE_GET_CLASS(datasrc);
    return (*datasrc_class->get)(datasrc);
}
