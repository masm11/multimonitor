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
    struct datasrc_t *src;
    
    const gchar *label;
    const gchar **sublabels;
};

struct datasrc_context_info_t {
    struct datasrc_t *src;
    gint sub_idx;
    
    gdouble min, max;
    gint nvalues;
    
    gint nfg;
    const gchar * const *fg_labels;	// [nfg]
    const GdkColor *default_fg;		// [nfg]
    
    gint nbg;
    const gchar * const *bg_labels;	// [nbg]
    const GdkColor *default_bg;		// [nbg]
    
    const gchar *sublabel;
};

struct datasrc_t {
    void (*sinit)(void);
    void (*sread)(void);
    void (*sfini)(void);
    const struct datasrc_info_t *(*sinfo)(void);
    
    struct datasrc_context_t *(*new)(gint subidx);
    MccValue *(*get)(struct datasrc_context_t *);
    const struct datasrc_context_info_t *(*info)(struct datasrc_context_t *);
    void (*destroy)(struct datasrc_context_t *);
};

extern struct datasrc_t linux_cpuload_datasrc;
extern struct datasrc_t linux_cpufreq_datasrc;
extern struct datasrc_t linux_battery_datasrc;

#endif	/* ifndef DATASRC_H__INCLUDED */
