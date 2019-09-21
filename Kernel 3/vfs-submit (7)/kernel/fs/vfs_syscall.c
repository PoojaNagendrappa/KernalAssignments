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
 *  FILE: vfs_syscall.c
 *  AUTH: mcc | jal
 *  DESC:
 *  DATE: Wed Apr  8 02:46:19 1998
 *  $Id: vfs_syscall.c,v 1.2 2018/05/27 03:57:26 cvsps Exp $
 */

#include "kernel.h"
#include "errno.h"
#include "globals.h"
#include "fs/vfs.h"
#include "fs/file.h"
#include "fs/vnode.h"
#include "fs/vfs_syscall.h"
#include "fs/open.h"
#include "fs/fcntl.h"
#include "fs/lseek.h"
#include "mm/kmalloc.h"
#include "util/string.h"
#include "util/printf.h"
#include "fs/stat.h"
#include "util/debug.h"

/*
 * Syscalls for vfs. Refer to comments or man pages for implementation.
 * Do note that you don't need to set errno, you should just return the
 * negative error code.
 */

/* To read a file:
 *      o fget(fd)
 *      o call its virtual read vn_op
 *      o update f_pos
 *      o fput() it
 *      o return the number of bytes read, or an error
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd is not a valid file descriptor or is not open for reading.
 *      o EISDIR
 *        fd refers to a directory.
 *
 * In all cases, be sure you do not leak file refcounts by returning before
 * you fput() a file that you fget()'ed.
 */
int
do_read(int fd, void *buf, size_t nbytes)
{
        if ((fd < 0 || fd >= NFILES) || (curproc->p_files[fd] == NULL))
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF;
        }        

        file_t *rfile = fget(fd);
	KASSERT(NULL != rfile);
	dbg(DBG_PRINT, "(GRADING2B)\n");
	dbg(DBG_PRINT, "(GRADING2B)\n");

	/*if(rfile == NULL)
        {
		  dbg(DBG_PRINT, "(GRADING2B)\n");
		  return -EBADF;
        }*/

	if (S_ISDIR(rfile->f_vnode->vn_mode))
        {
                fput(rfile);
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EISDIR;
        }

        if (!(rfile->f_mode & FMODE_READ))
        {
                fput(rfile);
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF;
        }

        int rcount = rfile->f_vnode->vn_ops->read(rfile->f_vnode, rfile->f_pos, buf, nbytes);

        if (rcount > 0)
        {
                do_lseek(fd, rcount, SEEK_CUR);
                dbg(DBG_PRINT, "(GRADING2B)\n");
        }

        fput(rfile);
        dbg(DBG_PRINT, "(GRADING2B)\n");
        return rcount;
}

/* Very similar to do_read.  Check f_mode to be sure the file is writable.  If
 * f_mode & FMODE_APPEND, do_lseek() to the end of the file, call the write
 * vn_op, and fput the file.  As always, be mindful of refcount leaks.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd is not a valid file descriptor or is not open for writing.
 */
int
do_write(int fd, const void *buf, size_t nbytes)
{
        if ((fd < 0 || fd >= NFILES) || (curproc->p_files[fd] == NULL))
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF;
        }

        file_t *rfile = fget(fd);
	KASSERT(NULL != rfile);
	dbg(DBG_PRINT, "(GRADING2B)\n");
	dbg(DBG_PRINT, "(GRADING2B)\n");

	/*if(rfile == NULL)
        {
		dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF;
        }*/

        if ((rfile->f_mode & FMODE_WRITE) != FMODE_WRITE)
        {
                fput(rfile);
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF;
        }

        if ((rfile->f_mode & FMODE_APPEND) == FMODE_APPEND)
        {
                do_lseek(fd, 0, SEEK_END);
                dbg(DBG_PRINT, "(GRADING2B)\n");
        }

        int wcount = rfile->f_vnode->vn_ops->write(rfile->f_vnode, rfile->f_pos, buf, nbytes);
        if (wcount > 0)
        {
                do_lseek(fd, wcount, SEEK_CUR);
                dbg(DBG_PRINT, "(GRADING2B)\n");
        
                KASSERT((S_ISCHR(rfile->f_vnode->vn_mode)) || 
                        (S_ISBLK(rfile->f_vnode->vn_mode)) || 
                        ((S_ISREG(rfile->f_vnode->vn_mode)) && (rfile->f_pos <= rfile->f_vnode->vn_len)));
                dbg(DBG_PRINT, "(GRADING2A 3.a)\n");
		dbg(DBG_PRINT, "(GRADING2B)\n");
        }

        fput(rfile);
        dbg(DBG_PRINT, "(GRADING2C 1)\n");
        return wcount;
}

