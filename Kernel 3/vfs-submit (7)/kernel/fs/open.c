/******************************************************************************/
/* Important Summer 2019 CSCI 402 usage information:                          */
/*                                                                            */
/* This fils is part of CSCI 402 kernel programming assignments at USC.       */
/*         53616c7465645f5fd1e93dbf35cbffa3aef28f8c01d8cf2ffc51ef62b26a       */
/*         f9bda5a68e5ed8c972b17bab0f42e24b19daa7bd408305b1f7bd6c7208c1       */
/*         0e36230e913039b3046dd5fd0ba706a624d33dbaa4d6aab02c82fe09f561       */
/*         01b0fd977b0051f0b0ce0c69f7db857b1b5e007be2db6d42894bf93de848       */
/*         806d9152bd5715e9                                                   */
/* Please understand that you are NOT permitted to distribute or publically   */
/*         display a copy of this file (or ANY PART of it) for any reason.    */
/* If anyone (including your prospective employer) asks you to post the code, */
/*         you must inform them that you do NOT have permissions to do so.    */
/* You are also NOT permitted to remove or alter this comment block.          */
/* If this comment block is removed or altered in a submitted file, 20 points */
/*         will be deducted.                                                  */
/******************************************************************************/

/*
 *  FILE: open.c
 *  AUTH: mcc | jal
 *  DESC:
 *  DATE: Mon Apr  6 19:27:49 1998
 */

#include "globals.h"
#include "errno.h"
#include "fs/fcntl.h"
#include "util/string.h"
#include "util/printf.h"
#include "fs/vfs.h"
#include "fs/vnode.h"
#include "fs/file.h"
#include "fs/vfs_syscall.h"
#include "fs/open.h"
#include "fs/stat.h"
#include "util/debug.h"

/* find empty index in p->p_files[] */
int
get_empty_fd(proc_t *p)
{
        int fd;

        for (fd = 0; fd < NFILES; fd++) {
                if (!p->p_files[fd]) 
                        return fd;
        }

        dbg(DBG_ERROR | DBG_VFS, "ERROR: get_empty_fd: out of file descriptors "
            "for pid %d\n", curproc->p_pid);
        return -EMFILE;
}

/*
 * There a number of steps to opening a file:
 *      1. Get the next empty file descriptor.
 *      2. Call fget to get a fresh file_t.
 *      3. Save the file_t in curproc's file descriptor table.
 *      4. Set file_t->f_mode to OR of FMODE_(READ|WRITE|APPEND) based on
 *         oflags, which can be O_RDONLY, O_WRONLY or O_RDWR, possibly OR'd with
 *         O_APPEND.
 *      5. Use open_namev() to get the vnode for the file_t.
 *      6. Fill in the fields of the file_t.
 *      7. Return new fd.
 *
 * If anything goes wrong at any point (specifically if the call to open_namev
 * fails), be sure to remove the fd from curproc, fput the file_t and return an
 * error.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EINVAL
 *        oflags is not valid.
 *      o EMFILE
 *        The process already has the maximum number of files open.
 *      o ENOMEM
 *        Insufficient kernel memory was available.
 *      o ENAMETOOLONG
 *        A component of filename was too long.
 *      o ENOENT
 *        O_CREAT is not set and the named file does not exist.  Or, a
 *        directory component in pathname does not exist.
 *      o EISDIR
 *        pathname refers to a directory and the access requested involved
 *        writing (that is, O_WRONLY or O_RDWR is set).
 *      o ENXIO
 *        pathname refers to a device special file and no corresponding device
 *        exists.
 */

