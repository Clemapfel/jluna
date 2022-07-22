#bin/bash

# needs to be executed inside jluna/docs
# only works on my machine, if you are not a developer of jluna, why are you snooping through my doc?

doxygen
sphinx-build -Ea -j 4 -b html -Dbreathe_projects.jluna=doxygen/xml . ./out