/*
 * Zero curproc->p_files[fd], and fput() the file. Return 0 on success
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd isn't a valid open file descriptor.
 */
int
do_close(int fd)
{
        if ((fd < 0 || fd >= NFILES) || (!curproc->p_files[fd]))
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF;
        }

        file_t *cfile = curproc->p_files[fd];
        curproc->p_files[fd] = NULL;
        
        fput(cfile);
        dbg(DBG_PRINT, "(GRADING2C 1)\n");
        return 0;
}

/* To dup a file:
 *      o fget(fd) to up fd's refcount
 *      o get_empty_fd()
 *      o point the new fd to the same file_t* as the given fd
 *      o return the new file descriptor
 *
 * Don't fput() the fd unless something goes wrong.  Since we are creating
 * another reference to the file_t*, we want to up the refcount.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd isn't an open file descriptor.
 *      o EMFILE
 *        The process already has the maximum number of file descriptors open
 *        and tried to open a new one.
 */
int
do_dup(int fd)
{
        if ((fd < 0 || fd >= NFILES) || (!curproc->p_files[fd]))
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF;
        }

        file_t *dupfile = fget(fd);
        
	KASSERT(NULL != dupfile);
	dbg(DBG_PRINT, "(GRADING2B)\n");
	dbg(DBG_PRINT, "(GRADING2B)\n");

	/*if(dupfile == NULL)
    	{
		dbg(DBG_PRINT, "(GRADING2B)\n");
		return -EBADF;
    	}*/

        int newfd = get_empty_fd(curproc);
 
	/*if(newfd < 0)
    	{    
		fput(dupfile);
		dbg(DBG_PRINT, "(GRADING2B)\n");
        	return newfd;
    	}*/

        curproc->p_files[newfd] = dupfile;
        dbg(DBG_PRINT, "(GRADING2B)\n");
        return newfd;
}

/* Same as do_dup, but insted of using get_empty_fd() to get the new fd,
 * they give it to us in 'nfd'.  If nfd is in use (and not the same as ofd)
 * do_close() it first.  Then return the new file descriptor.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        ofd isn't an open file descriptor, or nfd is out of the allowed
 *        range for file descriptors.
 */
int
do_dup2(int ofd, int nfd)
{
	if ((ofd < 0 || ofd >= NFILES) || (!curproc->p_files[ofd]))
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF; 
        }

        /*if (nfd < 0 || nfd >= NFILES)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF; 
        }*/    

        if (ofd == nfd)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return ofd;
        }

        file_t *dup2file = fget(ofd);

	KASSERT(NULL != dup2file);
	dbg(DBG_PRINT, "(GRADING2B)\n");
	dbg(DBG_PRINT, "(GRADING2B)\n");


	/*if (dup2file == NULL) 
	{
		dbg(DBG_PRINT, "(GRADING2B)\n");
		return -EBADF;
	}*/

        curproc->p_files[nfd]=dup2file;
        dbg(DBG_PRINT, "(GRADING2B)\n");
        return nfd;
}

/*
 * This routine creates a special file of the type specified by 'mode' at
 * the location specified by 'path'. 'mode' should be one of S_IFCHR or
 * S_IFBLK (you might note that mknod(2) normally allows one to create
 * regular files as well-- for simplicity this is not the case in Weenix).
 * 'devid', as you might expect, is the device identifier of the device
 * that the new special file should represent.
 *
 * You might use a combination of dir_namev, lookup, and the fs-specific
 * mknod (that is, the containing directory's 'mknod' vnode operation).
 * Return the result of the fs-specific mknod, or an error.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EINVAL
 *        mode requested creation of something other than a device special
 *        file.
 *      o EEXIST
 *        path already exists.
 *      o ENOENT
 *        A directory component in path does not exist.
 *      o ENOTDIR
 *        A component used as a directory in path is not, in fact, a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
int
do_mknod(const char *path, int mode, unsigned devid)
{
        size_t namelen = 0;
        const char *name;
        vnode_t  *dir_vnode = NULL;
        vnode_t  *lnode = NULL;
	int result = 0;
   
	/*if (!(S_ISCHR(mode) || (S_ISBLK(mode))))
	{
	      dbg(DBG_PRINT, "(GRADING2B)\n");
	      return -EINVAL;
	}*/

	result = dir_namev(path, &namelen, &name, NULL, &dir_vnode);
        KASSERT(0 == result);
	dbg(DBG_PRINT, "(GRADING2B)\n");
	dbg(DBG_PRINT, "(GRADING2B)\n");
	/*if((result = dir_namev(path, &namelen, &name, NULL, &dir_vnode)) != 0)
	{
		dbg(DBG_PRINT, "(GRADING2B)\n");
		return result;
	}*/

        result = lookup(dir_vnode, name, namelen, &lnode);

        if (result == -ENOENT)
        {
                KASSERT(NULL != dir_vnode->vn_ops->mknod);
                dbg(DBG_PRINT, "(GRADING2A 3.b)\n");
		dbg(DBG_PRINT, "(GRADING2B)\n");

                result = dir_vnode->vn_ops->mknod(dir_vnode, name, namelen, mode, devid);
                dbg(DBG_PRINT, "(GRADING2B)\n");
        }
        vput(dir_vnode);
        dbg(DBG_PRINT, "(GRADING2B)\n");
        return result;
}

