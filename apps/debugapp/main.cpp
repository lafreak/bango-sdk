#include <bango/network/server.h>
#include <bango/network/client.h>

#include <atomic>
#include <thread>
#include <vector>
#include <memory>
#include <future>
#include <chrono>
#include <mutex>

#define MULTI

using namespace bango::network;

static std::atomic<int> next_id{1};

class my_user : public writable, public authorizable {
public:
    int id;
    // using writable::writable;
    my_user(const std::shared_ptr<tacopie::tcp_client>& client) 
            : writable(client) {
        this->id = next_id++;
    }
};

int main()
{
    std::atomic<int> non_synchronized_var{20};
    //int non_synchronized_var{20};
    std::mutex mx;
    bool work_started = false;
    server<my_user> serv;
    serv.set_nb_workers(40);
    serv.on_connected([](const std::shared_ptr<my_user>& user) {
        std::cout << "New user connected: " << user->id << std::endl;
    });
    serv.when(30, [&non_synchronized_var, &work_started, &mx](const std::shared_ptr<my_user>& user, packet& pack) {
        if (work_started) {
            //throw std::logic_error("bad");
        }
        work_started = true;
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
        if (!work_started) {
            //throw std::logic_error("bad");
        }
        work_started = false;
    } );
    serv.start("localhost", 6969);

#ifndef MULTI
    client clt;
    clt.connect("localhost", 6969);
    
    for (int i = 0; i < 8000; i++) {
        struct TT {
            char d[893];
        };
        packet d(30);
        d << TT();
        clt.write(d);
    }

    std::cout << "Result: " << non_synchronized_var << std::endl;
    std::cin.get();
#else
    const size_t numberOfWorkers = 30;
    std::vector<std::unique_ptr<std::thread>> workers;
    std::vector<std::promise<void>> signals(numberOfWorkers);
    std::vector<client> clients{numberOfWorkers};
    for (int i = 0; i < numberOfWorkers; i++) {
        clients[i].connect("localhost", 6969);
    }

    for (int i = 0; i < numberOfWorkers; i++) {
        workers.emplace_back(std::make_unique<std::thread>([i, &signals, &clients]() {
            signals[i].get_future().get();
            for (int j = 0; j < 50; j++) {
                clients[i].write(packet(30));
            }
            // for (int j = 0; j < 50; j++) {
            //     client cc;
            //     cc.connect("localhost", 6969);
            //     cc.write(packet(30));
            // }
            
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
