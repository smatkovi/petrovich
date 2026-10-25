/* share.h - sending files over Bluetooth or by e-mail (see share.c) */
#ifndef __SHARE_H_
#define __SHARE_H_

#include <gtk/gtk.h>
#include <libosso.h>

void share_file(osso_context_t *ctx, GtkWindow *parent, const gchar *uri);
void share_files(osso_context_t *ctx, GtkWindow *parent, GSList *uris);

#endif /* __SHARE_H_ */
