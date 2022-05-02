#pragma once

#include <atomic>
#include <vector>

#include <opencv2/core/types.hpp>

#include <opencv2/highgui.hpp>

namespace cc {
    namespace types {
        namespace bridge {
            struct mouse {
                std::vector<cv::Point2i>* _p_edited_points;
                std::atomic_bool*         _p_is_updated;
            };
        }

        namespace callback_container {
            using mouse = types::bridge::mouse;
        }
    }

    class EditorBase {
    public:
        void editor() {
            cv::setMouseCallback("Circle Counter", &mouse_callback, &_mouse_container);
        }

        void clear_editor() {
            _mouse_container._p_edited_points->clear();
            _mouse_container._p_is_updated->store(false);
        }

    protected:
        types::bridge::mouse _mouse_container;

    private:
        static void mouse_callback(const int event, const int x, const int y, int, void* container) {
            if (event != cv::EVENT_LBUTTONDOWN) return;
            auto mouse{static_cast<types::callback_container::mouse*>(container)};
            mouse->_p_edited_points->push_back(cv::Point2i(x, y));
            mouse->_p_is_updated->store(false);
        }
    };
}
