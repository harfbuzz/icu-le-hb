
NULL =

include_HEADERS = \
	LayoutEngine.h \
	LEFontInstance.h \
	LEGlyphFilter.h \
	LEGlyphStorage.h \
	LEInsertionList.h \
	LELanguages.h \
	LEScripts.h \
	LESwaps.h \
	LETypes.h \
	loengine.h \
	$(NULL)

lib_LTLIBRARIES = libicule.la

libicule_la_LDFLAGS =
libicule_la_CPPFLAGS = \
	$(HARFBUZZ_CFLAGS) \
	$(ICU_CFLAGS) \
	$(NULL)
libicule_la_LIBADD = \
	$(HARFBUZZ_LIBS) \
	$(ICU_LIBS) \
	$(NULL)

libicule_la_SOURCES = \
	$(include_HEADERS) \
	LayoutEngine.cpp
	LEFontInstance.cpp
	LEGlyphStorage.cpp
	loengine.cpp
	$(NULL)

-include $(top_srcdir)/git.mk
