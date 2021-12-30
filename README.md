(<COMPILE SERVER>
gcc server.c functions.c -pthread -o server
</COMPILE SERVER>
&&
<RUN SERVER>
./server <server-port-number>
</RUN SERVER>)
||
make server_target
--- - ---
(<COMPILE CLIENT>
gcc client.c functions.c -pthread -o client
</COMPILE CLIENT>
&&
<RUN CLIENT>
./client <client-port-number> <server-port-number>
</RUN CLIENT>)
||
make client_target

<!---While running only the client and the server, without troll binary, <server-port-number> argument of server and <client-port-number> argument of client should be the same.--->