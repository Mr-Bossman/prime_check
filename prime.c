#include <gmpxx.h>
#include <stdlib.h>
#include <iostream>
#include <ctime>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
time_t start;
bool verbose = false;
int THREADS = 0;
int max = INT_MAX;
std::mutex coutMutex;
std::atomic<bool> kill (false);
std::atomic<int> total (1);
std::vector<mpz_class> r;
void prime(int id){
    uint64_t i = id*2;
    while(1){
        if(kill) break;
        int p = mpz_probab_prime_p(r[id].get_mpz_t(),50);
        double diff = difftime(time(0),start);
        while(!coutMutex.try_lock());
        if(!verbose){
            std::cout << "\raverage check time: " << diff/(total)<< "                                        ";
        }
        if(p || verbose)
        std::cout << ((!verbose)?'\r':'\0')<< "Offset " << i <<((p)?" is prime, on thread: ":" is not prime, on thread: ") << id << ",with an average check time: " << diff/(total) << '\n';
        coutMutex.unlock();
        if(i > max){
            kill = true;
            break;
        }
        mpz_add_ui(r[id].get_mpz_t(),r[id].get_mpz_t(),THREADS*2);
        i += THREADS*2;
        total += 1;
    }
}
int main(int argc,char** argv)
{
    if(!(argc == 3 || argc == 4 || argc == 5)){
        std::cout << "threads power max verbose\n";
        std::cout << "5 threads, two to the power of 6, max check offset of 10000 verbose\n";
        std::cout << "./prime 5 6 10000 v\n";
        std::cout << "5 threads, two to the power of 6, max check offset of 10000, not verbose\n";
        std::cout << "./prime 5 6 10000\n";
        std::cout << "5 threads, two to the power of 6, no max check offset, not verbose\n";
        std::cout << "./prime 5 6\n";
        std::cout << "5 threads, two to the power of 6, no max check offset, verbose\n";
        std::cout << "./prime 5 6 v\n";
        exit(-1);
    }
    if(argc == 4){
        if(argc != 5 && argv[3][0] ==  'v')
            verbose = true;
        else 
            max = atoi(argv[3]);
    }
    if(argc == 5){
        if(argv[4][0] ==  'v')
        verbose = true;
    }
    THREADS = atoi(argv[1]);

    std::vector<std::thread> threads;
    mpz_class s;
    mpz_ui_pow_ui (s.get_mpz_t(), 2, atoi(argv[2]));
    mpz_add_ui(s.get_mpz_t(),s.get_mpz_t(),1);
    start = time(0);
    std::cout << "Start time: " << ctime(&start) << '\n';
    r.resize(THREADS);
    for (int i=0; i<THREADS; i++) {
        mpz_add_ui(r[i].get_mpz_t(),s.get_mpz_t(),i*2);
        threads.push_back(std::thread(prime,i));
    }
    while(1){
        for (auto& th : threads) {

            if(kill){
                for (auto& t : threads) {
                    t.join();
                }
                exit(0);
            };
        }
    }

}