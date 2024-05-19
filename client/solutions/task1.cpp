#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

int main() {
    int N, K;
    std::cin >> N >> K;

    std::vector<int> tastiness(N);
    for (int i = 0; i < N; ++i) {
        std::cin >> tastiness[i];
    }

    // Sort the tastiness values in descending order
    std::sort(tastiness.begin(), tastiness.end(), std::greater<int>());

    // Get the K-th tastiest croissant (1-based index)
    int kthTastiest = tastiness[K - 1];

    // Write the result to the output file
    std::cout << kthTastiest << std::endl;

    return 0;
}