/* Use dir_namev() to find the vnode of the dir we want to make the new
 * directory in.  Then use lookup() to make sure it doesn't already exist.
 * Finally call the dir's mkdir vn_ops. Return what it returns.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EEXIST
 *        path already exists.
 *      o ENOENT
 *        A directory component in path does not exist.
 *      o ENOTDIR
 *        A component used as a directory in path is not, in fact, a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
int
do_mkdir(const char *path)
{
        size_t namelen = 0;
        const char *name = NULL;
        vnode_t *dir_vnode = NULL;
        vnode_t *lnode = NULL;
	int result = 0;

        result = dir_namev(path, &namelen, &name, NULL, &dir_vnode);
	if (result < 0)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return result;
        }
        result = lookup(dir_vnode, name, namelen, &lnode);

        if (result == 0)
        {
                result = -EEXIST;
                vput(lnode);
                dbg(DBG_PRINT, "(GRADING2B)\n");
        }
        else if (result == -ENOENT)
        {
                KASSERT(NULL != dir_vnode->vn_ops->mkdir);
                dbg(DBG_PRINT, "(GRADING2A 3.c)\n");
		dbg(DBG_PRINT, "(GRADING2B)\n");

                result = dir_vnode->vn_ops->mkdir(dir_vnode, name, namelen);
                dbg(DBG_PRINT, "(GRADING2B)\n");
        }

        vput(dir_vnode);
        dbg(DBG_PRINT, "(GRADING2C 1)\n");
        return result;
}

/* Use dir_namev() to find the vnode of the directory containing the dir to be
 * removed. Then call the containing dir's rmdir v_op.  The rmdir v_op will
 * return an error if the dir to be removed does not exist or is not empty, so
 * you don't need to worry about that here. Return the value of the v_op,
 * or an error.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EINVAL
 *        path has "." as its final component.
 *      o ENOTEMPTY
 *        path has ".." as its final component.
 *      o ENOENT
 *        A directory component in path does not exist.
 *      o ENOTDIR
 *        A component used as a directory in path is not, in fact, a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
int
do_rmdir(const char *path)
{
	size_t namelen = 0;
        const char *name = NULL;
        vnode_t *dir_vnode = NULL;

        int retVal = dir_namev(path, &namelen, &name, NULL, &dir_vnode);

	if(retVal < 0)
	{
		dbg(DBG_PRINT, "(GRADING2C 1)\n");
        	return retVal;
	}
        if(strcmp(name, ".") == 0)
        {
                vput(dir_vnode);
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EINVAL;
        }       
                
        if(strcmp(name, "..") == 0)      
        {
                vput(dir_vnode);
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -ENOTEMPTY;
        }                

        if (retVal == 0)
        {
                if (dir_vnode->vn_ops->rmdir == NULL)
                {
                        vput(dir_vnode);
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        return -ENOTDIR;
                }
                else
                {
                        KASSERT(NULL != dir_vnode->vn_ops->rmdir);
                        dbg(DBG_PRINT, "(GRADING2A 3.d)\n");
                        dbg(DBG_PRINT, "(GRADING2B)\n");

                        retVal = dir_vnode->vn_ops->rmdir(dir_vnode, name, namelen);
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                }
                dbg(DBG_PRINT, "(GRADING2B)\n");
        }

        vput(dir_vnode);
        dbg(DBG_PRINT, "(GRADING2B)\n");
        return retVal;
}

/*
 * Similar to do_rmdir, but for files.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EPERM
 *        path refers to a directory.
 *      o ENOENT
 *        Any component in path does not exist, including the element at the
 *        very end.
 *      o ENOTDIR
 *        A component used as a directory in path is not, in fact, a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
int
do_unlink(const char *path)
{
        size_t namelen = 0;
        const char *name = NULL;
        vnode_t *dir_vnode = NULL;
        vnode_t *lnode = NULL;
        int result = 0;

        result = dir_namev(path, &namelen, &name, NULL, &dir_vnode);

        result = lookup(dir_vnode, name, namelen, &lnode);

        if (result == 0)
        {
                vput(lnode);
                if (S_ISDIR(lnode->vn_mode))
                {
                        result = -EPERM;                        
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                }
                else
                {
                        KASSERT(NULL != dir_vnode->vn_ops->unlink);
                        dbg(DBG_PRINT, "(GRADING2A 3.e)\n");
                        dbg(DBG_PRINT, "(GRADING2B)\n");

                        result = dir_vnode->vn_ops->unlink(dir_vnode, name, namelen);
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                }
                dbg(DBG_PRINT, "(GRADING2B)\n");
        }

        vput(dir_vnode);
        dbg(DBG_PRINT, "(GRADING2B)\n");
        return result;
}

/* To link:
 *      o open_namev(from)
 *      o dir_namev(to)
 *      o call the destination dir's (to) link vn_ops.
 *      o return the result of link, or an error
 *
 * Remember to vput the vnodes returned from open_namev and dir_namev.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EEXIST
 *        to already exists.
 *      o ENOENT
 *        A directory component in from or to does not exist.
 *      o ENOTDIR
 *        A component used as a directory in from or to is not, in fact, a
 *        directory.
 *      o ENAMETOOLONG
 *        A component of from or to was too long.
 *      o EPERM
 *        from is a directory.
 */
