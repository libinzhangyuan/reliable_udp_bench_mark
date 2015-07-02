This project is a bench mark test for udp reliable protocal framework.
The bench mark is for realtime pvp game. For example, the multiplayer first person shooting game.

Followwing framework will be test,
UDT    -   UDP-based Data Transfer Protocol   https://sourceforge.net/projects/udt/
kcp    -   A Fast and Reliable ARQ Protocol   https://github.com/skywind3000/kcp
enet   -   Reliable UDP networking library    http://enet.bespin.org/index.html

You can see the bench mark test result in bench_mark.md.

### Building udt_bench_mark:
1. making gcc version >= 4.8
2. download UDT 4.11 from http://nchc.dl.sourceforge.net/project/udt/udt/4.11/udt.sdk.4.11.tar.gz
    - or just use the pack in install_pack folder.
3. unpack udt.sdk.4.11.tar.gz and copy udt4 folder into udt_bench_mark.
4. modify the OS and ARCH in allmake.sh.
5. $ . allmake.sh

server_dgram  - a server using unblock SOCK_DGRAM socket by checking read/write frequently.
client_dgram  - a client using unblock SOCK_DGRAM socket by checking read/write frequently.

server_stream - a server using unblock SOCK_STREAM socket by checking read/write frequently.
client_stream - a client using unblock SOCK_STREAM socket by checking read/write frequently.


### Run test:
server/server 127.0.0.1 12345
client_epoll/client_epoll 22222 127.0.0.1 12345 1400 2>>./state.log
or  sh launcher_clients.sh




Other notes:
