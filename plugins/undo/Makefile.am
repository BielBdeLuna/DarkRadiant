AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/libs \
              $(XML_CFLAGS) $(LIBSIGC_CFLAGS)

modulesdir = $(pkglibdir)/modules
modules_LTLIBRARIES = undo.la

undo_la_LDFLAGS = -module -avoid-version $(WX_LIBS)
undo_la_SOURCES = UndoSystem.cpp