int
do_link(const char *from, const char *to)
{
        size_t namelen = 0;
        const char *name = NULL;
        vnode_t *res_vnode = NULL;
        vnode_t *lnode = NULL;
        vnode_t *onode = NULL;
        

        int result = open_namev(from, 0, &onode, NULL);

	if(result < 0)
	{
		dbg(DBG_PRINT, "(GRADING2B)\n");
        	return result;
	}

	/*if(S_ISDIR(onode->vn_mode))
     	{
		dbg(DBG_PRINT, "(GRADING2B)\n");
       		return -EISDIR;
     	}*/

        int retVal = dir_namev(to, &namelen, &name, NULL, &res_vnode);

	KASSERT(retVal >= 0);
	dbg(DBG_PRINT, "(GRADING2B)\n");
	dbg(DBG_PRINT, "(GRADING2B)\n");

	/*if(retVal < 0)
       	{
		dbg(DBG_PRINT, "(GRADING2B)\n");
         	return retVal;
       	}*/

        if (res_vnode->vn_ops->link == NULL)
        {
                vput(res_vnode);
                vput(onode);
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -ENOTDIR;
        }

        return 0;
}

/*      o link newname to oldname
 *      o unlink oldname
 *      o return the value of unlink, or an error
 *
 * Note that this does not provide the same behavior as the
 * Linux system call (if unlink fails then two links to the
 * file could exist).
 */
int
do_rename(const char *oldname, const char *newname)
{
        int retVal = do_link(oldname, newname);

        if (retVal < 0)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return retVal;
        }
	dbg(DBG_PRINT, "(GRADING2B)\n"); 
        return 0;
}

/* Make the named directory the current process's cwd (current working
 * directory).  Don't forget to down the refcount to the old cwd (vput()) and
 * up the refcount to the new cwd (open_namev() or vget()). Return 0 on
 * success.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o ENOENT
 *        path does not exist.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 *      o ENOTDIR
 *        A component of path is not a directory.
 */
int
do_chdir(const char *path)
{
        vnode_t *new_Dir;

        int result = open_namev(path, 0, &new_Dir, NULL);

	if(result < 0)
	{
		dbg(DBG_PRINT, "(GRADING2B)\n");
		return result;	
	}
        if (!S_ISDIR(new_Dir->vn_mode))
        {
                result = -ENOTDIR;
                vput(new_Dir);
                dbg(DBG_PRINT, "(GRADING2B)\n");
        }        
        else
        {
                vput(curproc->p_cwd);
                curproc->p_cwd = new_Dir;
                dbg(DBG_PRINT, "(GRADING2B)\n");
        }
        dbg(DBG_PRINT, "(GRADING2B)\n");
        return result;
}

/* Call the readdir vn_op on the given fd, filling in the given dirent_t*.
 * If the readdir vn_op is successful, it will return a positive value which
 * is the number of bytes copied to the dirent_t.  You need to increment the
 * file_t's f_pos by this amount.  As always, be aware of refcounts, check
 * the return value of the fget and the virtual function, and be sure the
 * virtual function exists (is not null) before calling it.
 *
 * Return either 0 or sizeof(dirent_t), or -errno.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        Invalid file descriptor fd.
 *      o ENOTDIR
 *        File descriptor does not refer to a directory.
 */
int
do_getdent(int fd, struct dirent *dirp)
{
       if ((fd < 0 || fd >= NFILES) || (!curproc->p_files[fd]))
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF;
        }

        file_t *ddir = fget(fd);

        if (!(S_ISDIR(ddir->f_vnode->vn_mode)))
        {
                fput(ddir);
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -ENOTDIR;
        }        

        dbg(DBG_PRINT, "(GRADING2B)\n");
        int retVal = ddir->f_vnode->vn_ops->readdir(ddir->f_vnode, ddir->f_pos, dirp);

        int fret_val = retVal;
        if (retVal > 0)
        {
                fret_val = sizeof(dirent_t);
                do_lseek(fd, retVal, SEEK_CUR);
                dbg(DBG_PRINT, "(GRADING2B)\n");
        }

        fput(ddir);
        dbg(DBG_PRINT, "(GRADING2C 1)\n");
        return fret_val;
}

