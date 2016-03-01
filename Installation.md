# Installation #

Since EDAMS isn't available in binary package, you'll need to compile it yourself.

# Details #

To be compiled, EDAMS needs the following libraries:

> ## EFL (the Enlightenment Foundation Libraries): Installation ##

  * Debian,Ubuntu https://launchpad.net/~efl/+archive/trunk
  * ArchLinux http://wiki.archlinux.org/index.php/E17
  * Gentoo http://overlays.gentoo.org/dev/vapier/wiki/enlightenment
  * Slackware http://slacke17.sourceforge.net/
  * OpenSUSE http://en.opensuse.org/Portal:Enlightenment
  * SLE, Fedora, CentOS, Mandriva  http://download.opensuse.org/repositories/X11:/Enlightenment:/Factory/
  * Others, you'll need sources: see 'Releases' content from http://enlightenment.org/p.php?p=download&l=en

> ## xPL\_Hub: Compilation and installation ##

  * If you distribution have package(ArchLinux,Fedora...), install it with your package manager(PACMAN, RPM...)
  * For others, you'll need to download it from http://www.xpl4java.org/xPL4Linux/downloads/xPLHub.tgz and install by by doing

```
tar -xvzf xPLHub.tgz
```

see INSTALL.txt in xPLHub directory to see how to install it.

> ## EDAMS: Compilation and installation ##

To compile EDAMS, you'll need to use subversion(svn), some developpement tools(gcc autoconf, automake,libtool..).

```
svn checkout http://edams.googlecode.com/svn/trunk/ edams-read-only
cd edams-read-only
sh autogen.sh
make
sudo make install 
```


Now all should be ok, try to run EDAMS:

```
edams
```