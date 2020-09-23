# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
# import os
# import sys
# sys.path.insert(0, os.path.abspath('.'))


# -- Project information -----------------------------------------------------

project = 'VK2D'
copyright = '2020, Niko Kauppi'
author = 'Niko Kauppi'

# The full version, including alpha/beta/rc tags
release = '1'


# -- General configuration ---------------------------------------------------

# The suffix(es) of source filenames.
# You can specify multiple suffix as a list of string:
#
# source_suffix = ['.rst', '.md']
source_suffix = '.rst'

# The master toctree document.
master_doc = 'index'

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    "breathe",
    'sphinx.ext.todo',
    'sphinx.ext.githubpages',
]

# Breathe configuration
breathe_default_project = "VK2D"

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']

# The name of the Pygments (syntax highlighting) style to use.
pygments_style = 'sphinx'


# -- Options for HTML output -------------------------------------------------

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']


# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.



# PDJ theme

if 1:
    import sphinx_pdj_theme
    html_theme = 'sphinx_pdj_theme'
    htm_theme_path = [sphinx_pdj_theme.get_html_theme_path()]
    
    html_theme_options = {
        #'style': 'darker'
    }



# cake phd theme

if 0:
    import cakephp_theme
    html_theme_path = [cakephp_theme.get_html_theme_path()]
    html_theme = 'cakephp_theme'
    extensions.append( 'cakephp_theme' )
    html_context = {
        'maintainer': 'Sphinx-themes test',
        'project_pretty_name': 'Sphinx Themes',
        'projects': {
            'CakePHP Book': 'https://book.cakephp.org/',
            'Some other project': 'https://example.com/',
        }
    }



# t3SphinxThemeRtd theme

if 0:
    html_theme = 't3SphinxThemeRtd'
    import t3SphinxThemeRtd
    html_theme_path = [t3SphinxThemeRtd.get_html_theme_path()]



# groundwork theme

if 0:
    html_theme = 'groundwork'
    html_theme_options = {
    "sidebar_width": '300px',
    "stickysidebar": True,
    "stickysidebarscrollable": True,
    "contribute": True,
    "github_fork": "useblocks/groundwork",
    "github_user": "useblocks",
    }
