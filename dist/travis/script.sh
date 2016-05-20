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
		if test "X$CC" = "Xgcc"; then
			# gcc on OSX is a mere frontend to clang, force using gcc 4.8
			export CXX=g++-4.8
			export CC=gcc-4.8
			# forcing brew versions (of gettext) over Mac versions
			export CFLAGS=-I/usr/local
			export CXXFLAGS=-I/usr/local
			export LDFLAGS=-L/usr/local/lib
			cmake \
				-DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=Release \
				-DCMAKE_CXX_FLAGS=-g -G 'Unix Makefiles' \
				.
			make
			make test
			sudo make install
		else
			# forcing brew versions (of gettext) over Mac versions
			export CFLAGS=-I/usr/local
			export CXXFLAGS=-I/usr/local
			export LDFLAGS=-L/usr/local/lib
			cmake \
				-DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=Release \
				-DCMAKE_CXX_FLAGS=-g -G Xcode \
				.
			xcodebuild -configuration Release -target ALL_BUILD
			xcodebuild -configuration Release -target RUN_TESTS
			sudo xcodebuild -configuration Release -target install
		fi
		;;
		
	*)
		echo "ERROR: unknown operating system '$OS'."
		;;
esac
	
