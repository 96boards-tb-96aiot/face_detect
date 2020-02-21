/****************************************************************************
*
*    Copyright (c) 2017 - 2019 by Rockchip Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Rockchip Corporation. This is proprietary information owned by
*    Rockchip Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Rockchip Corporation.
*
*****************************************************************************/

#ifndef _ROCKFACE_H
#define _ROCKFACE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief rockface handle
 */
typedef void *rockface_handle_t;

/**
 * @brief 函数返回值
 */
typedef enum {
    ROCKFACE_RET_SUCCESS = 0,      ///< 成功
    ROCKFACE_RET_FAIL = -1,        ///< 失败
    ROCKFACE_RET_PARAM_ERR = -2,   ///< 输入参数错误
    ROCKFACE_RET_AUTH_FAIL = -99,  ///< 授权失败
    ROCKFACE_RET_NOT_SUPPORT = -98 ///< 不支持设备
} rockface_ret_t;

/**
 * @brief 图像像素格式
 */
typedef enum {
    ROCKFACE_PIXEL_FORMAT_GRAY8 = 0,       ///< Gray8
    ROCKFACE_PIXEL_FORMAT_RGB888,          ///< RGB888
    ROCKFACE_PIXEL_FORMAT_BGR888,          ///< BGR888
    ROCKFACE_PIXEL_FORMAT_RGBA8888,        ///< RGBA8888
    ROCKFACE_PIXEL_FORMAT_BGRA8888,        ///< BGRA8888
    ROCKFACE_PIXEL_FORMAT_YUV420P_YU12,    ///< YUV420P YU12: YYYYYYYYUUVV
    ROCKFACE_PIXEL_FORMAT_YUV420P_YV12,    ///< YUV420P YV12: YYYYYYYYVVUU
    ROCKFACE_PIXEL_FORMAT_YUV420SP_NV12,   ///< YUV420SP NV12: YYYYYYYYUVUV
    ROCKFACE_PIXEL_FORMAT_YUV420SP_NV21,   ///< YUV420SP NV21: YYYYYYYYVUVU
    ROCKFACE_PIXEL_FORMAT_YUV422P_YU16,    ///< YUV422P YU16: YYYYYYYYUUUUVVVV
    ROCKFACE_PIXEL_FORMAT_YUV422P_YV16,    ///< YUV422P YV16: YYYYYYYYVVVVUUUU
    ROCKFACE_PIXEL_FORMAT_YUV422SP_NV16,   ///< YUV422SP NV16: YYYYYYYYUVUVUVUV
    ROCKFACE_PIXEL_FORMAT_YUV422SP_NV61,   ///< YUV422SP NV61: YYYYYYYYVUVUVUVU
    ROCKFACE_PIXEL_FORMAT_MAX,
} rockface_pixel_format;

/**
 * @brief Image Rotate Mode
 */
typedef enum {
    ROCKFACE_IMAGE_TRANSFORM_NONE              = 0x00,  ///< Do not transform
    ROCKFACE_IMAGE_TRANSFORM_FLIP_H            = 0x01,  ///< Flip image horizontally
    ROCKFACE_IMAGE_TRANSFORM_FLIP_V            = 0x02,  ///< Flip image vertically
    ROCKFACE_IMAGE_TRANSFORM_ROTATE_90         = 0x04,  ///< Rotate image 90 degree
    ROCKFACE_IMAGE_TRANSFORM_ROTATE_180        = 0x03,  ///< Rotate image 180 degree
    ROCKFACE_IMAGE_TRANSFORM_ROTATE_270        = 0x07,  ///< Rotate image 270 defree
} rockface_image_transform_mode;

/**
 * @brief 表示二维图像上人脸的矩形区域
 */
typedef struct rockface_rect_t {
    int left;       ///< 矩形最左边的坐标
    int top;        ///< 矩形最上边的坐标
    int right;      ///< 矩形最右边的坐标
    int bottom;     ///< 矩形最下边的坐标
} rockface_rect_t;

/**
 * @brief 表示一个二维图像
 */
typedef struct rockface_image_t {
    uint8_t *data;                          ///< 图像数据
    uint32_t size;                          ///< 图像数据大小
    uint8_t is_prealloc_buf;                ///< 图像数据是否已预分配内存
    rockface_pixel_format pixel_format;     ///< 图像像素格式 (@ref rockface_pixel_format)
    uint32_t width;                         ///< 图像宽
    uint32_t height;                        ///< 图像高
} rockface_image_t;

/**
 * @brief 表示一个检测的人脸
 */
typedef struct rockface_det_t {
    int id;                     ///< 跟踪ID
    int reserve;                ///< 保留字段
    rockface_rect_t box;        ///< 人脸区域
    float score;                ///< 人脸分数
} rockface_det_t;

/**
 * @brief 表示检测到人脸的数组
 */
typedef struct rockface_det_array_t {
    int count;                     ///< 数组大小 (0 <= count < 128)
    rockface_det_t face[128];      ///< 人脸数组
} rockface_det_array_t;

/**
 * 创建Handle
 * 
 * @return @ref rockface_handle_t 
 */
rockface_handle_t rockface_create_handle();

/**
 * 释放Handle
 * 
 * @param handle 需要释放的Handle
 * @return @ref rockface_ret_t 
 */
rockface_ret_t rockface_release_handle(rockface_handle_t handle);

/**
 * 设置数据文件路径（如果所有的data文件与librockface.so放在相同目录下，可以不需要设置）
 * 
 * @param handle 需要设置的Handle
 * @param data_path 数据文件的路径
 * @return @ref rockface_ret_t
 */
rockface_ret_t rockface_set_data_path(rockface_handle_t handle, const char* data_path);

/**
 * 初始化人脸检测器
 * 
 * @param handle 需要初始化的Handle
 * @return @ref rockface_ret_t 
 */
rockface_ret_t rockface_init_detector(rockface_handle_t handle);


/**
 * 人脸检测
 * 
 * @param handle [in] 已初始化的Handle（调用 @ref rockface_init_detector 函数初始化）
 * @param in_img [in] 输入图像
 * @param face_array [out] 人脸检测结果
 * @return @ref rockface_ret_t
 */
rockface_ret_t rockface_detect(rockface_handle_t handle, rockface_image_t *in_img, rockface_det_array_t *face_array);



#ifdef __cplusplus
} //extern "C"
#endif

#endif // _ROCKFACE_H
