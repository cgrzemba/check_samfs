/*
 *    SAM-QFS_notice_begin
 *
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at pkg/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at pkg/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright (c) 2010, 2015, Oracle and/or its affiliates. All rights reserved.
 *
 *    SAM-QFS_notice_end
 */

#ifndef SAM_VERSION_H
#define	SAM_VERSION_H

#ifdef  __cplusplus
extern "C" {
#endif

#define	SAM_NAME "SAM-FS"
#define	SAM_MAJORV "6.1"
#define	SAM_MAJORV_NUM 61
#define	SAM_MINORV "0"
#define	SAM_FIXV "18"
#define	SAM_VERSION SAM_MAJORV "." SAM_FIXV
#define	SAM_PREV_MAJORV "6.0"
#define	SAM_BUILD_INFO SAM_VERSION ", t2k-brm-10 2016-03-24 23:26:02"
#define	SAM_BUILD_UNAME "SunOS t2k-brm-10 5.11 11.0 sun4v sparc SUNW,Sun-Fire-T200"
/* the sam_license_file is relative to the SAMPATH */
#define	 SAM_LICENSE_FILE    "LICENSE." SAM_MAJORV

#ifdef  __cplusplus
}
#endif

#endif /* SAM_VERSION_H */
