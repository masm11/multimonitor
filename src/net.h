#ifndef NET_H__INCLUDED
#define NET_H__INCLUDED

void net_init(void);
void net_read_data(gint type);
void net_draw_1(gint type, GdkPixbuf *pix, GdkColor *bg, GdkColor *fg, GdkColor *err);
void net_discard_data(gint type, gint size);

#endif	/* ifndef NET_H__INCLUDED */
