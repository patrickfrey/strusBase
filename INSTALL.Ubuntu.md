Ubuntu 16.04 on x86_64, i686
----------------------------

# Build system
Cmake with gcc or clang. Here in this description we build with 
gcc >= 4.9 (has C++11 support). Build with C++98 is possible.

# Prerequisites
Install packages with 'apt-get'/aptitude.

## Required packages
	boost-all >= 1.57

# Fetch sources
	git clone https://github.com/patrickfrey/strusBase
	cd strusBase

# Configure with GNU C/C++
	cmake -DCMAKE_BUILD_TYPE=Release .

# Configure with Clang C/C++
	cmake -DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_C_COMPILER="clang" -DCMAKE_CXX_COMPILER="clang++" .

# Build
	make

# Run tests
	make test

# Install
	make install

