#! /bin/sh
$EXTRACTRC *.ui *.kcfg *.rc >> rc.cpp || exit 11
LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`
if test -n "$LIST"; then
$XGETTEXT $LIST rc.cpp -o $podir/kmplot.pot
fi
