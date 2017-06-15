/************************************************************************
 *
 * Main File check_samfs
 * Written By: Carsten Grzemba (grzemba@contac-dt.de)
 * Last Modified: 11-05-2012
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

#pragma ident	"$Revision: 1.00 $"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>

#include "pub/mgmt/faults.h"
#include "pub/mgmt/error.h"

#define MSG_BUFF_SIZE 512
#define MAX_X_PARAMS 256
char *excludeList[MAX_X_PARAMS];
char **excludeLptr = excludeList;

void appendExcludeList(char* pattern)
{
    char *lptr = NULL;
    if (*excludeLptr == NULL) {
        *excludeLptr = pattern;
        if (excludeLptr < &excludeList[MAX_X_PARAMS])
            excludeLptr += 1;
        else {
            fprintf(stderr, "too many parameters");
            exit (1);
        }
    }           
}


int
main (int argc,char **argv)
{
    int ret = 3,fdis;
    ctx_t ctx_d;
    int num_critical_faults = 0;
    int num_major_faults = 0;
    int num_minor_faults = 0;
    char log_msg[MSG_BUFF_SIZE];
    int bufspace = MSG_BUFF_SIZE;
    int cnt=0;
    extern char *optarg;
    extern int optind;
    int c, i, ignore=0;
    
    sqm_lst_t		*lst = NULL;
    node_t 		    *node = NULL;
    fault_attr_t	*fault_attr = NULL;
    char key[20];
    char value[256];
    
    while ((c = getopt(argc, argv, "x:")) != -1)
        switch (c){
          case 'x':
            appendExcludeList(optarg);
            break;
          case '?':
            if (optopt == 'f')
              fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint (optopt))
              fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            else
              fprintf (stderr,
                       "Unknown option character `\\x%x'.\n",
                       optopt);
            return 1;
          }
    
    
    
    if ((fdis=open(FAULTLOG,O_RDONLY)) < 0) {
        if (errno == ENOENT) { 
            /* clear all faults removes the whole file, so no file is ok */
            printf( "SamFS Ok - no faults, no file %s\n", FAULTLOG);
            return (0);

        }
        fprintf(stderr,"access %s failed: ",FAULTLOG,errno);perror("");
        return (3);
    }
    close(fdis);
    
    if (get_all_faults(&ctx_d, &lst) != 0) {
        /* samerrmsg and samerrno is already populated */
        fprintf(stderr, "get fault summary failed: %s\n", samerrmsg);
        return (3);
    }
    
    node = lst->head;
    while (node != NULL) {
        fault_attr = (fault_attr_t *)node->data;
        node = node->next;
        /* count of unacknowledged faults only */
        if (fault_attr->state == UNRESOLVED) {
            for (i=0; &excludeList[i] < excludeLptr; i++){
                ignore = 0;
                if (strncmp(excludeList[i], fault_attr->msg, strlen(excludeList[i])) == 0){
                    ignore = 1;
                    break;
                }
            }
            if (ignore) continue;
            
            if (fault_attr->errorType == 0) {
                /* critical fault */
                num_critical_faults++;
            } else if (fault_attr->errorType == 1) {
                /* major fault */
                num_major_faults++;
            } else {
                /* minor fault */
                num_minor_faults++;
            }
            if (bufspace > 0) {
                cnt += snprintf(&log_msg[cnt],bufspace,"[%s] %s; ", fault_attr->compID, fault_attr->msg);
                bufspace -= cnt;
            }
        }
    }
    lst_free_deep(lst);
    
    if ( num_major_faults == 0 && num_minor_faults == 0 &&  num_critical_faults == 0 ){
        ret = 0;
        printf( "SamFS Ok - no faults\n" );
        return (ret);
    }
    if ( num_minor_faults > 0){
        ret = 1;
        printf("SamFS Warning - minor faults=%d; %s\n", num_minor_faults, log_msg);
    }        
    if ( num_major_faults > 0 || num_critical_faults > 0 ) {
        ret = 2;
        printf("SamFS Critical - critical faults=%d; major_faults=%d\n", num_critical_faults, num_major_faults);
    }
    return (ret);	 /* 0 ok, 1 warnig, 2 critical, 3 unknown */
    
}


