# **************************************************************************************************************
# build-cave.csh
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

#setenv S2BUILDPATH "${S2PATH}/scripts"
setenv S2BUILDPATH "scripts"

unsetenv S2INSTALLPATH
#setenv S2EXTRAINC "-I$HOME/git/s2volsurf "
setenv S2EXTRAINC "-I{$S2VOLSURF} "
setenv S2EXTRALIB "-lpng -lz -lpng -lGLEW -L{$S2VOLSURF}/{$S2VOLARCH} -lxrw -L{$CFITSIO} -lcfitsio"

#cbuild.csh cavehd
#$S2BUILDPATH/yuricbuild.csh s2hd shaderService
$S2BUILDPATH/s2cavebuild.csh s2hd hdsupport utility shaderService float16 json routines png_util
