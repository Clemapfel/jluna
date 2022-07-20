#bin/bash
doxygen
sphinx-build -Ea -j 4 -b html -Dbreathe_projects.jluna=doxygen/xml . ./out