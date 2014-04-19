#ifndef CPULOAD_H__INCLUDED
#define CPULOAD_H__INCLUDED

void cpuload_init(void);
void cpuload_read_data(gint type);
void cpuload_draw_1(gint type, GdkPixmap *pix, GdkGC *bg, GdkGC *fg, GdkGC *err);

#endif	/* ifndef CPULOAD_H__INCLUDED */
