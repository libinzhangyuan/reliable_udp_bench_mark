#include <arpa/inet.h>
#include <udt.h>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/time.h>
#include <ctime>
#include <sstream>
#include <thread>

#include "test_util.h"
#include "udt_server.h"

const int g_IP_Version = AF_INET;
//const int g_Socket_Type = SOCK_STREAM;
const int g_Socket_Type = SOCK_DGRAM;

/* get system time */
static inline void itimeofday(long *sec, long *usec)
{
    struct timeval time;
    gettimeofday(&time, NULL);
    if (sec) *sec = time.tv_sec;
    if (usec) *usec = time.tv_usec;
}

/* get clock in millisecond 64 */
static inline uint64_t iclock64(void)
{
    long s, u;
    uint64_t value;
    itimeofday(&s, &u);
    value = ((uint64_t)s) * 1000 + (u / 1000);
    return value;
}


std::string get_cur_time_str()
{
    time_t tmpcal_ptr = {0};
    struct tm *tmp_ptr = NULL;
    tmpcal_ptr = time(NULL);
    tmp_ptr = localtime(&tmpcal_ptr);
    std::ostringstream osstrm;
    osstrm << tmp_ptr->tm_hour << ":" << tmp_ptr->tm_min << "." << tmp_ptr->tm_sec;
    return osstrm.str();
}



UDTServer::UDTServer(void) :
    listen_sock_(UDT::INVALID_SOCK)
{
}

// return -1 means error. otherwise return 0
int UDTServer::CreateListenSocket(int listen_port)
{
    listen_sock_ = UDT::socket(AF_INET, SOCK_STREAM, 0);
    //listen_sock_ = UDT::socket(AF_INET, SOCK_DGRAM, 0);
    if (UDT::INVALID_SOCK == listen_sock_)
    {
        std::cout << "UDT socket: " << UDT::getlasterror().getErrorCode() << ' ' << UDT::getlasterror().getErrorMessage();
        return -1;
    }

    // setopt
    {
        bool block = false;
        UDT::setsockopt(listen_sock_, 0, UDT_SNDSYN, &block, sizeof(bool));
        UDT::setsockopt(listen_sock_, 0, UDT_RCVSYN, &block, sizeof(bool));

        //UDT::setsockopt(listen_sock_, 0, UDT_MSS, new int(1500), sizeof(int)); // default is 1500

        int snd_buf = 934400;// 1460 * 64 = 93440
        int rcv_buf = 934400;
        UDT::setsockopt(listen_sock_, 0, UDT_SNDBUF, &snd_buf, sizeof(int)); // default is 10MB
        UDT::setsockopt(listen_sock_, 0, UDT_RCVBUF, &rcv_buf, sizeof(int)); // default is 10MB


        //int fc = 4096;
        //UDT::setsockopt(listen_sock_, 0, UDT_FC, &fc, sizeof(int));

        bool reuse = true;
        UDT::setsockopt(listen_sock_, 0, UDT_REUSEADDR, &reuse, sizeof(bool));

        bool rendezvous = false;
        UDT::setsockopt(listen_sock_, 0, UDT_RENDEZVOUS, &rendezvous, sizeof(bool));
    }

    // bind
    sockaddr_in my_addr;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(listen_port);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    std::memset(&(my_addr.sin_zero), '\0', 8);
    if (UDT::ERROR == UDT::bind(listen_sock_, (sockaddr*)&my_addr, sizeof(my_addr)))
    {
        std::cout << "UDT bind: " << UDT::getlasterror().getErrorCode() << ' ' << UDT::getlasterror().getErrorMessage() << std::endl;
        return -1;
    }

    // listen
    //
    if (UDT::ERROR == UDT::listen(listen_sock_, 20))
    {
        std::cout << "UDT listen: " << UDT::getlasterror().getErrorCode() << ' ' << UDT::getlasterror().getErrorMessage() << std::endl;
        return -1;
    }

    // success
	return 0;
}

