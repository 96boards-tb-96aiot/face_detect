cmake_minimum_required(VERSION 3.8.2)

project(face_detect)

set(SRC
    rockface_control.c
    rkisp_control.c
    rga_control.c
    video_common.c
    main.c
)

include_directories(${DRM_HEADER_DIR})

link_directories(.)
add_executable(face_detect ${SRC})

target_link_libraries(face_detect rkisp rkisp_api rockface_s rknn_api_s drm rga pthread)

install(TARGETS face_detect DESTINATION bin)
install(FILES face_detection.rknn DESTINATION bin)
install(FILES librockface_s.so DESTINATION lib)
install(FILES librknn_api_s.so DESTINATION lib)
