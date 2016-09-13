# **************************************************************************************************************
# build-mac.csh
#
# Copyright (c) 2015-2016 Dany Vohl, David G. Barnes, Christopher J. Fluke,
#                    Yuri Benovitski, Tsz Ho Wong, Owen L Kaluza, Toan D. Nguyen.
#
# This file is part of encube.
#
# encube is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# encube is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with encube.  If not, see <http://www.gnu.org/licenses/>.
#
# We would appreciate it if research outcomes using encube would
# provide the following acknowledgement:
#
# "Visual analytics of multidimensional data was conducted with encube."
#
# and a reference to
#
# Dany Vohl, David G. Barnes, Christopher J. Fluke, Govinda Poudel, Nellie Georgiou-Karistianis,
# Amr H. Hassan, Yuri Benovitski, Tsz Ho Wong, Owen L Kaluza, Toan D. Nguyen, C. Paul Bonnington. (2016).
# Large-scale comparative visualisation of sets of multidimensional data. PeerJ Computer Science, In Press.
# **************************************************************************************************************

#!/bin/csh

unsetenv S2INSTALLPATH

#setenv S2EXTRAINC "-I$HOME/code/cave2/s2volsurf -I/usr/X11/include -I/opt/local/include"
setenv S2EXTRAINC "-I${S2VOLSURF} -I/opt/X11/include/ -I/usr/local/include -I/opt/local/include"
#setenv S2EXTRALIB "-L/usr/X11/lib -L/opt/local/lib -lpng -lz -lGLEW"
setenv S2EXTRALIB "-L${S2VOLSURF}/${S2ARCH} -lxrw ${S2VOLSURF}/${S2ARCH}/libxrw.dylib -L/usr/X11/lib -L/usr/local/lib -lpng -lz -lGLEW -lcfitsio"

scripts/s2cavebuild.csh s2hd shaderService float16 json routines hdsupport utility png_util