void UDTServer::Run(int listen_port)
{
    // create, bind and listen
    //
	CreateListenSocket(listen_port);

    while (true)
    {
        struct sockaddr addr;
        int addrlen = sizeof(sockaddr);

        const UDTSOCKET accept_ret = UDT::accept(listen_sock_, &addr, &addrlen);
        if (accept_ret == UDT::INVALID_SOCK)
        {
            CUDTException& lasterror = UDT::getlasterror();
            const int error_code = lasterror.getErrorCode();

            if (error_code == CUDTException::EASYNCRCV)
            {
                {
                    std::cout << ".";
                    static time_t static_last_time = 0;
                    const time_t cur_time = std::time(NULL);
                    if (cur_time != static_last_time)
                    {
                        static_last_time = cur_time;
                        std::cout << std::endl << get_cur_time_str() << " ";
                    }
                    std::cout.flush();
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }

            std::cout << "UDT accept: " << error_code << " " <<  lasterror.getErrorMessage() << std::endl;
            break;
        }

        {
            const UDTSOCKET new_sock = accept_ret;
            std::thread connection_thread(std::bind(&UDTServer::WorkThread, shared_from_this(), new_sock));
            connection_thread.detach();
        }
    }




	std::cout << "Close server ...";
	int close_state = UDT::close(listen_sock_);
    if (close_state != 0)
        std::cout << "UDT close:" << UDT::getlasterror().getErrorCode() << ' ' << UDT::getlasterror().getErrorMessage() << std::endl;

	std::cout << "ok\n";
}

void UDTServer::WorkThread(const UDTSOCKET& sock)
{
    std::string msg_buf_need_send;
	while (true)
    {
        // recv msg
        TryRecvMsg(sock, msg_buf_need_send);

        // send msg.
        if (msg_buf_need_send.size() > 0)
            TrySendMsg(sock, msg_buf_need_send);

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}


}

void UDTServer::TryRecvMsg(const UDTSOCKET& sock, std::string& msg_buf_need_send)
{
    do
    {
        const std::string recved_msg = DoRecvMsg(sock);
        if (recved_msg.size() > 0)
            msg_buf_need_send += recved_msg;
        else
            break;
    } while (true);
}

void UDTServer::TrySendMsg(const UDTSOCKET& sock, std::string& msg_buf_need_send)
{
    if (!msg_buf_need_send.empty())
    {
        const int send_ret = DoSendMsg(sock, msg_buf_need_send);
        if (send_ret > 0)
            msg_buf_need_send.erase(0, send_ret);
    }
}

// return the actual size of data that has been sent
int UDTServer::DoSendMsg(const UDTSOCKET& sock, const std::string& msg)
{
    //std::cout << "UDT client DoSendMsg: " << msg << std::endl;

    int send_ret = UDT::send(sock, msg.c_str(), msg.size(), 0);
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
        return 0;
    }
    if (static_cast<size_t>(send_ret) != msg.size())
    {
        std::cout << "UDT sendmsg: not all msg send!" << std::endl;
        return 0;
    }
    return send_ret;
}

std::string UDTServer::DoRecvMsg(const UDTSOCKET& sock)
{
    int recv_ret = 0;
    char recv_buf[1024 * 100];

    if (UDT::ERROR == (recv_ret = UDT::recv(sock, recv_buf, sizeof(recv_buf), 0)))
    {
        CUDTException& lasterror = UDT::getlasterror();
        const int error_code = lasterror.getErrorCode();

        if (error_code == CUDTException::EASYNCRCV) // no data available for read.
        {
            return "";
        }

        std::cout << "UDT recv: " << error_code << " " << lasterror.getErrorMessage() << std::endl;
        if (CUDTException::EINVPARAM == error_code || CUDTException::ECONNLOST == error_code || CUDTException::EINVSOCK == error_code) {
        }
        return "";
    }

    if (recv_ret > 0) 
    {
        std::string recved_str(recv_buf, recv_ret);
        return recved_str;
    }

    std::cout << "UDT recv msg with zero len" << std::endl;
    return "";
}


