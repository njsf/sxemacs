## SXEmacs - Ent Makefile.am

## Copyright (C) 2010 Steve Youngs

## This file is part of SXEmacs.

## SXEmacs is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.

## SXEmacs is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.

## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <http://www.gnu.org/licenses/>.

## Process this file with automake to produce Makefile.in

# Help the Developers and yourself. Just use the C locale and settings
# for the compilation. They can still be overriden by make LANG=<whatever>
# but that is general a not very good idea
LANG=C
LC_ALL=C

builddir = @builddir@
srcdir = @srcdir@
top_builddir = @top_builddir@
top_build_prefix = $(top_builddir)/
top_srcdir = @top_srcdir@
abs_builddir = @abs_builddir@
abs_top_builddir = @abs_top_builddir@
abs_top_srcdir = @abs_top_srcdir@
sxelibsrcdir = $(abs_top_srcdir)/lib-src

AM_CFLAGS = -Demacs $(OPENSSL_CFLAGS) -DUSE_SXEMACS_CONFIG_H -DXTSTRINGDEFINES
AM_CPPFLAGS = -I$(srcdir) -I$(builddir) -I$(top_srcdir)/src -I$(top_builddir)/src -I$(top_builddir) $(c_switch_general) $(LTDLINCL)

headerdir = $(includedir)/$(instvarsepdir)
archlibdir = ${libdir}/${instvardir}/${configuration}


ETAGS = $(top_builddir)/lib-src/etags
TAGS_DEPENDENCIES = $(ETAGS)

SUBDIRS=
all_sources=
noinst_LIBRARIES=
header_HEADERS=$(acgen_headers)

BUILT_SOURCES = Emacs.ad.h
EXTRA_DIST = Emacs.ad.h

libsxeui_a_headers =				   \
	Emacs.ad.h  bitmaps.h			   \
	conslots.h console-stream.h		   \
	console.h device.h faces.h frame.h	   \
	frameslots.h getpagesize.h gifrlib.h	   \
	glyphs.h gui.h gutter.h	   \
	imgproc.h insdel.h keymap.h menubar.h	   \
	objects.h redisplay.h scrollbar.h select.h \
	toolbar.h universe.h window.h winslots.h
libsxeui_a_sources =							\
	console.c console-stream.c device.c faces.c font-lock.c         \
	frame.c glyphs.c glyphs-eimage.c glyphs-widget.c gui.c gutter.c \
	imgproc.c keymap.c insdel.c minibuf.c redisplay.c               \
	redisplay-output.c select.c specifier.c widget.c window.c       \
	objects.c 
libsxeui_a_SOURCES = $(libsxeui_a_headers) $(libsxeui_a_sources)
libsxeui_a_CPPFLAGS = $(AM_CPPFLAGS) $(c_switch_general) $(X_CFLAGS)
EXTRA_libsxeui_a_SOURCES=scrollbar.c menubar.c dialog.c toolbar.c
libsxeui_a_LIBADD = $(libsxeui_objs)
libsxeui_a_DEPENDENCIES = $(libsxeui_objs)
all_sources += $(libsxeui_a_sources) $(libsxeui_objs:.o=.c)

header_HEADERS += $(libsxeui_a_headers)
noinst_LIBRARIES += libsxeui.a

if NEED_LIBSXEUITTY
SUBDIRS+=TTY
endif

if DESCEND_LWLIB
SUBDIRS+=lwlib
endif

if NEED_LIBSXEUIX11
SUBDIRS+=X11
endif

Emacs.ad.h: $(top_srcdir)/etc/Emacs.ad
	@echo "Producing \`src/Emacs.ad.h' from \`etc/Emacs.ad'."
	@(echo "/*	Do not edit this file!" ; \
	  echo "	Automatically generated from $(top_srcdir)/etc/Emacs.ad" ; \
	  echo " */" ; \
	  $(SHELL) $(sxelibsrcdir)/ad2c $(top_srcdir)/etc/Emacs.ad ) > $@

#
# Help the SXEmacs developers get nice post-processed source files

## Create preprocessor output (debugging purposes only)
.c.i:
	$(COMPILE) -E -o $@ $<

## Create assembler output (debugging purposes only)
.c.s:
	$(COMPILE) -S -c $(cflags) $<