/*
 * Modify f_pos according to offset and whence.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd is not an open file descriptor.
 *      o EINVAL
 *        whence is not one of SEEK_SET, SEEK_CUR, SEEK_END; or the resulting
 *        file offset would be negative.
 */
int
do_lseek(int fd, int offset, int whence)
{
	int roffset = 0;
        if ((fd < 0 || fd >= NFILES) || (!curproc->p_files[fd]))
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF;
        }

        file_t *fseek = fget(fd);

	KASSERT(NULL != fseek);
	dbg(DBG_PRINT, "(GRADING2B)\n");
	dbg(DBG_PRINT, "(GRADING2B)\n");

	/*if(fseek == NULL)
	{
		dbg(DBG_PRINT, "(GRADING2B)\n");
		return -EBADF;	
	}*/

        switch (whence)
        {
                case SEEK_SET:
                        roffset = offset;
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        break;

                case SEEK_CUR:
                        roffset = fseek->f_pos + offset;
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        break;

                case SEEK_END:
                        roffset = fseek->f_vnode->vn_len + offset;
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        break;

                default:
                        roffset = -EINVAL;
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        break;
        }

        if (roffset >= 0)
        {
                fseek->f_pos = roffset;
                dbg(DBG_PRINT, "(GRADING2B)\n");
        }
        else 
        {
                roffset = -EINVAL;
                dbg(DBG_PRINT, "(GRADING2B)\n");
        }

        fput(fseek);
        dbg(DBG_PRINT, "(GRADING2B)\n");
        return roffset;
}

/*
 * Find the vnode associated with the path, and call the stat() vnode operation.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o ENOENT
 *        A component of path does not exist.
 *      o ENOTDIR
 *        A component of the path prefix of path is not a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 *      o EINVAL
 *        path is an empty string.
 */
int
do_stat(const char *path, struct stat *buf)
{
       vnode_t *res_vnode = NULL;

        if (strlen(path) == 0)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EINVAL;
        }

        int res = open_namev(path, 0, &res_vnode, NULL);

        int retVal = res;
        if (res == 0)
        {
                KASSERT(NULL != res_vnode->vn_ops->stat);
                dbg(DBG_PRINT, "(GRADING2A 3.f)\n");
                dbg(DBG_PRINT, "(GRADING2B)\n");

                retVal = res_vnode->vn_ops->stat(res_vnode, buf);
                vput(res_vnode);
                dbg(DBG_PRINT, "(GRADING2B)\n");
        }       
        dbg(DBG_PRINT, "(GRADING2B)");
        return retVal;
}

#ifdef __MOUNTING__
/*
 * Implementing this function is not required and strongly discouraged unless
 * you are absolutely sure your Weenix is perfect.
 *
 * This is the syscall entry point into vfs for mounting. You will need to
 * create the fs_t struct and populate its fs_dev and fs_type fields before
 * calling vfs's mountfunc(). mountfunc() will use the fields you populated
 * in order to determine which underlying filesystem's mount function should
 * be run, then it will finish setting up the fs_t struct. At this point you
 * have a fully functioning file system, however it is not mounted on the
 * virtual file system, you will need to call vfs_mount to do this.
 *
 * There are lots of things which can go wrong here. Make sure you have good
 * error handling. Remember the fs_dev and fs_type buffers have limited size
 * so you should not write arbitrary length strings to them.
 */
int
do_mount(const char *source, const char *target, const char *type)
{
        NOT_YET_IMPLEMENTED("MOUNTING: do_mount");
        return -EINVAL;
}

/*
 * Implementing this function is not required and strongly discouraged unless
 * you are absolutley sure your Weenix is perfect.
 *
 * This function delegates all of the real work to vfs_umount. You should not worry
 * about freeing the fs_t struct here, that is done in vfs_umount. All this function
 * does is figure out which file system to pass to vfs_umount and do good error
 * checking.
 */
int
do_umount(const char *target)
{
        NOT_YET_IMPLEMENTED("MOUNTING: do_umount");
        return -EINVAL;
}
#endif
