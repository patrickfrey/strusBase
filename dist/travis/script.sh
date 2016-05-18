#!/bin/sh

OS=$(uname -s)

case $OS in
	Linux)
		cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release \
			-DLIB_INSTALL_DIR=lib -DCMAKE_CXX_FLAGS=-g \
			.
		make
		make test
		sudo make install
		;;

	Darwin)
		# gcc on OSX is a mere frontend to clang, force using gcc 4.8
		if test "X$CC" = "Xgcc"; then
			export CXX=g++-4.8
			export CC=gcc-4.8
		fi
		cmake -DCMAKE_CXX_COMPILER=$(which g++-4.8) -DCMAKE_C_COMPILER=$(which gcc-4.8) \
			-DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=Release \
			-DCMAKE_CXX_FLAGS=-g -G Xcode \
			.
		xcodebuild -configuration Release -target ALL_BUILD
		xcodebuild -configuration Release -target RUN_TESTS
		sudo xcodebuild -configuration Release -target install
		;;
		
	default)
		echo "ERROR: unknown operating system '$OS'."
		;;
esac
	
