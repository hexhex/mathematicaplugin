#
# replace 'plugin' on the right side by your plugin's name
#
plugin_LTLIBRARIES = libdlvhexmathematica.la

#
# replace 'plugin' on the left side as above and
# add all sources of your plugin
#
libdlvhexmathematica_la_SOURCES = mathematicaplugin.cpp \
					mathlinksingleton.cpp

#
# extend compiler flags by CFLAGS of other needed libraries
#
AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	$(BOOST_CPPFLAGS) \
	$(DLVHEX_CFLAGS)

libdlvhexmathematica_la_LDFLAGS = -avoid-version -module


libdlvhexmathematica-static.la: $(libdlvhexmathematica_la_OBJECTS)
	$(CXXLINK) -avoid-version -module -rpath $(plugindir) $(libdlvhexmathematica_la_OBJECTS)

install-static: libdlvhexmathematica-static.la
	$(LIBTOOL) --mode=install $(INSTALL) -s libdlvhexmathematica-static.la $(DESTDIR)$(plugindir)/libdlvhexmathematica-static.la

uninstall-static:
	$(LIBTOOL) --mode=uninstall rm -f $(DESTDIR)$(plugindir)/libdlvhexmathematica-static.la
