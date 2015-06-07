#!/bin/sh
#
# A script to setup your git area to contribute back to SXEmacs
#
# (C) 2008 Nelson Ferreira
#
# This program is free software; you can redistribute it and/or modify it
# under a BSD-like licence.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
# Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
# Neither the name of the Technical University of Berlin nor the names of its
# contributors may be used to endorse or promote products derived from this
# software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
echo "If you have any questions or concerns about how to contribute "
echo "to SXEmacs, ask us on freenode channel #sxemacs"
USER_EMAIL=$(git config user.email)
if [ -z "$USER_EMAIL" ]; then
    echo "You need to setup your email address with:"
    echo "    git config user.email <your email address>"
    exit 1
fi
USER_NAME=$(git config user.name)
if [ -z "$USER_NAME" ]; then
    echo "You need to configure git with your name:"
    echo "    git config user.name \"John Doe\""
    exit 1
fi
git branch --track for-steve origin/master
git checkout for-steve
if ! type gpg > dev/null 2>&1 ; then
    echo "MANDATORY: Please install gpg and create/install your private key."
    exit 1
fi
echo ""
SIGNKEY=$(git config user.signingkey)
if [ -z "$SIGNKEY" ]; then
    KEYGUESS=$(gpg --list-keys $USER_EMAIL |\
               awk '/^pub/ { split($2,k,"/"); print k[2] }')
    echo "You need to setup your GPG signing key:"
    echo "    git config user.signingkey ${KEYGUESS:-<GPG key signature>}"
    exit 1
fi
CO_ALIAS=$(git config alias.co)
if [ -z "$CO_ALIAS" ]; then
    echo "RECOMMENDED: It is recommended you define the 'co' alias"
    echo "             to quickly switch betwen the master and "
    echo "             for-steve branches."
    echo "    git config alias.co checkout"
fi
REMOTE=$(git remote | grep -v origin)
INITIALS=""
for word in $(git config user.name); do
  INITIALS="${INITIALS}$(echo $w | cut -c1)"
done
DEVKEYTAG=$(git show devkey.$INITIALS | grep Tagger | sed 's/^Tagger: //')
EXPECTEDTAG="$USER_NAME <$USER_EMAIL>"
if [ -z "$DEVKEYTAG" ]; then
    echo "RECOMMENDED: It is recommended you tag (and push) your "
    echo "             GPG public key as devkey.${INITIALS}:"
    echo ""
    echo "       git tag -s devkey.$INITIALS \ "
    echo "               -m 'Public key for $USER_NAME <$USER_EMAIL>' \ "
    echo "          $(gpg --armor --export $USER_EMAL | \ "
    echo "            git hash-object -w --stdin)"
    echo ""
    echo "Don't forget to push the tag to your remote:"
    echo "       git push <myremote> devkey.${INITIALS}"
    exit 1
elif [ "$DEVKEYTAG" != "EXPECTEDTAG" ]; then
    LONGDEVKEYTAG=$(git show devkey.$INITIALS.$SIGNKEY | grep Tagger)
    if [ -z "$LONGDEVKEYTAG" ]; then
    echo "It seems there is someone else using your initials for a key:"
    echo "       $DEVKEYTAG"
    echo "RECOMMENDED: It is recommended you tag (and push) your "
    echo "             GPG public key as devkey.${INITIALS}.${SIGNKEY}:"
    echo ""
    echo "       git tag -s devkey.${INITIALS}.${SIGNKEY} \ "
    echo "               -m 'Public key for $USER_NAME <$USER_EMAIL>' \ "
    echo "          $(gpg --armor --export $USER_EMAL | \ "
    echo "            git hash-object -w --stdin)"
    echo ""
    echo "Don't forget to push the tag to your remote:"
    echo "       git push <myremote> devkey.${INITIALS}.${SIGNKEY}"
fi
MAINTAINER=$(git show maintaner-pgp | grep Tagger | sed 's/^Tagger: //')
if ! gpg list-keys "$MAINTAINER" > /devnull 2>&1 ; then
    echo "RECOMMENDED: You should import the maintainers key to your "
    echo "             key ring so you can verify objects on the repository"
    echo "       git show maintainer-pgp | gpg --import"
    echo ""
fi
if [ -z "$REMOTE" ]; then
    echo "MANDATORY: You now must configure your remote repository "
    echo "           location using:"
    echo "    git remote add <myremote> <repository location>"
    echo ""
    echo "           We recommend that you use 'myremote' explicitly"
    echo "           for the remote name, but it can be whatever name"
    echo "           you wish, except origin"
    echo "           The repository location can be either a git server"
    echo "               git://example.com/sxemacs.git"
    echo "           or an ssh accessible location:"
    echo "               ssh://user@example.com/~/path/to/git"
    echo "           in this last case it is VERY convenient that you "
    echo "           setup SSH public key authentication."
else
    echo "Please verify that one of these remotes is for your SXEmacs "
    echo "public repository"
    for r in $REMOTE; do
	git remote show $r
    done
fi
echo ""
echo "Make sure to read the SPPM for more information."
echo "Info node: (sppm)Setting up a publicly accessible repo"
echo ""
