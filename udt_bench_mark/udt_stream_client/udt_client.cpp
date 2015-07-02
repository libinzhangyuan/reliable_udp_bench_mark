#include <arpa/inet.h>
#include <udt.h>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <memory>
#include <algorithm>

#include "udt_client.h"
#include "test_util.h"


const int g_IP_Version = AF_INET;


UDTClient::UDTClient(int local_port, const std::string& ip_connect_to, int port_connect_to, recv_callback_func_t recv_func) :
    recv_callback_func_(recv_func),
    local_port_(local_port),
    ip_connect_to_(ip_connect_to),
    port_connect_to_(port_connect_to),
    sock_(UDT::INVALID_SOCK),
    udtbuf_recved_len_(0),
    udt_running_(0),
    udt_eid_(-1)
{
}

int UDTClient::Start(void)
{
    udt_running_ = 0;

    // create, bind
    //
    CreateSocket();

    // connect to the server, implict bind
    if (0 == Connect())
    {
        return 0;
    }

    // sleep 5 sencods for connect success.
    std::this_thread::sleep_for(std::chrono::seconds(5));

    udt_thread_ = std::thread(std::bind(&UDTClient::Run, shared_from_this()));
    return 1;
}

void UDTClient::SendMsg(msg_ptr_t msg)
{
    send_msg_buf_ += *msg;
}

size_t UDTClient::SendMsgBufSize(void)
{
    return send_msg_buf_.size();
}

std::deque<msg_ptr_t> UDTClient::RecvMsg(void) // todo: using move symatic
{
    return std::deque<msg_ptr_t>();
}

// return -1 means error. otherwise return 0
int UDTClient::CreateSocket(void)
{
    sock_ = UDT::socket(AF_INET, SOCK_STREAM, 0);
    //sock_ = UDT::socket(AF_INET, SOCK_DGRAM, 0);
    if (UDT::INVALID_SOCK == sock_)
    {
        std::cout << "UDT socket: " << UDT::getlasterror().getErrorCode() << ' ' << UDT::getlasterror().getErrorMessage();
        return -1;
    }

    // setopt
    {
        bool block = false;
        UDT::setsockopt(sock_, 0, UDT_SNDSYN, &block, sizeof(bool));
        UDT::setsockopt(sock_, 0, UDT_RCVSYN, &block, sizeof(bool));

        //UDT::setsockopt(sock_, 0, UDT_MSS, new int(1500), sizeof(int));

        //int snd_buf = 16000;// 8192;
        //int rcv_buf = 16000;//8192;
        //int snd_buf = 7300;// 1460 * 5
        int snd_buf = 934400;// 1460 * 5
        int rcv_buf = 934400;
        UDT::setsockopt(sock_, 0, UDT_SNDBUF, &snd_buf, sizeof(int));
        UDT::setsockopt(sock_, 0, UDT_RCVBUF, &rcv_buf, sizeof(int)); 


        //int fc = 4096;
        //UDT::setsockopt(sock_, 0, UDT_FC, &fc, sizeof(int));

        bool reuse = true;
        UDT::setsockopt(sock_, 0, UDT_REUSEADDR, &reuse, sizeof(bool));

        bool rendezvous = false;
        UDT::setsockopt(sock_, 0, UDT_RENDEZVOUS, &rendezvous, sizeof(bool));
    }

    // bind
    sockaddr_in my_addr;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(local_port_);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    std::memset(&(my_addr.sin_zero), '\0', 8);
    if (UDT::ERROR == UDT::bind(sock_, (sockaddr*)&my_addr, sizeof(my_addr)))
    {
        std::cout << "UDT bind: " << UDT::getlasterror().getErrorCode() << ' ' << UDT::getlasterror().getErrorMessage() << std::endl;
        return -1;
    }

    // success
	return 0;
}

void UDTClient::TrySendMsg(void)
{
    if (send_msg_buf_.empty())
        return;

    int send_len = DoSendMsg(send_msg_buf_);
    if (send_len > 0)
        send_msg_buf_.erase(0, send_len);
}

// return the actual send len
int UDTClient::DoSendMsg(const std::string& msg)
{
    int send_ret = UDT::send(sock_, msg.c_str(), msg.size(), 0);
    if (UDT::ERROR == send_ret)
    {
        CUDTException& lasterror = UDT::getlasterror();
        const int error_code = lasterror.getErrorCode();
        if (error_code == CUDTException::EASYNCSND)
        {
            std::cerr << "N"; std::cerr.flush();
            return 0;
        }

        std::cout << "UDT sendmsg: " << error_code << ' ' << lasterror.getErrorMessage() << std::endl;
        udt_running_ = 0;
        return 0;
    }
    return send_ret;
}

void UDTClient::DoRecvMsg(void)
{
    udtbuf_recved_len_ = 0;

    int recv_ret = 0;
    if (UDT::ERROR == (recv_ret = UDT::recv(sock_, udtbuf_, sizeof(udtbuf_), 0))) {
        CUDTException& lasterror = UDT::getlasterror();
        const int error_code = lasterror.getErrorCode();

        if (error_code == CUDTException::EASYNCRCV) // no data available for read.
        {
            return;
        }

        std::cout << "UDT recv: " << error_code << " " << lasterror.getErrorMessage() << std::endl;
        if (CUDTException::EINVPARAM == error_code || CUDTException::ECONNLOST == error_code || CUDTException::EINVSOCK == error_code) {
            udt_running_ = 0;
        }
        return;
    }

    if (recv_ret > 0) 
    {
        udtbuf_recved_len_ = recv_ret;
        if (recv_callback_func_)
        {
            std::shared_ptr<std::string> recved_str(new std::string(udtbuf_, udtbuf_recved_len_));
            recv_callback_func_(recved_str);
        }
        return;
    }

    std::cout << "UDT recv zero len" << std::endl;
    return;
}

int UDTClient::Connect(void)
{
    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_connect_to_);
    inet_pton(AF_INET, ip_connect_to_.c_str(), &serv_addr.sin_addr);
    std::memset(&(serv_addr.sin_zero), '\0', 8);
    std::cout << "connect to: " << ip_connect_to_ << " " << port_connect_to_ << std::endl;
    if (UDT::ERROR == UDT::connect(sock_, (sockaddr*)&serv_addr, sizeof(serv_addr)))
    {
        std::cout << "UDT connect: " << UDT::getlasterror().getErrorCode() << ' ' << UDT::getlasterror().getErrorMessage() << std::endl;
        return 0;
    }

    return 1;
}

void UDTClient::TryRecvMsg(void)
{
    DoRecvMsg();
}

int UDTClient::Run(void)
{
	udt_running_ = 1;

	std::cout << "Run UDT client loop ...\n";
	udt_running_ = 1;

	while (udt_running_) {
        TrySendMsg();
        TryRecvMsg();

        TrySendMsg();
        TryRecvMsg();

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}


	std::cout << "Close client ...";
	int close_state = UDT::close(sock_);
    if (close_state != 0)
        std::cout << "UDT close: " << UDT::getlasterror().getErrorCode() << ' ' << UDT::getlasterror().getErrorMessage() << std::endl;

	std::cout << "ok\n";

	pthread_detach(pthread_self());

    return 1;
}
