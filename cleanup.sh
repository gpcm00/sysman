DIR=$(dirname $(realpath $0))

cd $DIR

make clean

rm -f $DIR/daemon_manager/test/c_messages 
rm -f $DIR/daemon_manager/test/cpp_messages