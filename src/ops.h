#ifndef OPS_H__INCLUDED
#define OPS_H__INCLUDED

#include <glib.h>
#include "mccvalue.h"

struct info_t {
    gdouble min, max;
    gint nvalues;
};

struct ops_t {
    void (*sinit)(void);
    void (*sread)(void);
    void (*sfini)(void);
    
    void *(*new)(void);
    MccValue *(*get)(void *);
    const struct info_t *(*info)(void *);
    void (*destroy)(void *);
};

extern struct ops_t linux_cpuload_ops;
extern struct ops_t linux_cpufreq_ops;

#endif	/* ifndef OPS_H__INCLUDED */
