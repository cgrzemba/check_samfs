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
#include "pub/mgmt/faults.h"
#include "pub/mgmt/error.h"

#define MSG_BUFF_SIZE 512

int
main (int argc,char *argv[])
{
	int ret = 0;
	ctx_t ctx_d;
	int num_critical_faults = 0;
	int num_major_faults = 0;
	int num_minor_faults = 0;
	char log_msg[MSG_BUFF_SIZE];
        int bufspace = MSG_BUFF_SIZE;
	int cnt=0;
	
	
	
	sqm_lst_t		*lst = NULL;
	node_t 		    *node = NULL;
	fault_attr_t	*fault_attr = NULL;
	
	
 
	if (get_all_faults(&ctx_d, &lst) != 0) {
		/* samerrmsg and samerrno is already populated */
		fprintf(stderr, "get fault summary failed: %s", samerrmsg);
		return (3);
	}

	node = lst->head;
	while (node != NULL) {
		fault_attr = (fault_attr_t *)node->data;
		/* count of unacknowledged faults only */
		if (fault_attr->state == UNRESOLVED) {
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
		node = node->next;
	}
	lst_free_deep(lst);
	
	if ( num_major_faults == 0 && num_minor_faults == 0 &&  num_critical_faults == 0 )
	    printf( "SamFS Ok - no faults\n" );
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


