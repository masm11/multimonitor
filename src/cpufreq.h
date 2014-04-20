#ifndef CPUFREQ_H__INCLUDED
#define CPUFREQ_H__INCLUDED

void cpufreq_init(void);
void cpufreq_read_data(gint type);
void cpufreq_draw_1(gint type, GdkPixbuf *pix, GdkColor *bg, GdkColor *fg, GdkColor *err);
void cpufreq_discard_data(gint type, gint size);

#endif	/* ifndef CPUFREQ_H__INCLUDED */
