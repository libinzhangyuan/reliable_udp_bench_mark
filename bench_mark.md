### Bench mark of udp reliable framework:
* UDT    -   UDP-based Data Transfer Protocol   [official website](https://sourceforge.net/projects/udt/)
* kcp    -   A Fast and Reliable ARQ Protocol   [official website](https://github.com/skywind3000/kcp)
* enet   -   Reliable UDP networking library    [official website](http://enet.bespin.org/index.html)

### Purpose
* This bench mark is for the situation of realtime pvp game.  For example, the multiplayer first person shooting game.
* The requirement of realtime pvp game is packet is small and frequently. It wants a minimal delay. And the worst delay should be not so worse.

### Test environment
* The tests run server on a server on internet. The Bandwidth of internet is 5M.
* And run client_epoll or client on home computer var wifi. The Bandwidth of ADSL is 10M.
* Both Bandwidth is much bigger than we need (10 times more).

* The client send 500 bytes data as one packet in every 50 milliseconds. Or 50 bytes for another test.
* The server sendback the data after receiving the packet.

## UDT
### UDT with epoll (SOCK_DGRAM and unblock socket)
[See code](https://github.com/libinzhangyuan/udt_patch_for_epoll)
* a. Bench mark log for 500 bytes in every 50 milliseconds. [here](https://github.com/libinzhangyuan/udt_patch_for_epoll/blob/master/bench_mark/udt_500.log).
* b. Bench mark log for 50 bytes in every 50 milliseconds. [here](https://github.com/libinzhangyuan/udt_patch_for_epoll/blob/master/bench_mark/udt_50.log).
* c. Bench mark log for 500 bytes in every 50 milliseconds using another framework kcp. [here](https://github.com/libinzhangyuan/udt_patch_for_epoll/blob/master/bench_mark/kcp_500.log). <br>
kcp is a [Fast and Reliable ARQ Protocol](https://github.com/skywind3000/kcp) <br>
The test project of kcp is [asio_kcp](https://github.com/libinzhangyuan/asio_kcp) <br>
I run b and c at the same time on same computer. So we known  the network situation by comparing log from b and c. <br>

### The conclusion of this UDT SOCK_DGRAM epoll mode:
* The udt epoll is not good at realtime pvp game currently.
* The latency is perfect at normal situation.
* UDT will fall into a badly situation when there is a network lag.
* The badly situation is more than serval seconds lag. Even serval ten seconds. And the recovery is not expected.
* The udt with epoll has good performance if sending 500 bytes in every 500 milliseconds. So you can use udt with epoll for some other situation.

### UDT without epoll (unblock socket)
[See code](https://github.com/libinzhangyuan/reliable_udp_bench_mark/tree/master/udt_bench_mark) <br>
This test call unblock recv and send repeatedly.

* a. Bench mark log for 500 bytes in every 50 milliseconds SOCK_DGRAM. [here](https://github.com/libinzhangyuan/reliable_udp_bench_mark/blob/master/bench_mark_log/udt_dgram_500.log)
* b. Bench mark log for 500 bytes in every 50 milliseconds SOCK_STREAM. [here](https://github.com/libinzhangyuan/reliable_udp_bench_mark/blob/master/bench_mark_log/udt_stream500.log)
* c. Bench mark log for 500 bytes in every 50 milliseconds using another framework kcp. [here](https://github.com/libinzhangyuan/reliable_udp_bench_mark/blob/master/bench_mark_log/kcp_500.log)
I run b and c at the same time on same computer. So we known the network situation by comparing log from b and c. <br>

### The conclusion of UDT without epoll:
* The udt without epoll is same as with epoll. It is not good at realtime pvp game currently.

## enet
   - Test project is [here](https://github.com/libinzhangyuan/enet_bench_test)
   - It change the ENET_PEER_DEFAULT_ROUND_TRIP_TIME from 500 to 50.
* a. enet log for 500 bytes in every 50 milliseconds [here](https://github.com/libinzhangyuan/enet_bench_test/blob/master/bench_mark_log/enet_500.log)
* b. kcp log for 500 bytes in every 50 milliseconds at same time with a [here](https://github.com/libinzhangyuan/enet_bench_test/blob/master/bench_mark_log/kcp_500.log)
   - I run a and b at the same time on same computer. So we known the network situation by comparing log from a and b. <br>
   - The lag is same as 50 when do not change ENET_PEER_DEFAULT_ROUND_TRIP_TIME. <br>

### The conclusion of enet test
* The enet is better than UDT about realtime pvp game.
* The enet has some lag about 1 seconds more time times. And the recovery of lag situation only some seconds.
* It is worse than kcp. But some game allow 1 second lag.

## kcp
* the bench mark log of kcp is show in UDT and enet test.
* kcp's lag less than 1 second always. The lag is 1/3 comparing to kcp when network lag happen.
* kcp is better than UDT and enet. The lag is less than 2 seconds when network lag happen.



# Final conclusion.
#### kcp
* The kcp is the first choice for realtime pvp game.
* The lag is less than 1 second when network lag happen.  3 times better than enet when lag happen.
#### enet
* The enet is a good choice if your game allow 2 second lag.
#### UDT
* UDT is a bad idea. It always sink into badly situation of more than serval seconds lag. And the recovery is not expected.

### other things may change your choice.
* enet has the problem of lack of doc. And it has lots of functions that you may intrest.
* kcp's doc is chinese. (Good thing is the function detail which is writen in code is english. And you can use asio_kcp which is a good wrap.)
    The kcp is a simple thing. You will write more code if you want more feature.
* UDT has a perfect doc.
    UDT may has more bug than others as I feeling.
