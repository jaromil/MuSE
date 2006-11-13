#!/bin/sh
# this small script is a dirty fix to doxygen
# which doesn't creates correct graphs in png
# so i must convert from the latex .eps files
# using doxygen 1.3 // jrml 28 march 2004
cd latex
echo "making graphs pictures"
for i in `ls --color=none *.eps`; do
  echo "converting $i ..."
  convert $i "../html/`echo $i|cut -d. -f1`.png"                                                                               
done 
echo "graphs done"
cd -
