/*
 * samrpc.h - SAM-FS user rpc library structures and definitions
 *
 * Definitions for SAM-FS user rpc library structures and definitions
 *
 */

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
 * Copyright (c) 2010, 2016, Oracle and/or its affiliates. All rights reserved.
 *
 *    SAM-QFS_notice_end
 */
#ifndef SAMFS_H_RPCGEN
#define	SAMFS_H_RPCGEN

#include <rpc/rpc.h>
#include <sys/types.h>
#include <sys/time.h>
#ifndef SAM_STAT_H
#include "stat.h"
#endif

#ifdef  __cplusplus
extern "C" {
#endif

#define	MAX_VSN 32
#define	MAX_OPTS 256

/*
 * Following is recommended string length for containing the message-digest
 * from the 'sam_getdigest()' call.  See the 'sam_getdigest(3)' man page for
 * more information.
 */

#define	MAX_GETDIGEST_LENGTH 255

struct filecmd {
	char *filename;
	char *options;
};

struct statcmd {
	char *filename;
	int size;
};

struct digest_arguments {
	char *filename;
	int size;
};
typedef struct digest_arguments digest_arguments;

struct digest_result {
	int result;
	int size;
	char *digest;
};
typedef struct digest_result digest_result;

typedef struct filecmd filecmd;

typedef struct statcmd statcmd;

typedef struct sam_stat samstat_t;


struct sam_st {
	int result;
	samstat_t s;
};

typedef struct sam_st sam_st;
typedef struct sam_copy_s samcopy;

#define	xdr_uint_t	xdr_u_int
#define	xdr_time_t	xdr_long
#define	xdr_ushort_t	xdr_u_short
#define	xdr_ulong_t	xdr_u_long

#define	SamFS ((unsigned long)(0x20000002))
#define	SAMVERS ((unsigned long)(1))
#define	samstat ((unsigned long)(1))
extern  sam_st * samstat_1();
#define	samlstat ((unsigned long)(2))
extern  sam_st * samlstat_1();
#define	samarchive ((unsigned long)(3))
extern  int *samarchive_1();
#define	samrelease ((unsigned long)(4))
extern  int *samrelease_1();
#define	samstage ((unsigned long)(5))
extern  int *samstage_1();
#define	samsetfa ((unsigned long)(6))
extern  int *samsetfa_1();
#define	samsegment ((unsigned long)(7))
extern  int *samsegment_1();
#define	samssum ((unsigned long)(8))
extern  int *samssum_1();
#define	samgetdigest ((unsigned long)(9))
extern  digest_result *samgetdigest_1();
extern int samfs_1_freeresult();

/* the xdr functions */
extern bool_t xdr_filecmd();
extern bool_t xdr_statcmd();
extern bool_t xdr_samstat_t();
extern bool_t xdr_sam_st();
extern bool_t xdr_digest_arguments();
extern bool_t xdr_digest_result();

#ifndef	SAM_LIB
extern	CLIENT	*clnt;
#endif /* !SAM_LIB */

#define	PROGNAME	"samfs"
#define	SAMRPC_HOST	"samhost"

/* Functions. */
int sam_initrpc(char *rpchost);
int sam_initrpc_timeout(char *rpchost, int seconds);
int sam_closerpc(void);

int sam_settimeout(int seconds);

#ifdef  __cplusplus
}
#endif

#endif /* SAMFS_H_RPCGEN */
