#!/bin/bash

SRC_DIR=$PWD

mkdir -p $SRC_DIR/build/assets/shaders/
mkdir -p $SRC_DIR/build/assets/textures/

cd assets/shaders

for file in *.metal; do
	root=${file%.*}
	shader_type=${root: -6}
	output_air=$root.air
	output_metallib=$root.metallib

	echo "Compiling shader: $file"
	xcrun -sdk macosx metal    -gline-tables-only -fvectorize -ffast-math -funroll-loops -MO $file -c -o $output_air
	xcrun -sdk macosx metallib $output_air -o $output_metallib

	mv $output_air      $SRC_DIR/build/assets/shaders
	mv $output_metallib $SRC_DIR/build/assets/shaders
done

cd ../textures

for file in *; do
    cp $file $SRC_DIR/build/assets/textures/
done
