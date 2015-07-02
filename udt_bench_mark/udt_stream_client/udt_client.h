#ifndef __UDT_CLIENT_H_
#define __UDT_CLIENT_H_

#include <functional>
#include <queue>
#include <memory>
#include <thread>
#include <set>
#include <queue>

#include "threadsafe_queue.hpp"


typedef int UDTSOCKET;
typedef std::function<void(const std::shared_ptr<std::string>&)> recv_callback_func_t;
typedef std::shared_ptr<std::string> msg_ptr_t;

// the UDTClient will create another thread for UDT sending and recving.
class UDTClient : public std::enable_shared_from_this<UDTClient>
{
public:
    // when recv_func valid,
    //   recv_callback_func will be called by another thread when a package recved.  recv_callback_func should be thread safe.
    // when recv_func invalid,
    //   packages recved will stored in queue. You should call TryRecvMsg frequently (10 milliseconds for example) to get the recved msg.
    UDTClient(int local_port, const std::string& ip_connect_to, int port_connect_to, recv_callback_func_t recv_func = nullptr);
    int Start(void);

    void SendMsg(msg_ptr_t msg);
    size_t SendMsgBufSize(void);
    std::deque<msg_ptr_t> RecvMsg(void); // todo: using move symatic

private:
    void TrySendMsg(void);

    // return the actual send len
    int DoSendMsg(const std::string& msg);

    void TryRecvMsg(void);
    void DoRecvMsg(void);
    int Run(void);
    int CreateSocket(void);
    int Connect(void);

private:
    recv_callback_func_t recv_callback_func_;
    std::string send_msg_buf_;

    int local_port_;
    std::string ip_connect_to_;
    int port_connect_to_;

    std::thread udt_thread_;
    UDTSOCKET sock_;
    char udtbuf_[1024 * 100];
    size_t udtbuf_recved_len_;
    int udt_running_;
    int udt_eid_;
};

#endif
