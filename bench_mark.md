#### Bench mark of udp reliable framework:
* UDT    -   UDP-based Data Transfer Protocol   https://sourceforge.net/projects/udt/
* kcp    -   A Fast and Reliable ARQ Protocol   https://github.com/skywind3000/kcp
* enet   -   Reliable UDP networking library    http://enet.bespin.org/index.html

#### Purpose
* This bench mark is for the situation of realtime pvp game.  For example, the multiplayer first person shooting game.
* The requirement of realtime pvp game is packet is small and frequently. It wants a minimal delay. And the worst delay should be not so worst.

#### Test environment
* The tests run server on a server on internet. The Bandwidth of internet is 5M.
* And run client_epoll or client on home computer var wifi. The Bandwidth of ADSL is 10M.
* Both Bandwidth is much bigger than we need (10 times more). And I use iptraf to verify it.

* The client send 500 bytes data as one packet in every 50 milliseconds. Or 50 bytes for another test.
* The server sendback the data after receiving the packet.


#### [UDT with epoll](https://github.com/libinzhangyuan/udt_patch_for_epoll) (SOCK_DGRAM and unblock socket)
* a. [Bench mark log for 500 bytes in every 50 milliseconds](https://github.com/libinzhangyuan/udt_patch_for_epoll/blob/master/bench_mark/udt_500.log).
* b. [Bench mark log for 50 bytes in every 50 milliseconds](https://github.com/libinzhangyuan/udt_patch_for_epoll/blob/master/bench_mark/udt_50.log).
* c. [Bench mark log for 500 bytes in every 50 milliseconds using another framework kcp](https://github.com/libinzhangyuan/udt_patch_for_epoll/blob/master/bench_mark/kcp_500.log).  https://github.com/libinzhangyuan/asio_kcp
I run b and c at the same time on same computer. So we known  the network situation by comparing log from b and c. <br>
The test log of client is some like client_epoll. I ignore the log from client.

## The conclusion of this UDT SOCK_DGRAM epoll mode:
* The udt epoll is not good at realtime pvp game currently.
* The latency is perfect at normal.
* But the udt will fall into a badly situation if there is a small lag.
* The udt with epoll has good performance if sending 500 bytes in every 500 milliseconds. So you can use udt with epoll for some other situation.


#### [UDT without epoll](https://github.com/libinzhangyuan/reliable_udp_bench_mark/tree/master/udt_bench_mark) (unblock socket for SOCK_DGRAM or SOCK_STREAM)
* a. [Bench mark log for 500 bytes in every 50 milliseconds SOCK_DGRAM](https://github.com/libinzhangyuan/reliable_udp_bench_mark/blob/master/bench_mark_log/udt_dgram_500.log)
