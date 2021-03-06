#include <iostream>

#include "ringbuffer.hpp"

int main() {
    ubn::ringbuffer<std::size_t, 3> rb;
    
    for(std::size_t i = 1; i != 10; ++i) {
        if (rb.push_head(i)) {
            std::cout << "get tail from filled ringbuffer " << rb.catch_tail() << " while pusing -> " << i << std::endl;
        } else {
            std::cout << "waiting ringbuffer to be filled -> " << i << std::endl;
        }
    }
}
