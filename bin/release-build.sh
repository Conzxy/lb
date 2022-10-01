#/bin/bash
if [ $# -lt 1 ]; 
then
  echo "Usage: ./build.sh target_name [-v]"
  exit 1
fi

target_name="$1"

cd ~/lb/build
cmake .. -DCMAKE_BUILD_TYPE=Release

if [ $# -gt 1 ] && [ "$2" == "-v" ];
then
    cmake --build . --target $target_name --parallel 8 -v
else
    cmake --build . --target $target_name --parallel 8 
fi
