/************************************************************************
 *
 * Main File ack_samfaults
 * Written By: Carsten Grzemba (grzemba@contac-dt.de)
 * Last Modified: 11-09-2019
 *
 * # CDDL HEADER START
 * #
 * # The contents of this file are subject to the terms of the
 * # Common Development and Distribution License (the "License").
 * # You may not use this file except in compliance with the License.
 * #
 * # You can obtain a copy of the license at pkg/OPENSOLARIS.LICENSE
 * # or http://www.opensolaris.org/os/licensing.
 * # See the License for the specific language governing permissions
 * # and limitations under the License.
 * #
 * # When distributing Covered Code, include this CDDL HEADER in each
 * # file and include the License file at pkg/OPENSOLARIS.LICENSE.
 * # If applicable, add the following below this CDDL HEADER, with the
 * # fields enclosed by brackets "[]" replaced with your own identifying
 * # information: Portions Copyright [yyyy] [name of copyright owner]
 * #
 * # CDDL HEADER END
 ************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>

#include "pub/mgmt/faults.h"
#include "pub/mgmt/error.h"
#include "mgmt/util.h"

#define MSG_BUFF_SIZE 512
#define MAX_X_PARAMS 256

static int verbose = 0;
static int dryrun = 1;

/*
 * Read the faults from the FAULTLOG binary and map the file
 */
static int
read_faults(const char* fn, /* filename */
int oflags,	/* FAULTLOG access modes */
int prot,	/* mapping access modes */
int flags,	/* handling of mapped data */
void **mp,	/* return - pointer to mmaped area with faults */
size_t *size	/* return - size of mmaped file */
)
{
    int		fd;	/* the file descr. */
    struct	stat64	st;	/* struct for file info */
    struct	stat64	buf;	/* struct for file info */
    
    *mp = NULL;
    
    if ((strcmp(fn, FAULTLOG) == 0) && lstat64(fn, &buf) != 0) {
        /* If FAULTLOG does not exist, it is a clean system */
        fprintf(stderr, "No faults log %s on system\n", fn);
        return (2);
    }
    
    if ((fd = open64(fn, oflags)) < 0) {
    
        samerrno = SE_CANT_OPEN_FLOG;
        snprintf(samerrmsg, MAX_MSG_LEN,
            GetCustMsg(SE_CANT_OPEN_FLOG), FAULTLOG);
    
        fprintf(stderr, "get faults log %s failed: %s\n", fn, samerrmsg);
        return (4);
    }
    
    if ((strcmp(fn, FAULTLOG) != 0)) {
        if((ftruncate(fd, *size)!=0)) {
            samerrno = SE_CANT_OPEN_FLOG;
            snprintf(samerrmsg, MAX_MSG_LEN,
            GetCustMsg(SE_CANT_OPEN_FLOG), FAULTLOG);
            fprintf(stderr, "get faults log %s failed: %s\n", fn, samerrmsg);
            return (4);
        }
    }
    /* Stat the file for info to map */
    if (fstat64(fd, &st) != 0) {
    
        samerrno = SE_FSTAT_FAILED;
        snprintf(samerrmsg, MAX_MSG_LEN,
            GetCustMsg(SE_FSTAT_FAILED), FAULTLOG);
    
        fprintf(stderr, "get faults failed: %s\n", samerrmsg);
    
        close(fd);
        return (5);
    }
    *size = st.st_size;	/* size of memory mapped area */
    
    /*
     * Map in the entire file.
     * Solaris's MM takes care of the page-ins and outs.
     */
    *mp = mmap(NULL, *size, prot, flags, fd, 0);
    if (*mp == MAP_FAILED) {
        samerrno = SE_MMAP_FAILED;
        snprintf(samerrmsg, MAX_MSG_LEN,
            GetCustMsg(SE_MMAP_FAILED), FAULTLOG);
    
        fprintf(stderr, "get faults failed: %s\n", samerrmsg);
    
        close(fd);
        return (6);
    }
    
    close(fd);
    return (0);
}

