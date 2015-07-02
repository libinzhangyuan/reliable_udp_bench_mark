#include <iostream>
#include <cstring>
#include <signal.h>
#include <cstdlib>
#include <sys/time.h>
#include <thread>
#include <chrono>
#include <sstream>
#include <algorithm>
#include <time.h>

#include "test_util.h"
#include "udt_client.h"

std::vector<int> recv_package_interval_;
std::vector<int> recv_package_interval10_;

static size_t test_str_size = 1460;

#define CLOCK_INTERVAL_STR "_"
msg_ptr_t make_test_str(size_t test_str_size)
{
    std::ostringstream ostr;
    ostr << iclock64();
    std::string msg_str = ostr.str();
    msg_str += test_str(CLOCK_INTERVAL_STR, test_str_size - msg_str.size());
    return msg_ptr_t(new std::string(msg_str));
}

uint64_t get_time_from_msg(const std::string& msg)
{
    std::size_t pos = msg.find(CLOCK_INTERVAL_STR);
    if (pos == std::string::npos)
    {
        std::cout << "wrong msg: " << msg << std::endl;
        return 0;
    }
    const std::string& time_str = msg.substr(0, pos);
    return std::atoll(time_str.c_str());
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

void handle_one_msg(const std::string& msg, const std::string& total_recved_str)
{
    uint64_t send_time = get_time_from_msg(msg);
    uint64_t cur_time = iclock64();

    static uint64_t static_last_refresh_time = 0;
    static size_t static_recved_bytes = 0;
    static_recved_bytes += msg.size();

    {
        static size_t static_good_recv_count = 0;

        static_good_recv_count++;

        uint64_t interval = cur_time - send_time;
        recv_package_interval_.push_back(interval);
        recv_package_interval10_.push_back(interval);
        //std::cout << interval << ":" total_recved_str.size() << "\t";
        std::cout << interval << "\t";

        if (static_good_recv_count % 10 == 0)
        {
            int average10 = 0;
            for (int x : recv_package_interval10_)
                average10 += x;
            average10 = (average10 / 10);

            int average_total = 0;
            for (int x: recv_package_interval_)
                average_total += x;
            average_total = average_total / recv_package_interval_.size();

            std::cout << "max: " << *std::max_element( recv_package_interval10_.begin(), recv_package_interval10_.end() ) <<
                "  average 10: " << average10 <<
                "  average total: " << average_total;
            if (cur_time - static_last_refresh_time > 10 * 1000)
            {
                std::cout << " " << static_cast<double>(static_recved_bytes * 10 / (cur_time - static_last_refresh_time)) / 10 << "KB/s(in)";
                static_last_refresh_time = cur_time;
                static_recved_bytes = 0;
            }
            std::cout << std::endl;
            std::cout << get_cur_time_str() << " ";
            recv_package_interval10_.clear();
        }
        std::cout.flush();
    }
}

void recv_func(const std::shared_ptr<std::string>& str)
{
    static std::string recved_str;
    recved_str += *str;
    while (recved_str.size() >= test_str_size)
    {
        size_t len_for_check = recved_str.size();
        const std::string& msg = recved_str.substr(0, test_str_size);
        recved_str.erase(0, test_str_size);
        if (len_for_check - recved_str.size() != test_str_size)
            std::cout << "split msg error" << std::endl;
        handle_one_msg(msg, recved_str);
    }
}

int main(int argc, char* argv[])
{
    try
    {
        // Check command line arguments.
        if (argc != 4 && argc != 5)
        {
            std::cerr << "Usage: battle_client_udt <local_port> <serv_address> <serv_port> <test_str_size>\n";
            std::cerr << "  For IPv4, try:\n";
            std::cerr << "    client_udt 12345 120.123,2.123 80 1460\n";
            return 1;
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "exception: " << e.what() << "\n";
    }

	//ignore SIGPIPE
	sigset_t ps;
	sigemptyset(&ps);
	sigaddset(&ps, SIGPIPE);
	pthread_sigmask(SIG_BLOCK, &ps, NULL);

    if (argc == 5)
        test_str_size = std::atoi(argv[4]);

    auto client = std::make_shared<UDTClient>(std::atoi(argv[1]), argv[2], std::atoi(argv[3]), recv_func);
    int start_ret = client->Start();
    if (start_ret == 0)
        return 0;

    while (true)
    {
        msg_ptr_t msg = make_test_str(test_str_size);
        client->SendMsg(msg);

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        size_t buf_size = client->SendMsgBufSize();
        while (buf_size > 500 * 50)
        {
            std::cerr << "B";
            std::cerr.flush();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            buf_size = client->SendMsgBufSize();
        }
    }

    return 1;
}
