/* CDDL HEADER START
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

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/param.h>
#include <libgen.h>
#include <unistd.h>
#include <strings.h>
#include <sys/varargs.h>
#include <sys/mman.h>

#include "pub/mgmt/faults.h"
#include "pub/mgmt/error.h"
#include "mgmt/config/common.h"
#include "sammgmt.h"

static int helper_get_faults(char *libname, equ_t eq, sqm_lst_t **faults_list);

/* malloc + error checking */
void *
mallocer(size_t size)
{

	void *m = calloc(1, size);

	if (NULL == m)
		setsamerr(SE_NO_MEM);
	return (m);
}

/*
 * mk_wc_path()
 *
 * Function to generate a path name for working copies of
 * files and creates the file.
 */
int
mk_wc_path(
	char *original, 	/* IN - path to original file */
	char *tmppath, 		/* IN/OUT - buffer to hold new file path */
	size_t buflen)		/* IN - length of buffer */
{
	char		*copypath;
	char		template[MAXPATHLEN+1];
	char		buf[MAXPATHLEN+1];
	char		*fname;
	int		ret;
	struct stat64	statbuf;

	if (ISNULL(original, tmppath)) {
		return (-1);
	}

	/* make sure target directory exists */
	ret = create_dir(NULL, default_tmpfile_dir);
	if (ret != 0) {
		return (ret);
	}

	ret = stat64(original, &statbuf);

	/*
	 * not an error if the original doesn't exist.  In this
	 * case, dummy up a mode to be used for the later mknod.
	 */
	if (ret != 0) {
		statbuf.st_mode = S_IFREG;
		statbuf.st_mode |= S_IRWXU|S_IRGRP|S_IROTH;
	}

	/* create the template name */
	strlcpy(buf, original, MAXPATHLEN+1);
	fname = basename(buf);
	snprintf(template, MAXPATHLEN+1, "%s/%s_XXXXXX",
	    default_tmpfile_dir, fname);

	copypath = mktemp(template);

	if (copypath == NULL) {
		return (-1);
	} else {
		strlcpy(tmppath, copypath, buflen);
	}

	/* make sure an old version isn't hanging around */
	unlink(tmppath);

	/* create the target file */
	ret = mknod(tmppath, statbuf.st_mode, 0);

	return (ret);
}

/*
 * create the directory dir. This function will not fail if the directory
 * already exists.
 */
int
create_dir(
	ctx_t	*ctx	/* ARGSUSED */,
	upath_t	dir)
{

	struct stat dir_stat;

	/* make the dir rwx by owner, and rx by other and group */
	static mode_t perms = 0 | S_IRWXU | S_IROTH | S_IXOTH |
	    S_IRGRP | S_IXGRP;

	if (ISNULL(dir)) {
		return (-1);
	}
	errno = 0;
	if (stat(dir, &dir_stat) == 0) {
		if (!S_ISDIR(dir_stat.st_mode)) {

			samerrno = SE_CREATE_DIR_FAILED;
			snprintf(samerrmsg, MAX_MSG_LEN,
			    GetCustMsg(SE_CREATE_DIR_FAILED), dir, "");
			strlcat(samerrmsg, strerror(ENOTDIR), MAX_MSG_LEN);
			fprintf(stderr, "creating directory failed: %s\n",
			    samerrmsg);

			return (-1);
		}
		fprintf(stderr, "directory %s already exists\n", dir);
		return (0);
	}

	if (errno == ENOENT) {
		errno = 0;
		if (mkdirp(dir, perms) == 0) {
			return (0);
		}
	}

	samerrno = SE_CREATE_DIR_FAILED;
	snprintf(samerrmsg, MAX_MSG_LEN,
	    GetCustMsg(SE_CREATE_DIR_FAILED), dir, "");
	strlcat(samerrmsg, strerror(errno), MAX_MSG_LEN);
	fprintf(stderr, "creating directory failed: %s\n", samerrmsg);
	return (-1);

}

/*
 * return 1 if an NULL argument is found in the list, 0 otherwise
 * set samerrno and samerrmsg accordingly.
 */
int
isnull(char *argnames, ...)
{

	va_list ap;
	char delim[] = ", ";
	char *crtname;
	char *names;
	char *rest;

	names = strdup(argnames);
	crtname = strtok_r(names, delim, &rest);
	va_start(ap, argnames);
	while (crtname) {
		if (NULL == va_arg(ap, void *)) {
			samerrno = SE_NULL_PARAMETER;
			snprintf(samerrmsg, MAX_MSG_LEN,
			    GetCustMsg(SE_NULL_PARAMETER), crtname);
			va_end(ap);
			free(names);
			return (1);
		}
		crtname = strtok_r(NULL, delim, &rest);
	}
	free(names);
	va_end(ap);
	return (0);
}

void
lst_free(
sqm_lst_t *lst)	/* free this list (but not the data) */
{

	node_t *node, *rmnode;

	if (lst == NULL)
		return;
	node = lst->head;
	while (node != NULL) {
		rmnode = node;
		node = node->next;
		free(rmnode);
	}
	free(lst);
}


void
lst_free_deep(
sqm_lst_t *lst)	/* free this list and its data */
{

	node_t *node;

	if (lst == NULL)
		return;
	node = lst->head;
	while (node != NULL) {
		if (node->data != NULL) {
			free(node->data);
		}
		node = node->next;
	}
	lst_free(lst);
}

sqm_lst_t *
lst_create(void)
{

	sqm_lst_t *lstp = (sqm_lst_t *)mallocer(sizeof (sqm_lst_t));

	if (NULL != lstp) {
		lstp->length = 0;
		lstp->head = lstp->tail = NULL;
	}
	return (lstp);
}

