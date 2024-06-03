#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>
#include <cmath>
#include <algorithm>
#include <atomic>
#include <windows.h>

using namespace std;

mutex mtx;

void echo(const string& str, int period, int duration) {
    auto start = chrono::high_resolution_clock::now();
    while (true) {
        this_thread::sleep_for(chrono::seconds(period));
        lock_guard<mutex> lock(mtx);
        cout << str << endl;
        auto now = chrono::high_resolution_clock::now();
        if (chrono::duration_cast<chrono::seconds>(now - start).count() >= duration) {
            break;
        }
    }
}

void dummy() {
    // Do nothing
}

int gcd(int a, int b) {
    while (b != 0) {
        int t = b;
        b = a % b;
        a = t;
    }
    return a;
}

void gcd_command(int x, int y) {
    lock_guard<mutex> lock(mtx);
    cout << "GCD of " << x << " and " << y << " is " << gcd(x, y) << endl;
}

int count_primes(int n) {
    vector<bool> is_prime(n + 1, true);
    is_prime[0] = is_prime[1] = false;
    for (int i = 2; i * i <= n; ++i) {
        if (is_prime[i]) {
            for (int j = i * i; j <= n; j += i) {
                is_prime[j] = false;
            }
        }
    }
    return count(is_prime.begin(), is_prime.end(), true);
}

void prime_command(int x) {
    lock_guard<mutex> lock(mtx);
    cout << "Number of primes <= " << x << " is " << count_primes(x) << endl;
}

void sum_worker(int start, int end, atomic<long long>& result) {
    for (int i = start; i <= end; ++i) {
        result += i;
    }
}

void sum_command(int x, int threads) {
    vector<thread> workers;
    atomic<long long> result(0);
    int chunk_size = x / threads;
    for (int i = 0; i < threads; ++i) {
        int start = i * chunk_size + 1;
        int end = (i == threads - 1) ? x : (i + 1) * chunk_size;
        workers.emplace_back(sum_worker, start, end, ref(result));
    }
    for (auto& worker : workers) {
        worker.join();
    }
    long long total = result % 1000000;
    lock_guard<mutex> lock(mtx);
    cout << "Sum of integers <= " << x << " % 1000000 is " << total << endl;
}

int main() {
    vector<thread> threads;

    // Echo command: echo abc -p 5 -d 50 -n 3
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back(echo, "abc", 5, 50);
    }

    // Sum command: sum 1000000 -m 4 -n 2
    for (int i = 0; i < 2; ++i) {
        threads.emplace_back(sum_command, 1000000, 4);
    }

    // GCD command: gcd 12 18
    threads.emplace_back(gcd_command, 12, 18);

    // Dummy command: dummy -n 100
    for (int i = 0; i < 100; ++i) {
        threads.emplace_back(dummy);
    }

    // Join threads that perform actual work
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    return 0;
}
