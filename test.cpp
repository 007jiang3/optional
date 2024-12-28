#include "optional.hpp"
#include <iostream>
#include <string>
#include <optional>

int main(int argc, char const* argv[]) {
    nonstd::optional<int> a(1);
    nonstd::optional<std::string> b("hello");
    nonstd::optional<nonstd::nullopt_t> c;
    nonstd::optional<double> d(0.1);

    a == d;

    a == c;

    return 0;
}
