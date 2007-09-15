#ifndef OPS_H__INCLUDED
#define OPS_H__INCLUDED

#include <glib.h>
#include "mccvalue.h"

struct ops_t {
    void (*sinit)(void);
    void (*sread)(void);
    void (*sfini)(void);
    
    void *(*new)(void);
    MccValue *(*get)(void *);
    gint (*nvalues)(void *);
    void (*destroy)(void *);
};

extern struct ops_t linux_cpuload_ops;

#endif	/* ifndef OPS_H__INCLUDED */
