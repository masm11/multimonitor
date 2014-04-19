#ifndef CPUFREQ_H__INCLUDED
#define CPUFREQ_H__INCLUDED

void cpufreq_init(void);
void cpufreq_read_data(gint type);
void cpufreq_draw_1(gint type, GdkPixmap *pix, GdkGC *bg, GdkGC *fg, GdkGC *err);

#endif	/* ifndef CPUFREQ_H__INCLUDED */
