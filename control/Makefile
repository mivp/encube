#Requires google closure compiler
VERSION = 0.1
COMP = java -jar ~/code/compiler-latest/compiler.jar --jscomp_warning internetExplorerChecks --js=
FLAGS = --js_output_file=
#COMP = cp 
#FLAGS = 

#Sources
SCRIPTS = src/minicave.js src/main.js src/slicer.js src/volume.js src/tools.js
LIBS = lib/gl-matrix-min.js lib/dat.gui.min.js lib/OK-min.js lib/jquery.js lib/jquery.tablesorter.js lib/jquery-ui.js lib/jquery.ui.touch-punch.min.js lib/d3.min.js lib/sharevol-min.js

all: encube.js

.PHONY : clean
clean:
	-rm lib/dat.gui.min.js
	-rm lib/*-min.js
	-rm encube.js

encube.js: $(LIBS)
	#Combine into final bundle
	cat $(LIBS) > encube.js

lib/sharevol-min.js: $(SCRIPTS)
	cat $(SCRIPTS) > lib/sharevol-all.js
	$(COMP)lib/sharevol-all.js $(FLAGS)lib/sharevol-min.js

lib/OK-min.js: lib/OK.js
	$(COMP)lib/OK.js $(FLAGS)lib/OK-min.js

lib/gl-matrix-min.js: lib/gl-matrix.js
	$(COMP)lib/gl-matrix.js $(FLAGS)lib/gl-matrix-min.js

lib/dat.gui.min.js: lib/gl-matrix.js
	$(COMP)lib/dat.gui.js $(FLAGS)lib/dat.gui.min.js
