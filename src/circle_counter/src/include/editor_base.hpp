#pragma once

#include <atomic>
#include <vector>

#include <opencv2/core/types.hpp>

#include <opencv2/highgui.hpp>

namespace cc {
    namespace types::callback_container {
        struct mouse {
            std::vector<cv::Point2i>* _p_edited_points;
            std::atomic_bool*         _p_is_updated;
        };
    }

    class EditorBase {
    public:
        void editor() {
            cv::setMouseCallback("Circle Counter", &mouse_callback, &_mouse_container);
        }

    protected:
        types::callback_container::mouse _mouse_container;

    private:
        static void mouse_callback(const int event, const int x, const int y, int, void* container) {
            if (event != cv::EVENT_LBUTTONDOWN) return;
            auto mouse{static_cast<types::callback_container::mouse*>(container)};
            mouse->_p_edited_points->push_back(cv::Point2i(x, y));
            mouse->_p_is_updated->store(false);
        }
    };
}
