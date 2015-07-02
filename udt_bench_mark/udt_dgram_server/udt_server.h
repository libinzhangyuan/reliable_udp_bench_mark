#ifndef __UDT_SERVER_H_
#define __UDT_SERVER_H_

#include <queue>
#include <memory>

typedef int UDTSOCKET;

class UDTServer : public std::enable_shared_from_this<UDTServer>
{
public:
    UDTServer(void);
    void Run(int listen_port);

private:
    int CreateListenSocket(int port);
    void WorkThread(const UDTSOCKET& sock);

    void TryRecvMsg(const UDTSOCKET& sock, std::queue<std::string>& msg_queue_need_send);
    void TrySendMsg(const UDTSOCKET& sock, std::queue<std::string>& msg_queue_need_send);
    std::string DoRecvMsg(const UDTSOCKET& sock, bool& bHaveMsgStill);
    int DoSendOneMsg(const UDTSOCKET& sock, const std::string& msg);

private:
    UDTSOCKET listen_sock_;
    int udt_running_;
};

#endif
