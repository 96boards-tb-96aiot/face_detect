/*
 * Copyright (C) 2019 Rockchip Electronics Co., Ltd.
 * author: Zhihua Wang, hogan.wang@rock-chips.com
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL), available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <stdio.h>
#include <stdbool.h>
#include <memory.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <pthread.h>
#include <errno.h>

#include "rockface_control.h"
#include "video_common.h"
#include "rkisp_control.h"

#define DEFAULT_FACE_NUMBER 1000
#define DEFAULT_FACE_PATH "/userdata"
#define FACE_SCORE 0.9
#define FACE_SCORE_REGISTER 0.9999
#define FACE_REGISTER_CNT 5
#define FACE_REAL_SCORE 0.9
#define LICENCE_PATH "/userdata/key.lic"
#define FACE_DATA_PATH "/usr/bin"
#define MIN_FACE_WIDTH(w) ((w) / 5)
#define CONVERT_RGB_WIDTH 640
#define CONVERT_IR_WIDTH 640

static void *g_face_data = NULL;
static int g_face_index = 0;
static int g_face_cnt = DEFAULT_FACE_NUMBER;

static rockface_handle_t face_handle;
static int g_total_cnt;

static pthread_t g_tid;
static bool g_run;
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_cond = PTHREAD_COND_INITIALIZER;
static bool g_flag;
static rockface_image_t g_rgb_img;
static rockface_det_t g_rgb_face;
static bo_t g_rgb_bo;
static int g_rgb_fd = -1;

static pthread_mutex_t g_ir_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_ir_cond = PTHREAD_COND_INITIALIZER;
static bo_t g_ir_bo;
static int g_ir_fd = -1;

static bool g_register = false;
static int g_register_cnt = 0;
static bool g_delete = false;

static rockface_det_t *get_max_face(rockface_det_array_t *face_array)
{
    rockface_det_t *max_face = NULL;
    if (face_array->count == 0)
        return NULL;

    for (int i = 0; i < face_array->count; i++) {
        rockface_det_t *cur_face = &(face_array->face[i]);
        if (max_face == NULL) {
            max_face = cur_face;
            continue;
        }
        int cur_face_box_area = (cur_face->box.right - cur_face->box.left) *
                                (cur_face->box.bottom - cur_face->box.top);
        int max_face_box_area = (max_face->box.right - max_face->box.left) *
                                (max_face->box.bottom - max_face->box.top);
        if (cur_face_box_area > max_face_box_area)
            max_face = cur_face;
    }

    return max_face;
}

static int _rockface_control_detect(rockface_image_t *image, rockface_det_t *out_face)
{
    rockface_ret_t ret;
    rockface_det_array_t face_array;

    memset(&face_array, 0, sizeof(rockface_det_array_t));
    memset(out_face, 0, sizeof(rockface_det_t));

    ret = rockface_detect(face_handle, image, &face_array);
    if (ret != ROCKFACE_RET_SUCCESS)
        return -1;

    rockface_det_t* face = get_max_face(&face_array);
    if (face == NULL || face->score < FACE_SCORE ||
        face->box.right - face->box.left < MIN_FACE_WIDTH(image->width) ||
        face->box.left < 0 || face->box.top < 0 ||
        face->box.right > image->width || face->box.bottom > image->height)
        return -1;

    memcpy(out_face, face, sizeof(rockface_det_t));
    return 0;
}

static int rockface_control_detect(void *ptr, int width, int height, rockface_pixel_format fmt,
                                   rockface_image_t *image, rockface_det_t *face)
{
    int ret;

    memset(face, 0, sizeof(rockface_det_t));
    memset(image, 0, sizeof(rockface_image_t));
    image->width = width;
    image->height = height;
    image->data = ptr;
    image->pixel_format = fmt;
    ret = _rockface_control_detect(image, face);
    if (face->score > FACE_SCORE) {
        int left, top, right, bottom;
        left = face->box.left;
        top = face->box.top;
        right = face->box.right;
        bottom = face->box.bottom;
        if (shadow_paint_box_cb)
            shadow_paint_box_cb(left, top, right, bottom);
        rkisp_control_expo_weights_90(left, top, right, bottom);
        printf("box = (%d %d %d %d) score = %f\n", face->box.left, face->box.top,
                face->box.right, face->box.bottom, face->score);
    } else {
        if (shadow_paint_box_cb)
            shadow_paint_box_cb(0, 0, 0, 0);
        rkisp_control_expo_weights_default();
    }

    return ret;
}

int rockface_control_convert(void *ptr, int width, int height, RgaSURF_FORMAT rga_fmt)
{
    rockface_ret_t ret;
    rockface_image_t image;
    rockface_det_t face;
    rga_info_t src, dst;
    rockface_pixel_format fmt;

    if (!g_run)
        return -1;

    if (rga_fmt == RK_FORMAT_YCbCr_420_SP)
        fmt = ROCKFACE_PIXEL_FORMAT_YUV420SP_NV12;
    else {
        printf("%s: unsupport rga fmt\n");
        return -1;
    }
    rockface_control_detect(ptr, width, height, fmt, &image, &face);

    if (!g_flag)
        return -1;

    return 0;
}

int rockface_control_init(void)
{
    rockface_ret_t ret;

    face_handle = rockface_create_handle();

    ret = rockface_set_data_path(face_handle, FACE_DATA_PATH);
    if (ret != ROCKFACE_RET_SUCCESS) {
        printf("%s: set data path error %d!\n", __func__, ret);
        return -1;
    }

    ret = rockface_init_detector(face_handle);
    if (ret != ROCKFACE_RET_SUCCESS) {
        printf("%s: init detector error %d!\n", __func__, ret);
        return -1;
    }

    g_run = true;
    return 0;
}

void rockface_control_exit(void)
{
    g_run = false;
    rockface_release_handle(face_handle);
}
