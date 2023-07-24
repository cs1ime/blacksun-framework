
#!/bin/bash

mkdir build
cd build

mkdir build-framework
cd build-framework
cmake ../.. -DCMAKE_C_COMPILER=clang-14 -DCMAKE_CXX_COMPILER=clang++-14
make -j$(nproc)

cd ..
mkdir build-moonlight
cd build-moonlight
qmake "QMAKE_CC=clang-14" "QMAKE_CXX=clang++-14" ../../moonlight/moonlight-qt/moonlight-qt.pro
make -j$(nproc)

cd ..

mkdir apex
mkdir apex/lib

cp build-framework/apex/apex apex
cp build-framework/blacksun/moonlight-mydrawer/libmoonlight-mydrawer.so apex/lib
cp build-moonlight/app/moonlight apex

cd apex
echo "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd`/lib" > runcheat.sh
echo "sudo -v" >> runcheat.sh
echo "./moonlight &>/dev/null &" >> runcheat.sh
echo "sudo ./apex" >> runcheat.sh
chmod +x *

cd ..



