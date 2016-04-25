#!/bin/sh
echo "-------------------------"
echo "Making executables"
make
chmod +x server
chmod +x client
echo "DONE"
echo "-------------------------"
echo "Creating work directories"
SERV_DIR="server_sandbox/"
CLNT="client"
CLIE_DIR="client_sandbox/"
BC="big_cli_file"
BS="big_srv_file"
LC="lit_cli_file"
LS="lit_srv_file"
if [ ! -d $SERV_DIR ]; then
	mkdir server_sandbox/
fi
if [ ! -d $CLIE_DIR ]; then
	mkdir client_sandbox/
fi
echo "DONE"
echo "-------------------------"
echo "Copying executables and files into directories"
cp server $SERV_DIR
cp client $CLIE_DIR
cp $BC $CLIE_DIR
cp $LC $CLIE_DIR
cp $BS $SERV_DIR
cp $LS $SERV_DIR
echo "DONE"
echo "-------------------------"
echo "Executing some tests"
echo "-------------------------"
cd $SERV_DIR
echo "Starting server with wrong parameters"
./server -P 9999 
ERR=$?
if [ 1 -eq $ERR ]; then
	echo "TEST OK"
fi
echo "-------------------------"
echo "Starting server on port 9999"
./server -p 9999 &
SERVER_PID=$!
echo "PID:"
echo $SERVER_PID
echo "OK"
cd ..
echo "-------------------------"
cd $CLIE_DIR
echo "Starting client with wrong parameters"
./client -h localhost -u lit_cli_file 
ERR=$?
if [ 1 -eq $ERR ]; then
	echo "OK"
else 
	echo "ERR"
fi
echo "-------------------------"
echo "Starting client on non listening server port"
./client -h localhost -p 10000 -u lit_cli_file
ERR=$?
if [ 1 -eq $ERR ]; then
	echo "OK"
else 
	echo "ERR"
fi
echo "-------------------------"
echo "Starting upload little file to server"
./client -h localhost -p 9999 -u lit_cli_file
ERR=$?
if [ 0 -eq $ERR ]; then
	echo "OK"
else 
	echo "ERR"
fi
echo "-------------------------"
echo "Starting upload big file to server"
./client -h localhost -p 9999 -u big_cli_file
ERR=$?
if [ 0 -eq $ERR ]; then
	echo "OK"
else 
	echo "ERR"
fi
echo "-------------------------"
echo "Starting upload big file to server again"
./client -h localhost -p 9999 -u big_cli_file
ERR=$?
if [ 0 -eq $ERR ]; then
	echo "OK"
else 
	echo "ERR"
fi
echo "-------------------------"
echo "Starting download little file from server"
./client -h localhost -p 9999 -d lit_srv_file
ERR=$?
if [ 0 -eq $ERR ]; then
	echo "OK"
else 
	echo "ERR"
fi
echo "-------------------------"
echo "Starting download big file from server"
./client -h localhost -p 9999 -d big_srv_file
ERR=$?
if [ 0 -eq $ERR ]; then
	echo "OK"
else 
	echo "ERR"
fi
echo "-------------------------"
echo "Starting download big file from server again"
./client -h localhost -p 9999 -d big_srv_file
ERR=$?
if [ 0 -eq $ERR ]; then
	echo "OK"
else 
	echo "ERR"
fi
echo "-------------------------"
echo "Starting download nonexising file from server"
./client -h localhost -p 9999 -d fiile
ERR=$?
if [ 1 -eq $ERR ]; then
	echo "OK"
else 
	echo "ERR"
fi
# echo "-------------------------"
# echo "Starting upload client executable file to server"
# ./client -h localhost -p 9999 -u client
# ERR=$?
# if [ 0 -eq $ERR ]; then
# 	echo "OK"
# else 
# 	echo "ERR"
# fi
echo "-------------------------"
echo "KILLING SERVER WITH PID:"
echo $SERVER_PID
kill $SERVER_PID
ERR=$?
if [ 0 -eq $ERR ]; then
	echo "SERVER KILLED"
else 
	echo "ERROR KILLING SERVER"
fi
echo "-------------------------"
cd ..
echo "Checking files structures"
echo "-------------------------"
#  checking files on : 
#    client side | server side
diff $CLIE_DIR$LS $SERV_DIR$LS
ERR=$?
if [ 0 -eq $ERR ]; then
	echo "FILE "$LS" MATCHES"
else 
	echo "FILE "$LS" DOES NOT MATCH"
fi
#----------------------------------
diff $CLIE_DIR$BS $SERV_DIR$BS
ERR=$?
if [ 0 -eq $ERR ]; then
	echo "FILE "$BS" MATCHES"
else 
	echo "FILE "$BS" DOES NOT MATCH"
fi
#----------------------------------
diff $SERV_DIR$LC $CLIE_DIR$LC
ERR=$?
if [ 0 -eq $ERR ]; then
	echo "FILE "$LC" MATCHES"
else 
	echo "FILE "$LC" DOES NOT MATCH"
fi
#----------------------------------
diff $SERV_DIR$BC $CLIE_DIR$BC 
ERR=$?
if [ 0 -eq $ERR ]; then
	echo "FILE "$BC" MATCHES"
else 
	echo "FILE "$BC" DOES NOT MATCH"
fi
#----------------------------------
# diff $SERV_DIR$CLNT $CLIE_DIR$CLNT
# ERR=$?
# if [ 0 -eq $ERR ]; then
# 	echo "FILE "$BC" MATCHES"
# else 
# 	echo "FILE "$BC" DOES NOT MATCH"
# fi
echo "-------------------------"
echo "DONE"
echo "-------------------------"