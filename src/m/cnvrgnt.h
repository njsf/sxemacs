/* machine description file for convergent S series.
   Copyright (C) 1989 Free Software Foundation, Inc.

This file is part of SXEmacs

SXEmacs is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

SXEmacs is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>. */


/* Synched up with: FSF 19.31. */

/* Now define a symbol for the cpu type, if your compiler
   does not define it automatically.  */

#define m68000

/* Data type of load average, as read out of kmem.
   These are commented out since it is not supported by this machine.  */

#define LOAD_AVE_TYPE double

/* Convert that into an integer that is 100 for a load average of 1.0 */

#define LOAD_AVE_CVT(x) ((int) ((x) * 100.0))

/* Define CANNOT_DUMP on machines where unexec does not work.
   Then the function dump-emacs will not be defined
   and temacs will do (load "loadup") automatically unless told otherwise.  */

/* #define CANNOT_DUMP */

/* Define C_ALLOCA if this machine does not support a true alloca
   and the one written in C should be used instead.
   Define HAVE_ALLOCA to say that the system provides a properly
   working alloca function and it should be used.
   Define neither one if an assembler-language alloca
   in the file alloca.s should be used.  */

#define C_ALLOCA
#undef HAVE_ALLOCA

/* Define NO_REMAP if memory segmentation makes it not work well
   to change the boundary between the text section and data section
   when Emacs is dumped.  If you define this, the preloaded Lisp
   code will not be sharable; but that's better than failing completely.  */

#define NO_REMAP

/* Change some things to avoid bugs in compiler.  */

#define SWITCH_ENUM_BUG

/* grows towards lower addresses.  */

#define	STACK_DIRECTION	-1

/* some errno.h's don't actually allocate the variable itself.
   Cause crt0.c to define errno.  */

#define NEED_ERRNO
