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
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <getopt.h>

#include "rockface_control.h"
#include "rkisp_control.h"
#include "video_common.h"

static void *face_thread(void *arg)
{
    rockface_control_init();
    pthread_detach(pthread_self());
    pthread_exit(NULL);
}

static void *isp_thread(void *arg)
{
    rkisp_control_init();
    pthread_detach(pthread_self());
    pthread_exit(NULL);
}

int main(int argc, const char *argv[])
{
    pthread_t t_face, t_isp;

    if (pthread_create(&t_face, NULL, face_thread, NULL)) {
        printf("create face_thread fail!\n");
        return -1;
    }

    if (pthread_create(&t_isp, NULL, isp_thread, NULL)) {
        printf("create isp_thread fail!\n");
        return -1;
    }

    while (1)
        sleep(5);

    rkisp_control_exit();

    rockface_control_exit();

    return 0;
}