int
do_open(const char *filename, int oflags)
{
	if ((oflags & 0x003) == 3) {
	        dbg(DBG_PRINT, "(GRADING2B)\n");
        	return -EINVAL;
    	}
	
	KASSERT(strlen(filename) <= MAXPATHLEN);
	dbg(DBG_PRINT, "(GRADING2B)\n");
	dbg(DBG_PRINT, "(GRADING2B)\n");
	
	/*if(strlen(filename) > MAXPATHLEN)
        {
            dbg(DBG_PRINT, "(GRADING2B)\n");
            return -ENAMETOOLONG;
        }*/


	dbg(DBG_PRINT, "(GRADING2A)\n");
	/* 1. Get the next empty file descriptor. */
        int fd = get_empty_fd(curproc);

	/*if(fd == -EMFILE)
        {
        	dbg(DBG_PRINT, "(GRADING2B)\n");
        	return -EMFILE;
        }*/

	/* 2. Call fget to get a fresh file_t. */
        file_t *ft = fget(-1);
        KASSERT(NULL != ft);
	dbg(DBG_PRINT, "(GRADING2B)\n");
	dbg(DBG_PRINT, "(GRADING2B)\n");
	/*if (ft == NULL)
        {
        	dbg(DBG_PRINT, "(GRADING2B)\n");
        	return -ENOMEM;
    	}*/

	/* 3. Save the file_t in curproc's file descriptor table. */
        KASSERT(!curproc->p_files[fd]);
	dbg(DBG_PRINT, "(GRADING2B)\n");
	dbg(DBG_PRINT, "(GRADING2C 1)\n");
        curproc->p_files[fd] = ft;

	/* 4. Set file_t->f_mode to OR of FMODE_(READ|WRITE|APPEND) based on
 	*     oflags, which can be O_RDONLY, O_WRONLY or O_RDWR, possibly OR'd with
	*     O_APPEND. */
	int oflags_temp = oflags&0x40f;

	if(oflags_temp == O_RDONLY)
	{
		ft->f_mode = FMODE_READ;
	}
        if(oflags_temp == O_WRONLY)
	{
            ft->f_mode = FMODE_WRITE;
	}
        if(oflags_temp == O_RDWR)
	{
            ft->f_mode = FMODE_READ | FMODE_WRITE;
	}
        /*if(oflags_temp == (O_RDONLY | O_APPEND))
	{
            ft->f_mode = FMODE_READ | FMODE_APPEND;
	}
        if(oflags_temp == (O_WRONLY | O_APPEND))
	{
            ft->f_mode = FMODE_WRITE | FMODE_APPEND;
	}*/
        if(oflags_temp == (O_RDWR | O_APPEND))
	{
            ft->f_mode = FMODE_READ | FMODE_WRITE | FMODE_APPEND;
	}

        vnode_t *res_vnode = NULL;

	/* 5. Use open_namev() to get the vnode for the file_t.*/
        int open_err = open_namev(filename, oflags, &res_vnode, NULL);
	dbg(DBG_PRINT, "(GRADING2B)\n");
        if (open_err != 0)
        {
                curproc->p_files[fd] = NULL;
                fput(ft);
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return open_err;
        }

	dbg(DBG_PRINT, "(GRADING2B)\n");
        if (S_ISDIR(res_vnode->vn_mode))
        {
                if ((oflags & O_WRONLY) == O_WRONLY || (oflags & O_RDWR) == O_RDWR)
                {
                        curproc->p_files[fd] = NULL;
                        fput(ft);
                    
                        vput(res_vnode);
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        return -EISDIR;
                }
                dbg(DBG_PRINT, "(GRADING2B)\n");
        }

	/*if(S_ISCHR(res_vnode->vn_mode))
        {
        	if(res_vnode->vn_cdev == NULL)
        	{
        		curproc->p_files[fd] = NULL;
			fput(ft);
			vput(res_vnode);
			dbg(DBG_PRINT, "(GRADING2B)\n");
			return -ENXIO;
        	}
        }

	if(S_ISBLK(res_vnode->vn_mode))
        {
        	if(res_vnode->vn_bdev == NULL)
            	{
            		curproc->p_files[fd] = NULL;
			fput(ft);
			vput(res_vnode);
			dbg(DBG_PRINT, "(GRADING2B)\n");
			return -ENXIO;
            	}
        }*/

	/* 6. Fill in the fields of the file_t. */
        ft->f_vnode = res_vnode;

        ft->f_pos = 0;

        ft->f_refcount = 1;

        dbg(DBG_PRINT, "(GRADING2C 1)\n");
        return fd;
}
