mkdir build
cd build
cmake -G "Visual Studio 14 2015" ..
cmake --build .
cd bin
start .
cd ../..
