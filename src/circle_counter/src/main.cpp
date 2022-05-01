#include <iostream>

#include "config_handler.hpp"
#include "circle_counter.hpp"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <Config Path> <Image Path>\n";
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
        switch (user_input) {
        case 27:
            return 0;
        case 'r':
            circle_counter.restart();
            break;
        case 's':
            circle_counter.snapshot();
            break;
        }
    }

    return 0;
}
