#include <iostream>

#include <fmt/format.h>

#include "config_handler.hpp"
#include "circle_counter.hpp"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fmt::print("Usage: {} <Config Path> <Image Path>\n", argv[0]);
        return 1;
    }

    auto config{cc::types::default_config};
    cc::ConfigHandler config_handler(argv[1], config);
    config_handler.load();

    cc::CircleCounter circle_counter(argv[2], &config_handler);
    circle_counter.load();
    circle_counter.display();
    circle_counter.console();
    circle_counter.editor();

    while (true) {
        if (!circle_counter.is_updated()) {
            circle_counter.process();
            circle_counter.visualize();
            circle_counter.update();
        }

        const auto user_input{cv::waitKey(30)};
        if (user_input == 'r') circle_counter.restart();
        if (user_input == 27) break;
    }

    return 0;
}
