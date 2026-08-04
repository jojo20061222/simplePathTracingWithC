#!/bin/bash

CC="clang"
buildPath="build"
binaryPath="bin"
sourcePath="src"

if [[ ! (-d $binaryPath) ]]; then
    mkdir $binaryPath
fi

if [[ ! (-d $buildPath) ]]; then
    mkdir $buildPath
fi

if [[ ! (-d $sourcePath) ]]; then
    echo "Where is your source?"
fi


optimizationFlags="-O2 -pthread -g -flto"

echo "all: $binaryPath/pathTracing"

mainDependency="$binaryPath/pathTracing: "

for file in src/*.c; do
    if [[ $file == "src/main.c" ]]; then
        continue
    fi
    filename=$(basename $file)
    xfpref=${filename%%.*}
    mainDependency+="$buildPath/$xfpref.o "
    echo "$buildPath/$($CC -MM $file)"
    echo -e "\t$CC -o \$@ -c \$< $optimizationFlags"
done

echo $mainDependency
echo -e "\t$CC \$^ -o \$@ -lm -lpthread -flto"
echo
echo ".PHONY: clean"
echo "clean: "
echo -e "\t- rm $binaryPath/*"
echo -e "\t- rm $buildPath/*.o"