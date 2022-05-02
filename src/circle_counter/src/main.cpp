#include <cctype>
#include <iostream>
#include <stdexcept>

#include "config_handler.hpp"
#include "circle_counter.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <Config Path(Optional)> <Image Path/Camera ID>\n"
            << "Keyboard Shortcuts:\n"
            << "    ESC - exit without saving config\n"
            << "    R/r - clear edited marks in editor\n"
            << "    S/s - save current image as snapshot\n"
            << "    C/c - reopen closed console window\n"
            << "    Q/q - exit and save current config\n";
        return 1;
    }

    cc::ConfigHandler config_handler(argc < 3 ? "etc/config.yaml" : argv[1]);
    try {
        config_handler.load();
    } catch (std::runtime_error& e) {
        std::cout << e.what();
        return 1;
    }

    cc::CircleCounter circle_counter(argv[argc - 1], &config_handler);
    circle_counter.load();
    circle_counter.display();
    circle_counter.console();
    circle_counter.editor();

    while (circle_counter.is_display_open()) {
        if (!circle_counter.is_updated()) {
            circle_counter.process();
            circle_counter.visualize();
            circle_counter.update();
        }

        const auto user_input{std::tolower(cv::waitKey(10))};
        if      (user_input == 27)  return 0;
        else if (user_input == 'r') circle_counter.clear_editor();
        else if (user_input == 's') circle_counter.snapshot();
        else if (user_input == 'c') circle_counter.console();
        else if (user_input == 'q') { config_handler.sync(); return 0; };
    }

    return 0;
}
