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

#include "../config.h"
#include "opendirat.h"
#include "mccdatasource.h"

enum {
    PROP_0,
    PROP_SUBIDX,
};

G_DEFINE_ABSTRACT_TYPE(MccDataSource, mcc_data_source, G_TYPE_OBJECT)

static void mcc_data_source_set_property(
	GObject *object,
	guint prop_id, const GValue *value,
	GParamSpec *pspec);
static void mcc_data_source_get_property(
	GObject *object,
	guint prop_id, GValue *value,
	GParamSpec *pspec);
static void mcc_data_source_finalize(GObject *obj);

static void mcc_data_source_class_init(MccDataSourceClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    
    gobject_class->set_property = mcc_data_source_set_property;
    gobject_class->get_property = mcc_data_source_get_property;
    gobject_class->finalize = mcc_data_source_finalize;

    g_object_class_install_property(gobject_class,
	    PROP_SUBIDX,
	    g_param_spec_int("subidx",
		    "Sub Index",
		    "Sub index",
		    0, 1024, 0,
		    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
    
    /* class の継承では、メンバはそのままコピーされる。
     * したがって、これらは何度も open されることはない。
     */
    klass->proc_dirfd = open_dir("/proc");
    klass->sys_dirfd = open_dir("/sys");
}

static void mcc_data_source_set_property(
	GObject *object,
	guint prop_id, const GValue *value,
	GParamSpec *pspec)
{
    MccDataSource *datasrc = MCC_DATA_SOURCE(object);
    
    switch (prop_id) {
    case PROP_SUBIDX:
	datasrc->subidx = g_value_get_int(value);
	if (MCC_DATA_SOURCE_GET_CLASS(datasrc)->set_subidx != NULL)
	    (*MCC_DATA_SOURCE_GET_CLASS(datasrc)->set_subidx)(datasrc);
	break;
    default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	break;
    }
}

static void mcc_data_source_get_property(
	GObject *object,
	guint prop_id, GValue *value,
	GParamSpec *pspec)
{
    MccDataSource *datasrc = MCC_DATA_SOURCE(object);
    
    switch (prop_id) {
    case PROP_SUBIDX:
	g_value_set_int(value, datasrc->subidx);
	break;
    default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	break;
    }
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
	data_src_class->has_new_data = FALSE;
	if (++data_src_class->tick_for_read >= data_src_class->tick_per_read) {
	    data_src_class->tick_for_read = 0;
	    (*data_src_class->read)(data_src_class);
	    data_src_class->has_new_data = TRUE;
	}
    }
}

gboolean mcc_data_source_has_new_data(MccDataSource *datasrc)
{
    MccDataSourceClass *datasrc_class = MCC_DATA_SOURCE_GET_CLASS(datasrc);
    return datasrc_class->has_new_data;
}

MccValue *mcc_data_source_get(MccDataSource *datasrc)
{
    MccDataSourceClass *datasrc_class = MCC_DATA_SOURCE_GET_CLASS(datasrc);
    return (*datasrc_class->get)(datasrc);
}

MccDataSource *mcc_data_source_new(GType type, gint subidx)
{
    MccDataSource *src = g_object_new(type,
	    "subidx", subidx,
	    NULL);
    return src;
}
