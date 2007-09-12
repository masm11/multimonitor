#include <stdio.h>
#include <string.h>
#include <glib.h>

#define NR_CPU 2
#define NR_DATA 7

static gint64 olddata[NR_CPU + 1][NR_DATA + 1];

static gboolean cpuload_read(gint64 (*ptr)[NR_DATA + 1]);

void cpuload_init(gint *ncpu, gint *ndata)
{
    *ncpu = NR_CPU;
    *ndata = NR_DATA;
    
    cpuload_read(olddata);
}

static gboolean cpuload_read(gint64 (*ptr)[NR_DATA + 1])
{
    FILE *fp;
    
    if ((fp = fopen("/proc/stat", "rt")) == NULL)
	return FALSE;
    
    int i;
    for (i = 0; i < NR_CPU + 1; i++) {
	char buf[1024];
	if (fgets(buf, sizeof buf, fp) == NULL)
	    break;
	if (sscanf(buf, "%*s %lld %lld %lld %lld %lld %lld %lld %lld",
			&ptr[i][0], &ptr[i][1], &ptr[i][2], &ptr[i][3],
			&ptr[i][4], &ptr[i][5], &ptr[i][6], &ptr[i][7]) != 8)
	    break;
    }
    
    fclose(fp);
    
    return TRUE;
}

gdouble **cpuload_get(void)
{
    int i, j;
    
    gdouble **rv = g_new0(gdouble *, NR_CPU + 1);
    for (i = 0; i < NR_CPU + 1; i++) {
	rv[i] = g_new0(gdouble, NR_DATA);
	for (j = 0; j < NR_DATA; j++)
	    rv[i][j] = 0;
    }
    
    gint64 data[NR_CPU + 1][NR_DATA + 1];
    
    if (cpuload_read(data)) {
	for (i = 0; i < NR_CPU + 1; i++) {
	    gint64 total
		    = data[i][0] - olddata[i][0]
		    + data[i][1] - olddata[i][1]
		    + data[i][2] - olddata[i][2]
		    + data[i][3] - olddata[i][3]
		    + data[i][4] - olddata[i][4]
		    + data[i][5] - olddata[i][5]
		    + data[i][6] - olddata[i][6]
		    + data[i][7] - olddata[i][7];
	    rv[i][0] = (data[i][0] - olddata[i][0]) / (gdouble) total;	// user
	    rv[i][1] = (data[i][1] - olddata[i][1]) / (gdouble) total;	// nice
	    rv[i][2] = (data[i][2] - olddata[i][2]) / (gdouble) total;	// sys
	    rv[i][3] = (data[i][4] - olddata[i][4]) / (gdouble) total;	// iowait
	    rv[i][4] = (data[i][5] - olddata[i][5]) / (gdouble) total;	// irq
	    rv[i][5] = (data[i][6] - olddata[i][6]) / (gdouble) total;	// softirq
	    rv[i][6] = (data[i][7] - olddata[i][7]) / (gdouble) total;	// steal
	}
	
	memcpy(&olddata, &data, sizeof olddata);
    }
    
    return rv;
}
