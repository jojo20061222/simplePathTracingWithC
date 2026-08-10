#!/bin/zsh

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


optimizationFlags="-O2 -pthread -g -DDEBUG -flto -DDOUBLE_AS_REAL"

echo "all: $binaryPath/main"

mainDependency="$binaryPath/main: "

for file in src/*.c; do
    filename=$(basename $file)
    if [[ $filename == pathTracing.c ]]; then
        continue
    fi
    
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
echo -e "\t$CC \$^ -o \$@ \`sdl2-config --libs\` -lm -lpthread -flto"
echo
echo ".PHONY: clean"
echo "clean: "
echo -e "\t- rm $binaryPath/*"
echo -e "\t- rm $buildPath/*.o"