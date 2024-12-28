#include "optional.hpp"
#include <iostream>
#include <string>
#include <optional>
#include <vector>

void test_in_place() {
    nonstd::optional<std::string> a;
    a.emplace("hello");
    std::cout << *a << std::endl;

    nonstd::optional<std::string> b(nonstd::in_place, "world");
    std::cout << *b << std::endl;

    nonstd::optional<std::vector<int>> c(nonstd::in_place, { 1, 2, 3 });
    for (auto i : *c) {
        std::cout << i << " ";
    }
}

int main(int argc, char const* argv[]) {
    test_in_place();

    return 0;
}
