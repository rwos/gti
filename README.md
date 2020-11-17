gti
===

Just a silly `git` launcher, basically. Inspired by `sl`.

Build and Install
-----------------

From a package system:
* [Fedora](https://src.fedoraproject.org/rpms/gti)
* it's in ArchLinux' AUR as [gti-git](https://aur.archlinux.org/packages/gti-git/) and as [gti](https://aur.archlinux.org/packages/gti/)
* [Gentoo](https://packages.gentoo.org/packages/dev-vcs/gti)
* [Nix/NixOS](https://search.nixos.org/packages?show=gti&query=gti)
* [FreeBSD ports](http://svnweb.freebsd.org/ports/head/games/gti/)
* [OpenBSD ports](http://openports.se/games/gti)
* [Homebrew/MacOS X](https://formulae.brew.sh/formula/gti#default)
* [Cygwin](https://cygwin.com/packages/summary/gti.html)

From source:

    $ make
    $ make install # as root, probably

The default install PREFIX is `/usr/bin`.

You can change the speed of the car at runtime via `GTI_SPEED`.
For example:

    $ GTI_SPEED=2000 gti push # default is 1000

Usage
-----

Try typing `git` really fast, on an unfamiliar keyboard.

Author and License
------------------

Copyright 2012 by Richard Wossal <richard@r-wos.org>

Permission to use, copy, modify, distribute, and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear in
supporting documentation.  No representations are made about the
suitability of this software for any purpose.  It is provided "as
is" without express or implied warranty.

