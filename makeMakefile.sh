#!/bin/zsh

CC="clang"
buildPath="build"
binaryPath="bin"
sourcePath="src"
optimizationFlags="-O2 -g -pthread"

echo "all: $binaryPath/main"

mainDependency="$binaryPath/main: "

for file in src/*.c; do
    filename=$(basename $file)
    xfpref=${filename%%.*}
    mainDependency+="$buildPath/$xfpref.o "
    if [[ $filename == main.c ]]; then
        echo "$buildPath/$($CC `sdl2-config --cflags` -MM $file)"
        echo -e "\t$CC -o \$@ -c \$< \`sdl2-config --cflags\` $optimizationFlags"
    else 
        echo "$buildPath/$($CC -MM $file)"
        echo -e "\t$CC -o \$@ -c \$< $optimizationFlags"
    fi
done

echo $mainDependency
echo -e "\t$CC \$^ -o \$@ \`sdl2-config --libs\` -lm -lpthread"
echo
echo ".PHONY: clean"
echo "clean: "
echo -e "\t- rm $binaryPath/main"
echo -e "\t- rm $buildPath/*.o"