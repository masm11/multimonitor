#ifndef LOADAVG_H__INCLUDED
#define LOADAVG_H__INCLUDED

void loadavg_init(void);
void loadavg_read_data(gint type);
void loadavg_draw_1(gint type, GdkPixmap *pix, GdkGC *bg, GdkGC *fg, GdkGC *err);

#endif	/* ifndef LOADAVG_H__INCLUDED */
