
AM_CPPFLAGS = \
	-I$(top_srcdir)					\
	-I$(top_srcdir)/libburner-media/					\
	-I$(top_builddir)/libburner-media/		\
	-I$(top_srcdir)/libburner-burn				\
	-I$(top_builddir)/libburner-burn/				\
	-DBURNER_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" 	\
	-DBURNER_PREFIX=\"$(prefix)\"           		\
	-DBURNER_SYSCONFDIR=\"$(sysconfdir)\"   		\
	-DBURNER_DATADIR=\"$(datadir)/burner\"     	    	\
	-DBURNER_LIBDIR=\"$(libdir)\"  	         	\
	$(WARN_CFLAGS)							\
	$(DISABLE_DEPRECATED)				\
	$(BURNER_GLIB_CFLAGS)				\
	$(BURNER_LIBXML_CFLAGS)

dvdauthordir = $(BURNER_PLUGIN_DIRECTORY)
dvdauthor_LTLIBRARIES = libburner-dvdauthor.la

libburner_dvdauthor_la_SOURCES = burn-dvdauthor.c 
libburner_dvdauthor_la_LIBADD = ../../libburner-burn/libburner-burn3.la $(BURNER_GLIB_LIBS) $(BURNER_LIBXML_LIBS)
libburner_dvdauthor_la_LDFLAGS = -module -avoid-version


-include $(top_srcdir)/git.mk
