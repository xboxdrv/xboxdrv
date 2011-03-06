##  Xbox/Xbox360 USB Gamepad Userspace Driver
##  Copyright (C) 2010 Ingo Ruhnke <grumbel@gmx.de>
##
##  This program is free software: you can redistribute it and/or modify
##  it under the terms of the GNU General Public License as published by
##  the Free Software Foundation, either version 3 of the License, or
##  (at your option) any later version.
##  
##  This program is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
##  
##  You should have received a copy of the GNU General Public License
##  along with this program.  If not, see <http://www.gnu.org/licenses/>.

DESTDIR = 
PREFIX  = "/usr/local"
DATADIR = "${PREFIX}/share/xboxdrv"
MANDIR  = "${PREFIX}/share/man"
BINDIR  = "${PREFIX}/bin"

xboxdrv:
	scons

clean:
	scons -c
	rm -rf .sconf_temp/
	rm -f .sconsign.dblite
	rm -f config.log

install: install-exec install-man

install-exec: xboxdrv
	install -D xboxdrv "${DESTDIR}${BINDIR}/xboxdrv"
	install -D xboxdrvctl "${DESTDIR}${BINDIR}/xboxdrvctl"

install-man:
	install -D doc/xboxdrv.1 "${DESTDIR}${MANDIR}/man1/xboxdrv.1"

.PHONY : clean install install-exec install-man

# EOF #
