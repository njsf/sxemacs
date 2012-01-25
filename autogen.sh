#!/bin/sh
# Configure script bootstrap for SXEmacs
#
# Copyright (C) 2005, 2006, 2007 Steve Youngs.
# Copyright (C) 2006, 2007, 2008 Sebastian Freundt.
# Copyright (C) 2007, 2010, 2011 Nelson Ferreira

# This file is part of SXEmacs.

# SXEmacs is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# SXEmacs is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Parts of SXEmacs are also distributed under a BSD-like licence.
# Check file headers for more information.

# BSD's m4 probably isn't gonna cut it, use gm4 if it is available
type gm4 >/dev/null 2>&1 && M4=gm4 || M4=m4

M4_VERSION=$($M4 --version | head -n1 | sed -e 's/^\(m4 \)\?(\?GNU M4)\? *//g' ) 
GOOD_M4=$( echo $M4_VERSION | awk -F. '{if( ($1>1) || ( ($1==1) && ($2>4) ) || ( ($1==1) && ($2==4) && ($3>=6) )) print 1 }')

if [ "$GOOD_M4" != "1" ]; then
    echo You have m4 version $M4_VERSION.  SXEmacs requires m4 version 1.4.6 or later.
    exit 1
fi

# To cater for Solaris
if test -d "/usr/xpg4/bin"; then
    PATH=/usr/xpg4/bin:$PATH
    export PATH
fi

type git >/dev/null 2>&1 && GIT=git
olddir=$(pwd)
srcdir=$(dirname $0)
cd "$srcdir"

EXPECTED_TREE_VERSION="22.1.14"

emacs_is_beta=t
if test -n "$GIT" -a -n "$($GIT symbolic-ref HEAD 2>/dev/null)"; then
	TREE_VERSION="$($GIT tag|tail -n1|tr -d v)"
	GIT_VERSION="$($GIT describe | head -n1)"
	IN_GIT="1"
fi
if test -z "$TREE_VERSION"; then
        TREE_VERSION="$EXPECTED_TREE_VERSION"
        if test -n "$IN_GIT"; then
	    echo "If you cloned this branch into your own you should issue:"
	    echo "\tgit tag -s v${TREE_VERSION}.<your branch_name>"
	    echo "\tgit push --tag"
	fi
fi
if test -z "$GIT_VERSION"; then
	GIT_VERSION="${TREE_VERSION}-no_git_version"
fi

emacs_major_version="$(echo $TREE_VERSION|cut -d. -f1)"
emacs_minor_version="$(echo $TREE_VERSION|cut -d. -f2)"
emacs_beta_version="$(echo $TREE_VERSION|cut -d. -f3)"
emacs_full_version="$emacs_major_version.$emacs_minor_version.$emacs_beta_version"
sxemacs_codename="Geo"
sxemacs_git_version="$GIT_VERSION"

if test "$emacs_full_version" != "$EXPECTED_TREE_VERSION"; then
    # Note, there is no need check for git repos, because
    # it can only happen in such a case anyway...
    echo "*******************************************"
    echo " WARNING: Your git tags may be out of date "
    echo ""
    echo " Expected tree version $EXPECTED_TREE_VERSION "
    echo " got $emacs_full_version (from $TREE_VERSION) "
    set -x
    git tag
    git describe
    git describe --long
    git config -l
    set +x
    echo "*******************************************" 
fi

autoconf_ver=$(autoconf --version 2>/dev/null | head -n1)
autoheader_ver=$(autoheader --version 2>/dev/null | head -n1)
automake_ver=$(automake --version 2>/dev/null | head -n1)
aclocal_ver=$(aclocal --version 2>/dev/null | head -n1)
libtool_ver=$(libtool --version 2>/dev/null | head -n1)


# When things go wrong... get a bigger hammer!
if test -n "$PHAMMER"; then
    HAMMER=$PHAMMER
fi

if test -n "$HAMMER"; then
	if test -n "$GIT" -a -n "$($GIT symbolic-ref HEAD 2>/dev/null)"; then
		$GIT clean -fxd
	else
		echo "ERROR: Not a git workspace, or you don't have git" >&2
		exit 1
	fi
	unset HAMMER
fi


cat>sxemacs_version.m4<<EOF
dnl autogenerated version number
m4_define([SXEM4CS_VERSION], [$emacs_full_version])
m4_define([SXEM4CS_MAJOR_VERSION], [$emacs_major_version])
m4_define([SXEM4CS_MINOR_VERSION], [$emacs_minor_version])
m4_define([SXEM4CS_BETA_VERSION], [$emacs_beta_version])
m4_define([SXEM4CS_BETA_P], [$emacs_is_beta])
m4_define([SXEM4CS_GIT_VERSION], [$sxemacs_git_version])
m4_define([SXEM4CS_CODENAME], [$sxemacs_codename])
m4_define([4UTOCONF_VERSION], [$autoconf_ver])
m4_define([4UTOHEADER_VERSION], [$autoheader_ver])
m4_define([4CLOCAL_VERSION], [$aclocal_ver])
m4_define([4UTOMAKE_VERSION], [$automake_ver])
m4_define([4IBTOOL_VERSION], [$libtool_ver])
EOF

if test -z "$FORCE"; then
    FORCE=
else
    rm -rf autom4te.cache aclocal.m4
    FORCE=--force
fi

if type glibtoolize 2>/dev/null; then
    LIBTOOLIZE=glibtoolize
else
    LIBTOOLIZE=libtoolize
fi

autoreconf $FORCE --verbose --install -Wall

# hack-o-matic.  Using gmp's config.{guess,sub} lets us have properer
# detected machine configurations --SY.
guess=$(grep GMP config.guess)
sub=$(grep GMP config.sub)
if test -z "${guess}"; then
    mv -vf config.guess configfsf.guess
    cp -v configgmp.guess config.guess
fi
if test -z "${sub}"; then
    mv -vf config.sub configfsf.sub
    cp -v configgmp.sub config.sub
fi

cd $olddir
