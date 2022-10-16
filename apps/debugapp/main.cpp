#include <bango/network/server.h>
#include <bango/network/client.h>

#include <atomic>
#include <thread>
#include <vector>
#include <memory>
#include <future>
#include <chrono>
#include <mutex>

//#define MULTI

using namespace bango::network;

static std::atomic<int> next_id{1};

class my_user : public writable, public authorizable {
public:
    int id;
    my_user(const std::shared_ptr<tacopie::tcp_client>& client) 
            : writable(client) {
        this->id = next_id++;
    }
};

int main()
{
    constexpr static char tested_packet_id = 30;

    std::atomic<int> non_synchronized_var{20};
    //int non_synchronized_var{20};  // uncomment when testing unsafe code
    std::mutex mx;
    server<my_user> serv;
    serv.set_nb_workers(40);
    serv.on_connected([](const std::shared_ptr<my_user>& user) {
        std::cout << "New user connected: " << user->id << std::endl;
    });
    serv.when(tested_packet_id, [&non_synchronized_var, &mx](const std::shared_ptr<my_user>& user, packet& pack) {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(20ms);
        non_synchronized_var++;
        int after = non_synchronized_var;
        std::chrono::time_point<std::chrono::system_clock> now = 
            std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        mx.lock();
        std::cout << "User id[" << user->id << "] Thread id[" << std::this_thread::get_id() << "] Time[" << millis << "] Incremented to " << after << std::endl;
        mx.unlock();
    } );
    serv.start("localhost", 6969);

#ifndef MULTI
    client clt;
    clt.connect("localhost", 6969);
    constexpr static size_t number_of_packets_to_be_sent_by_single_client = 8000;
    constexpr static size_t packet_data_size = 893;

    for (int i = 0; i < number_of_packets_to_be_sent_by_single_client; i++) {
        struct TT {
            char d[packet_data_size];
        };
        packet d(tested_packet_id);
        d << TT();
        clt.write(d);
    }

    std::cout << "Result: " << non_synchronized_var << std::endl;
    std::cin.get();
#else
    constexpr static size_t number_of_workers = 30;
    std::vector<std::unique_ptr<std::thread>> workers;
    std::vector<std::promise<void>> signals(number_of_workers);
    std::vector<client> clients{number_of_workers};
    for (int i = 0; i < number_of_workers; i++) {
        clients[i].connect("localhost", 6969);
    }

    constexpr static size_t number_of_packets_per_client = 50;
    for (int i = 0; i < number_of_workers; i++) {
        workers.emplace_back(std::make_unique<std::thread>([i, &signals, &clients]() {
            signals[i].get_future().get();
            for (int j = 0; j < number_of_packets_per_client; j++) {
                clients[i].write(packet(tested_packet_id));
            }
        }));
    }

    std::for_each(signals.begin(), signals.end(), [](auto& sig) { sig.set_value(); });
    std::for_each(workers.begin(), workers.end(), [](auto& thread) { thread->join(); });

    std::cout << "Result: " << non_synchronized_var << std::endl;
    std::cin.get();
    clients.clear();
#endif


    std::cout << "Result: " << non_synchronized_var << std::endl;
    return 0;    
}
