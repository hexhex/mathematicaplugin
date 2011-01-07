AC_PREREQ([2.61])
AC_INIT([dlvhex-mathematicaplugin],[0.1.0],[annemarie.goldmann@chello.at])
AC_CONFIG_SRCDIR([src/mathematicaplugin.cpp])

MAJOR=`echo $PACKAGE_VERSION | sed "s,^\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\),\1,"`
MINOR=`echo $PACKAGE_VERSION | sed "s,^\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\),\2,"`
MICRO=`echo $PACKAGE_VERSION | sed "s,^\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\),\3,"`

AC_DEFINE_UNQUOTED([MATHEMATICAPLUGIN_MAJOR], [$MAJOR], [Major version number of the mathematica-plugin])
AC_DEFINE_UNQUOTED([MATHEMATICAPLUGIN_MINOR], [$MINOR], [Minor version number of the mathematica-plugin])
AC_DEFINE_UNQUOTED([MATHEMATICAPLUGIN_MICRO], [$MICRO], [Micro version number of the mathematica-plugin])

AM_INIT_AUTOMAKE

AC_CANONICAL_HOST

# by default we look into /opt/local and /sw for include files (macports vs. fink)
case $host_os in
     darwin*|macosx*)
	CPPFLAGS="$CPPFLAGS -I/opt/local/include -I/sw/include"
     ;;
esac 


# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for header files.
AC_HEADER_DIRENT
AC_CHECK_HEADERS([sys/param.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST

# Checks for library functions.
AC_LIBTOOL_DLOPEN # we build a module library
AC_DISABLE_STATIC # we don't need a static library
AC_PROG_LIBTOOL

#
# check for other libraries here
#
# Assume Mathematica in this directory:
ASSUMED_MATH_BASE="/usr/local/Wolfram/Mathematica/7.0/SystemFiles/Links/MathLink/DeveloperKit/Linux/CompilerAdditions"

#
# running configure --with-mathematica will take mathematicadir.
#

AC_ARG_WITH(mathematica,
           [  --with-mathematica          Base directory of Mathematica], 
           [mathematicadir=$withval],
           [mathematicadir=$ASSUMED_MATH_BASE]
           )

#AC_SUBST([mathematicadir])

# Look for Mathematica header and library in additional directories
CPPFLAGS="$CPPFLAGS -I$mathematicadir"
LDFLAGS="$LDFLAGS -L$mathematicadir"

AC_CHECK_HEADERS(mathlink.h,
                 [],
		 AC_MSG_ERROR([Mathematica header not found. \
Check your Mathematica installation \
or use --with-mathematica to \
specify the Mathematica utility directory.]))

#
# check for the Mathematica library
#
AC_CHECK_LIB([ML32i3], 
             MLEnvironmentData, 
             [],
	     [AC_CHECK_LIB([ML64i3], 
             MLEnvironmentData, 
             [],
	     [AC_MSG_ERROR([Mathematica library not found. \
Check your Mathematica installation \
or use --with-mathatica to specify \
the Mathematica utility directory.])])
	     ],
	     [-lm -lpthread -lrt]
)


#
# of course we need dlvhex itself!
#
PKG_CHECK_MODULES([DLVHEX], [dlvhex >= 1.7])


# checking for boost libs
BOOST_REQUIRE([1.41.0])
BOOST_SMART_PTR
BOOST_STRING_ALGO
BOOST_TOKENIZER


#
# where the plugin will be installed by default.
# running configure will take sysplugindir from dlvhex.pc,
# running configure --enable-userinstall will take homeplugindir.
#

AC_ARG_ENABLE(userinstall, 
             [  --enable-userinstall    Install plugin into home directory],
             [plugindir=$HOME/`pkg-config dlvhex --variable=userplugindir`],
             [plugindir=`pkg-config dlvhex --variable=sysplugindir`]
             )
AC_SUBST([plugindir])

#
# all needed makefiles
#
AC_CONFIG_FILES([
           Makefile 
           src/Makefile
	   include/Makefile
])

AC_OUTPUT

