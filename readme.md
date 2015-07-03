### This project is a bench mark test for udp reliable protocal framework. <br>
The bench mark is for realtime pvp game. For example, the multiplayer first person shooting game. <br>
The requirement of realtime pvp game is packet is small and frequently. It wants a minimal delay. And the worst delay should be not so worse. <br>
The test client send 500 bytes in every 50 milliseconds. And the server send it back after receiving immediately. <br>

#### Followwing framework will be test,
* [UDT](https://sourceforge.net/projects/udt/)    -   UDP-based Data Transfer Protocol
* [kcp](https://github.com/skywind3000/kcp)    -   A Fast and Reliable ARQ Protocol
* [enet](http://enet.bespin.org/index.html)   -   Reliable UDP networking library

You can see the whole bench mark test result in [bench_mark.md](https://github.com/libinzhangyuan/reliable_udp_bench_mark/blob/master/bench_mark.md).
