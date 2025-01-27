DIR=$(dirname $(realpath $0))
cd $DIR
cd daemon_manager/test

rm -f c_messages cpp_messages

echo "compiling test files"
g++ cpp_messages.cpp -o cpp_messages
gcc c_messages.c -o c_messages

cd $DIR

make clean
make NEW_LOG_PATH=$(pwd)/log/ NEW_APP_PATH=$(pwd)/daemon_manager/ NEW_LOG_MANAGER_PATH=$(pwd)/log_manager/
