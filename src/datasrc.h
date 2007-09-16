#ifndef DATASRC_H__INCLUDED
#define DATASRC_H__INCLUDED

#include <glib.h>
#include <gdk/gdk.h>
#include "mccvalue.h"

struct datasrc_context_t;
static inline struct DATASRC_CONTEXT_T *datasrc_context_ptr(struct datasrc_context_t *p) {
    return (struct DATASRC_CONTEXT_T *) p;
}
static inline struct datasrc_context_t *datasrc_context_base_ptr(struct DATASRC_CONTEXT_T *p) {
    return (struct datasrc_context_t *) p;
}

struct datasrc_info_t {
    gdouble min, max;
    gint nvalues;
    
    const gchar **value_labels;	// [nvalues]
    const GdkColor *default_fg;	// [nvalues]
    gint nbg;
    const GdkColor *default_bg;	// [nbg]
};

struct datasrc_t {
    void (*sinit)(void);
    void (*sread)(void);
    void (*sfini)(void);
    
    struct datasrc_context_t *(*new)(void);
    MccValue *(*get)(struct datasrc_context_t *);
    const struct datasrc_info_t *(*info)(struct datasrc_context_t *);
    void (*destroy)(struct datasrc_context_t *);
};

extern struct datasrc_t linux_cpuload_datasrc;
extern struct datasrc_t linux_cpufreq_datasrc;

#endif	/* ifndef DATASRC_H__INCLUDED */
