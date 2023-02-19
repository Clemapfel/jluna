# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# http://www.sphinx-doc.org/en/master/config

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
# import os
# import sys
# sys.path.insert(0, os.path.abspath('.'))
#
# reference used: https://devblogs.microsoft.com/cppblog/clear-functional-c-documentation-with-sphinx-breathe-doxygen-cmake/

import subprocess, os, hachibee_sphinx_theme

# -- Project information -----------------------------------------------------

project = "jluna"
copyright = '2022, C. Cords'

# -- Read the Docs Config ----------------------------------------------------

breathe_projects = {}
if os.environ.get('READTHEDOCS', None) == 'True':
    subprocess.call('doxygen', shell=True)
    breathe_projects['jluna'] = '.doxygen/xml'


# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
#...

extensions = ["breathe", "myst_parser"]
breathe_default_project = "jluna"

#...

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']

# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = "furo"

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']

# disable "show source" in html output
html_show_sourcelink = False

html_logo = "_static/logo.png"
html_favicon = "_static/favicon.png"
# note: this icon is from noto-emoji, which is a public domain emote package. No copyright applies

# enable markdown links
myst_heading_anchors = 7
