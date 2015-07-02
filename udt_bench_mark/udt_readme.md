### UDT    -   UDP-based Data Transfer Protocol   https://sourceforge.net/projects/udt/

#### Building udt_bench_mark:
* making gcc version >= 4.8
* download UDT 4.11 from [udt on sourceforge](http://nchc.dl.sourceforge.net/project/udt/udt/4.11/udt.sdk.4.11.tar.gz)
     - or just use the pack in install_pack folder.
* unpack udt.sdk.4.11.tar.gz and copy udt4 folder into udt_bench_mark.
* modify the OS and ARCH in allmake.sh.
* $ . allmake.sh

#### project introduce
* udt_dgram_server  - a server using unblock SOCK_DGRAM socket by checking read/write frequently.
* udt_dgram_client  - a client using unblock SOCK_DGRAM socket by checking read/write frequently.

* udt_stream_server - a server using unblock SOCK_STREAM socket by checking read/write frequently.
* udt_stream_client - a client using unblock SOCK_STREAM socket by checking read/write frequently.


#### Run test:
##### SOCK_DGRAM:
* $ udt_bench_mark/udt_dgram_server/udt_dgram_server 127.0.0.1 12345
* $ udt_bench_mark/udt_dgram_client/udt_dgram_client 22222 127.0.0.1 12345 1400 2>>./state.log

#####SOCK_STREAM:
* $ udt_bench_mark/udt_stream_server/udt_stream_server 127.0.0.1 12345
* $ udt_bench_mark/udt_stream_client/udt_stream_client 22222 127.0.0.1 12345 1400 2>>./state.log

#### Other notes:
##### 1. the meaning of state.log
* . - epoll_wait timeout return. means no socket prepare.
* N - send: socket is non-blocking (UDT_SNDSYN = false) but no buffer space is available.
* R - remove write event to socket
* A - add write event to socket
* B - epoll busy. sleep for stop add more msg to send buf.