int
persist_ack(void * mp, size_t size) {
	char tmpfilnam[MAXPATHLEN] = {0};
	FILE *tmpfile = NULL;
	fault_attr_t *fp = NULL;
	int fault_len = sizeof (fault_attr_t);
    int total_faults = size / sizeof (fault_attr_t);
	int fd = 0;
	void *dp =NULL;
    
	/* get a temporary file to writer faults to */
	if (mk_wc_path(FAULTLOG, tmpfilnam, sizeof (tmpfilnam)) != 0) {
		fprintf(stderr, "Unable to update faults: %s\n", samerrmsg);
		return (7);
	}

	fp = (fault_attr_t *)mp;

    if (read_faults(tmpfilnam, O_RDWR|O_CREAT, PROT_READ | PROT_WRITE, MAP_SHARED,
	    &dp, &size) != 0) {
        /* samerrmsg and samerrno is already populated */
        fprintf(stderr, "open new faults log %s failed: %s\n", tmpfilnam, strerror(errno));
        return (3);
    }
    total_faults = size / sizeof (fault_attr_t);
    memcpy(dp, mp, size); /* does the file copy */

	munmap(dp,size);
	fclose(tmpfile);

	munmap(mp,size);
	
	/* Now rename the temporary file to FAULTLOG */
	unlink(FAULTLOG);

	if (rename(tmpfilnam, FAULTLOG) < 0) {
		samerrno = SE_RENAME_FAILED;
		snprintf(samerrmsg, MAX_MSG_LEN, GetCustMsg(samerrno),
		    tmpfilnam, FAULTLOG);

		fprintf(stderr, "Delete faults failed: [%d] %s\n",
		    samerrno, samerrmsg);
		return (8);
	}

	fprintf(stderr, "ackknowlede the faults from log\n");
	return (0);
}

int
ack_fault(int sequenceno) {
    ctx_t ctx_d;
    void *mp = NULL;
    size_t size = 0;
    int total_faults = 0;
    long errorID[DEFAULTS_MAX];

    if (read_faults(FAULTLOG, O_RDWR, PROT_READ | PROT_WRITE, MAP_PRIVATE,
	    &mp, &size) != 0) {
        /* samerrmsg and samerrno is already populated */
        fprintf(stderr, "read faults failed: %s\n", strerror(errno));
        return (3);
    }
    total_faults = size / sizeof (fault_attr_t);

    if (verbose) {
        printf ("%11s %-12s %9s %25s %-20s %-12s %s\n",
            "errorID","compID","errorType","timestamp","hostname","state","description");
    }
    
    int i = 0, n;
    fault_attr_t *fp;
    for (fp =  (fault_attr_t *)mp, n = 0; n < total_faults; fp++, n++) {
        if (verbose || dryrun) {
            char tbuf[32];
            strftime(tbuf, 32,"%a %b %d %H:%M:%S %Y",localtime(&fp->timestamp));
            printf ("%11ld %-12s %9s %25s %-20s %-12s %s\n",  
                fp->errorID,
                fp->compID,
                (fp->errorType==0) ? "CRITICAL": (fp->errorType>1)?"MINOR":"MAJOR",
                tbuf,
                fp->hostname,
                fp->state == ACK? "ACKNOWLEDGED":"UNRESOLVED",
                fp->msg);
        }
        if (fp->state == UNRESOLVED) {
            errorID[i++] = fp->errorID;
            if (!dryrun) {
                fp->state = ACK;
                if (verbose) 
                    printf ("acknowledged %d\n", fp->errorID);
            }
            else
                printf ("dryrun: would acknowledged %d\n", fp->errorID);
        }
    }


    if (!dryrun && i > 0)
        persist_ack(mp, size);
    else
        munmap(mp, size);

    return (0);
}

int
main (int argc,char **argv)
{
    int ret = 3,fdis;
    int num_critical_faults = 0;
    int num_major_faults = 0;
    int num_minor_faults = 0;
    char log_msg[MSG_BUFF_SIZE];
    int bufspace = MSG_BUFF_SIZE;
    int cnt=0;
    extern char *optarg;
    extern int optind;
    int c, i, ignore=0, sequenceno = 0;
    char usagemsg[] = "usage: %s [-v][-a]\n\t-v verbose\n\t-a doit, without '-a' only list entries\n\tacknowledges faults in faultlog.bin\n";

    if (argc < 2) {
        printf(usagemsg, argv[0]);
        return 1;
    }
         
    while ((c = getopt(argc, argv, "i:hva")) != -1)
        switch (c){
            case 'i':
                sequenceno = atol(optarg);
                break;
            case 'v':
                verbose++;
                break;
            case 'a':
                dryrun = 0;
                break;
            case '?':
                if (optopt == 'i')
                  fprintf (stderr, "Option -%c requires an unsigned int argument.\n", optopt);
                else if (isprint (optopt))
                  fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                else
                  fprintf (stderr,
                           "Unknown option character `\\x%x'.\n",
                           optopt);
                return 1;
            case 'h':
                printf(usagemsg, argv[0]);
                return 1;
          }
    
    ret = ack_fault(sequenceno);
    
    return (ret);
    
}

