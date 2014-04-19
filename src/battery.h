#ifndef BATTERY_H__INCLUDED
#define BATTERY_H__INCLUDED

void battery_init(void);
void battery_read_data(gint type);
void battery_draw_1(gint type, GdkPixmap *pix, GdkGC *bg, GdkGC *fg, GdkGC *err);
void battery_discard_data(gint type, gint size);

#endif	/* ifndef BATTERY_H__INCLUDED */
