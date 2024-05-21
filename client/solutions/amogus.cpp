#include <iostream>
#include <cmath>

// Function to check if a number is prime
bool is_prime(int n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
}

// Function to find two prime numbers that sum up to an even number N
std::pair<int, int> prime_sum_to_even(int N) {
    for (int i = 2; i <= N / 2; ++i) {
        if (is_prime(i) && is_prime(N - i)) {
            return std::make_pair(i, N - i);
        }
    }
    return std::make_pair(-1, -1); // Should never happen for valid input
}

int main() {
    int even_number;
    std::cin >> even_number;

    std::pair<int, int> result = prime_sum_to_even(even_number);
    if (result.first != -1) {
        std::cout << result.first << " " << result.second << std::endl;
    } else {
        std::cout << "No prime pair found." << std::endl;
    }

    return 0;
}
