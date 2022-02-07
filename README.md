This repository comprises RDT 3.0 implementation done with User Datagram Protocol.

>gcc server.c functions.c -pthread -o server\
>./server server-port-number<br/>

OR
>make server_target<br/>
--- - ---
>gcc client.c functions.c -pthread -o client\
>./client client-port-number server-port-number

OR
>make client_target

<!---While running only the client and the server, without troll binary, <server-port-number> argument of server and <client-port-number> argument of client should be the same.--->
