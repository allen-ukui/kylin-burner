module_flags = -avoid-version -module

AM_CPPFLAGS =					\
	-I$(top_srcdir)/libburner-utils/	\
	-I$(top_builddir)/libburner-utils/	\
	-I$(top_srcdir)/libburner-media/	\
	-I$(top_builddir)/libburner-media/	\
	-I$(top_srcdir)/libburner-burn/	\
	-I$(top_builddir)/libburner-burn/	\
	-I$(top_srcdir)/src/			\
	-I$(top_builddir)/src/			\
	$(WARN_CFLAGS)							\
	$(WERROR)				\
	-D_FILE_OFFSET_BITS=64			\
	-D_GNU_SOURCE				\
	-D_LARGEFILE64_SOURCE			\
	-D_REENTRANT				\
	-D_POSIX_PTHREAD_SEMANTICS		\
	-DPREFIX=\"$(prefix)\"			\
	-DBINDIR=\""$(bindir)"\"		\
	-DLIBEXECDIR=\""$(libexecdir)"\"	\
	-DSHAREDIR=\""$(datadir)"\"		\
	-DDATADIR=\""$(datadir)/burner"\"	\
	$(NAUTILUS_EXTENSION_CFLAGS)		\
	$(DISABLE_DEPRECATED)			\
	-Wno-error=format-y2k

@INTLTOOL_DESKTOP_RULE@

desktopdir=$(datadir)/applications
desktop_in_files = burner-nautilus.desktop.in

desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

EXTRA_DIST = $(desktop_in_files)
CLEANFILES = $(desktop_DATA)
DISTCLEANFILES = $(NULL)

### Nautilus Extension
nautilus_extensiondir = $(NAUTILUSDIR)
nautilus_extension_LTLIBRARIES = libnautilus-burner-extension.la

libnautilus_burner_extension_la_SOURCES = 	\
	nautilus-burn-bar.c			\
	nautilus-burn-bar.h			\
	nautilus-burn-extension.c

libnautilus_burner_extension_la_LIBADD =	\
	$(NAUTILUS_EXTENSION_LIBS)		\
	../libburner-utils/libburner-utils3.la	\
	../libburner-media/libburner-media3.la	\
	../libburner-burn/libburner-burn3.la
	
libnautilus_burner_extension_la_LDFLAGS = $(module_flags)

-include $(top_srcdir)/git.mk
