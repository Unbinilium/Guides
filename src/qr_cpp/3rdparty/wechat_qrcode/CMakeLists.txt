cmake_minimum_required(VERSION 3.12.0)

add_library(wechat_qrcode SHARED
  src/wechat_qrcode.cpp
  src/binarizermgr.cpp
  src/decodermgr.cpp
  src/imgsource.cpp
  src/detector/align.cpp
  src/detector/ssd_detector.cpp
  src/scale/super_scale.cpp
)
target_include_directories(wechat_qrcode PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

find_package(OpenCV REQUIRED)
target_link_libraries(wechat_qrcode PUBLIC ${OpenCV_LIBS})

file(GLOB zxing_src
  src/zxing/*.cpp
  src/zxing/*/*.cpp
  src/zxing/*/*/*.cpp
)
add_library(zxing SHARED ${zxing_src})
target_compile_definitions(zxing PRIVATE NO_ICONV=1)
target_link_libraries(zxing PUBLIC ${OpenCV_LIBS})
target_link_libraries(wechat_qrcode PUBLIC zxing)

set(wechat_qrcode_commit_hash "a8b69ccc738421293254aec5ddb38bd523503252")
set(hash_detect_caffemodel "238e2b2d6f3c18d6c3a30de0c31e23cf")
set(hash_detect_prototxt "6fb4976b32695f9f5c6305c19f12537d")
set(hash_sr_caffemodel "cbfcd60361a73beb8c583eea7e8e6664")
set(hash_sr_prototxt "69db99927a70df953b471daaba03fbef")

set(model_types caffemodel prototxt)
set(model_names detect sr)

foreach(model_name ${model_names})
  foreach(model_type ${model_types})
    file(DOWNLOAD
      "https://raw.githubusercontent.com/WeChatCV/opencv_3rdparty/${wechat_qrcode_commit_hash}/${model_name}.${model_type}"
      "${CMAKE_BINARY_DIR}/bin/${model_name}.${model_type}"
      STATUS res
    )
    if(NOT res)
      message(WARNING "WeChatQRCode: Can't get ${model_name} ${model_type} file for wechat qrcode.")
    endif()
  endforeach()
endforeach()
