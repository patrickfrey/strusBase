#!/bin/sh

cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release \
	-DLIB_INSTALL_DIR=lib -DCMAKE_CXX_FLAGS=-g \
	.
make
make test
sudo make install
