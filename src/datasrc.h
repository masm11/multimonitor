#ifndef DATASRC_H__INCLUDED
#define DATASRC_H__INCLUDED

#include <glib.h>
#include "mccvalue.h"

struct datasrc_info_t {
    gdouble min, max;
    gint nvalues;
};

struct datasrc_t {
    void (*sinit)(void);
    void (*sread)(void);
    void (*sfini)(void);
    
    void *(*new)(void);
    MccValue *(*get)(void *);
    const struct datasrc_info_t *(*info)(void *);
    void (*destroy)(void *);
};

extern struct datasrc_t linux_cpuload_datasrc;
extern struct datasrc_t linux_cpufreq_datasrc;

#endif	/* ifndef OPS_H__INCLUDED */
