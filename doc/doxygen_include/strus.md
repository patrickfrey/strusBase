strusBase	 {#mainpage}
=========

The project strusBase implements some libraries with common functions 
and interfaces of the strus projects.
The project strus provides some libraries for building a search engine
for information retrieval.
The project is hosted at <a href="https://github.com/patrickfrey/strus">github</a>.

The most important interface provided by strusBase is the 
[error buffer interface](@ref strus::ErrorBufferInterface).
This interface provides a mechnism to buffer exceptions thrown by strus functions.
C++ exceptions cannot be thrown accross interface boundaries because C++ does not allow 
exceptions be caught thrown by another shared object (module) or library
without intrusive build and linkage rules (all binary artefacts built with the
same compiler and version and with all symbols exported).
[See C++ Coding Standards: 101 Rules, Guidelines, and Best Practices
By Herb Sutter, Andrei Alexandrescu: 61. Don’t allow exceptions to propagate across module boundaries]



