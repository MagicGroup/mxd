# set the include path for X, qt and KDE
INCLUDES = $(all_includes)

# these are the headers for your project
noinst_HEADERS = mxd.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(EXTRACTRC) `find . -name \*.ui -o -name \*.rc` > rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/mxd.pot

KDE_ICON = AUTO

#########################################################################
# APPLICATION SECTION
#########################################################################
# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = mxd

# the application source, library search path, and link libraries
mxd_SOURCES = main.cpp mxd.cpp settingdialog.cpp statisticsbase.ui \
	statistics.cpp mxdconfigureuibase.ui
mxd_LDFLAGS = $(KDE_RPATH) $(all_libraries)
mxd_LDADD = -ltqt-mt $(LIB_TDEUI) $(LIB_TDECORE)

# this is where the desktop file will go 
shelldesktopdir   = $(kde_appsdir)/Utilities
shelldesktop_DATA = mxd.desktop

# this is where the shell's XML-GUI resource file goes
shellrcdir   = $(kde_datadir)/mxd
shellrc_DATA = mxdui.rc

scriptdir = ${prefix}/bin
script_DATA = rppppoek.sh

SUBDIRS = pics
