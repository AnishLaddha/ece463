# The code is subject to Purdue University copyright policies.
# DO NOT SHARE, DISTRIBUTE, OR POST ONLINE
#

#!/bin/bash

echo cc -w -o dns_udp_server src/dns_udp_server.c
cc -w -o dns_udp_server src/dns_udp_server.c
echo cc -w -o dns_udp_client src/dns_udp_client.c
cc -w -o dns_udp_client src/dns_udp_client.c
echo cc -w -o file_transfer_tcp_server src/file_transfer_tcp_server.c
cc -w -o file_transfer_tcp_server src/file_transfer_tcp_server.c
echo cc -w -o file_transfer_tcp_client src/file_transfer_tcp_client.c
cc -w -o file_transfer_tcp_client src/file_transfer_tcp_client.c
