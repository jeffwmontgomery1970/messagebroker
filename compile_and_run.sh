#!/bin/sh

echo "generating libpubsub.so library.."
g++ -shared -fPIC *.h subscriber.cpp subscriberlist.cpp publisher.cpp -o libpubsub.so
echo "copying libpubsub.so library to share library folder"
sudo cp libpubsub.so /usr/lib 
echo "compiling sample application.."
g++ main.cpp -L $PWD -o app -lpubsub -lpthread -lsbio-wrapper
echo "updating LD_LIBRARY_PATH.."
export LD_LIBRARY_PATH=$PWD:$LD_LIBRARY_PATH
echo "running application..defaults to Publisher mode with default channel and log_file name"
echo "--------------------------------------------------------"
./app