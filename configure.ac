AC_PREREQ[(2.58)]
AC_INIT(m4_esyscmd([dpkg-parsechangelog | sed -n 's/Source: //p' | tr -d '\n']), m4_esyscmd([dpkg-parsechangelog | sed -n 's/Version: //p' | tr -d '\n']), m4_esyscmd([dpkg-parsechangelog | sed -n 's/Maintainer: //p' | cut -d\< -f2 | sed s/\>// | tr -d '\n']))
AC_CONFIG_SRCDIR([src/petrovich.c])
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS([config.h])

AC_PROG_CC
AM_PROG_CC_C_O
AC_HEADER_STDC
AM_PATH_GLIB_2_0([2.16.0])
AM_PATH_GTK_2_0([2.14.7])
AC_PROG_INSTALL

PKG_CHECK_MODULES([PETROVICH], [glib-2.0 gio-2.0 libosso libhildonmime gtk+-2.0 hildon-1 hildon-fm-2 libmodest-dbus-client-1.0])
AC_SUBST([PETROVICH_CFLAGS])
AC_SUBST([PETROVICH_LIBS])

SHARING_ICON="general_share"
AC_DEFINE_UNQUOTED(SHARING_ICON, "$SHARING_ICON", [Name of Sharing icon])
AC_SUBST([SHARING_ICON])

OSSO_SERVICE="com.qwerty12.$PACKAGE"
AC_DEFINE_UNQUOTED(OSSO_SERVICE, "$OSSO_SERVICE", [Name of D-Bus service])
AC_SUBST([OSSO_SERVICE])

DESKTOP_DIR=`$PKG_CONFIG osso-af-settings --variable=desktopentrydir`
AC_SUBST([DESKTOP_DIR])

DBUS_SERVICE_DIR=`$PKG_CONFIG osso-af-settings --variable=dbusservicedir`
AC_SUBST([DBUS_SERVICE_DIR])

AC_OUTPUT([Makefile data/petrovich.desktop data/petrovich.service data/Makefile src/Makefile])