int
lst_append(
sqm_lst_t *lst,	/* append to this list */
void *data)	/* data stored in the new node */
{

	node_t *new_node;

	if (ISNULL(lst, data))
		return (-1);
	if (NULL == (new_node = (node_t *)mallocer(sizeof (node_t))))
		return (-1);
	new_node->data = (void *)data;
	new_node->next = NULL;
	if (lst->length == 0)
		lst->head = new_node;
	else
		lst->tail->next = new_node;
	lst->tail = new_node;
	lst->length++;
	return (0);
}

/*
 * get all faults
 */
int
get_all_faults(
ctx_t	*ctx,		/* ARGSUSED */
sqm_lst_t **faults_list)	/* return - list of faults */
{

	int ret = -1;

	ret = helper_get_faults(
	    "", /* don't filter by library name */
	    -1, /* dont filter by library eq */
	    faults_list);

	if (ret != 0) {
		fprintf(stderr, "get all faults failed:[%d]%s\n",
		    samerrno, samerrmsg);
	}
	return (ret);
}

/*
 * Read the faults from the FAULTLOG binary and map the file
 */
static int
read_faults(
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
	*size = 0;

	if (lstat64(FAULTLOG, &buf) != 0) {
		/* If FAULTLOG does not exist, it is a clean system */
		return (0);
	}

	if ((fd = open64(FAULTLOG, oflags)) < 0) {

		samerrno = SE_CANT_OPEN_FLOG;
		snprintf(samerrmsg, MAX_MSG_LEN,
		    GetCustMsg(SE_CANT_OPEN_FLOG), FAULTLOG);

		fprintf(stderr, "get faults failed: %s\n", samerrmsg);
		return (-1);
	}

	/* Stat the file for info to map */
	if (fstat64(fd, &st) != 0) {

		samerrno = SE_FSTAT_FAILED;
		snprintf(samerrmsg, MAX_MSG_LEN,
		    GetCustMsg(SE_FSTAT_FAILED), FAULTLOG);

		fprintf(stderr, "get faults failed: %s\n", samerrmsg);

		close(fd);
		return (-1);
	}

	*size = st.st_size;	/* size of memory mapped area */

	/*
	 * Map in the entire file.
	 * Solaris's MM takes care of the page-ins and outs.
	 */
	*mp = mmap(NULL, st.st_size, prot, flags, fd, 0);
	if (*mp == MAP_FAILED) {
		samerrno = SE_MMAP_FAILED;
		snprintf(samerrmsg, MAX_MSG_LEN,
		    GetCustMsg(SE_MMAP_FAILED), FAULTLOG);

		fprintf(stderr, "get faults failed: %s\n", samerrmsg);

		close(fd);
		return (-1);
	}

	close(fd);
	return (0);
}



static int
helper_get_faults(
char *libname,	/* If empty, don't filter by library name, return all */
equ_t eq,		/* If EQU_MAX + 1, don't filter by eq, return all */
sqm_lst_t ** faults_list)	/* return - list of faults */
{
	fault_attr_t *fp = NULL;	/* a ptr to fault in mem-mapped file */
	fault_attr_t *tmpp = NULL;	/* dup fault data */
	void*		mp = NULL;	/* a ptr to the mapped address */
	size_t		size = 0;	/* size of file memory mapped */
	int		total_faults = 0;
	int		ret = 0;	/* assume success */

	if (ISNULL(faults_list)) {
		fprintf(stderr, "get faults exit:[%d] %s\n", samerrno, samerrmsg);
		return (-1);
	}

	/* Get a memory map of all the faults in the system */
	if (read_faults(O_RDONLY, PROT_READ, MAP_SHARED, &mp, &size) == -1) {
		fprintf(stderr, "get faults exit:[%d] %s\n", samerrno, samerrmsg);
		return (-1);
	}

	*faults_list = NULL;
	*faults_list = lst_create();
	if (*faults_list == NULL) {
		fprintf(stderr, "get faults exit:[%d] %s\n", samerrno, samerrmsg);
		munmap(mp, size);
		return (-1);
	}

	/* calculate the total number of faults. */
	total_faults = size / sizeof (fault_attr_t);

	/* copy the pointer to the mapped area and cast it to a fault type */

	/*
	 * copy the fault records to a linked-list, start from the
	 * tail of the mmap pointer to get the latest faults first
	 */
	for (fp = (fault_attr_t *)mp;
	    (total_faults > 0 && fp != NULL);
	    fp++, total_faults--) {

		if ((libname != NULL) && (libname[0] != '\0') &&
		    (strncmp(fp->library, libname, strlen(libname)) != 0)) {

			continue;
		}

		if ((eq != (EQU_MAX + 1)) && (fp->eq != eq)) {
			continue;
		}

		tmpp = (fault_attr_t *)mallocer(sizeof (fault_attr_t));
		if (tmpp == NULL) {
			ret = -1;
			break;
		}
		memcpy(tmpp, fp, sizeof (fault_attr_t));

		if (lst_append(*faults_list, (void *)tmpp) != 0) {
			ret = -1;
			break;
		}
		tmpp = NULL;
	}

	munmap(mp, size);
	if (ret == -1) {
		fprintf(stderr, "get faults failed: [%d] %s\n",
		    samerrno, samerrmsg);
		if (tmpp != NULL) {
			free(tmpp);
			tmpp = NULL;
		}
		if (faults_list != NULL) {
			((*faults_list)->length > 0) ?
			    lst_free_deep(*faults_list) :
			    lst_free(*faults_list);

			*faults_list = NULL;
		}
		return (-1);
	} else {
		return (0);
	}
}
