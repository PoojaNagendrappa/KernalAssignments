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

#include "kernel.h"
#include "globals.h"
#include "types.h"
#include "errno.h"

#include "util/string.h"
#include "util/printf.h"
#include "util/debug.h"

#include "fs/dirent.h"
#include "fs/fcntl.h"
#include "fs/stat.h"
#include "fs/vfs.h"
#include "fs/vnode.h"

/* This takes a base 'dir', a 'name', its 'len', and a result vnode.
 * Most of the work should be done by the vnode's implementation
 * specific lookup() function.
 *
 * If dir has no lookup(), return -ENOTDIR.
 *
 * Note: returns with the vnode refcount on *result incremented.
 */
int
lookup(vnode_t *dir, const char *name, size_t len, vnode_t **result)
{
	KASSERT(NULL != dir);
	dbg(DBG_PRINT, "(GRADING2A 2.a)\n");
	dbg(DBG_PRINT, "(GRADING2D)\n");

	KASSERT(NULL != name);
	dbg(DBG_PRINT, "(GRADING2A 2.a)\n");
	dbg(DBG_PRINT, "(GRADING2D)\n");

	KASSERT(NULL != result);
	dbg(DBG_PRINT, "(GRADING2A 2.a)\n");
	dbg(DBG_PRINT, "(GRADING2D)\n");

	if(len == 0)
	{
		dbg(DBG_PRINT, "(GRADING2B)\n");
		return -EINVAL;
	}

	if(!S_ISDIR(dir->vn_mode) || !dir->vn_ops->lookup)
	{
		dbg(DBG_PRINT, "(GRADING2D)\n");
		return -ENOTDIR;
	}

	if((dir == vfs_root_vn) && (strncmp(name,"..",2) == 0))
	{
		*result = vfs_root_vn;
		vref(vfs_root_vn);
		dbg(DBG_PRINT, "(GRADING2B)\n");
		return 0;
	}

	int retVal = dir->vn_ops->lookup(dir, name, len, result);
	dbg(DBG_PRINT, "(GRADING2A)\n");
	return retVal;
}




/* When successful this function returns data in the following "out"-arguments:
 *  o res_vnode: the vnode of the parent directory of "name"
 *  o name: the `basename' (the element of the pathname)
 *  o namelen: the length of the basename
 *
 * For example: dir_namev("/s5fs/bin/ls", &namelen, &name, NULL,
 * &res_vnode) would put 2 in namelen, "ls" in name, and a pointer to the
 * vnode corresponding to "/s5fs/bin" in res_vnode.
 *
 * The "base" argument defines where we start resolving the path from:
 * A base value of NULL means to use the process's current working directory,
 * curproc->p_cwd.  If pathname[0] == '/', ignore base and start with
 * vfs_root_vn.  dir_namev() should call lookup() to take care of resolving each
 * piece of the pathname.
 *
 * Note: A successful call to this causes vnode refcount on *res_vnode to
 * be incremented.
 */
int
dir_namev(const char *pathname, size_t *namelen, const char **name,
          vnode_t *base, vnode_t **res_vnode)
{
	vnode_t *tempBase = base;
	vnode_t *nextNode;
	int retVal = 0;
	char file_name[MAXPATHLEN];
	const char *local_pathname = pathname;
	const char *next_pathname = NULL;
	char *slash_end = strchr(pathname, '/');

	KASSERT(NULL != pathname);
	dbg(DBG_PRINT, "(GRADING2A 2.b)\n");
	dbg(DBG_PRINT, "(GRADING2D)\n");

	KASSERT(NULL != namelen);
	dbg(DBG_PRINT, "(GRADING2A 2.b)\n");
	dbg(DBG_PRINT, "(GRADING2D)\n");

	KASSERT(NULL != name);
	dbg(DBG_PRINT, "(GRADING2A 2.b)\n");
	dbg(DBG_PRINT, "(GRADING2D)\n");

	KASSERT(NULL != res_vnode);
	dbg(DBG_PRINT, "(GRADING2A 2.b)\n");
	dbg(DBG_PRINT, "(GRADING2D)\n");

	if(tempBase == NULL)
	{
		tempBase = curproc->p_cwd;
		dbg(DBG_PRINT,"(GRADING2A)\n");
	}

	if(slash_end != NULL  && (slash_end - pathname) == 0)
	{
		tempBase = vfs_root_vn;
		while(*local_pathname == '/')
		{
			local_pathname++;
			dbg(DBG_PRINT,"(GRADING2A)\n");
		}
		
		if(*local_pathname == '\0')
		{
			*name = &pathname[local_pathname - pathname];
			*namelen = 0;
			vref(tempBase);
			*res_vnode = tempBase;
			dbg(DBG_PRINT,"(GRADING2B)\n");
			return 0;
		}
		dbg(DBG_PRINT,"(GRADING2A)\n");
		slash_end = strchr(local_pathname, '/');
	}
	dbg(DBG_PRINT,"(GRADING2A)\n");
	vref(tempBase);

	while(slash_end != NULL)
	{
		dbg(DBG_PRINT,"(GRADING2A)\n");
		strncpy(file_name, local_pathname, slash_end - local_pathname);
		file_name[slash_end - local_pathname] = '\0';
		
		/*if(strlen(file_name) > NAME_LEN)
		{			
			vput(tempBase);
			dbg(DBG_PRINT,"(GRADING2D)\n");
dbg(DBG_TEMP, " -------------------------------------------------------------------------------- CHECK  32\n");
			return -ENAMETOOLONG;
		}*/
		
		next_pathname = slash_end;
		while(*next_pathname == '/')
		{
			next_pathname++;
			dbg(DBG_PRINT,"(GRADING2A)\n");
		}
		
		if(*next_pathname == '\0')
		{
			dbg(DBG_PRINT,"(GRADING2A)\n");
			break;
		}
	
		retVal = lookup(tempBase, file_name, strlen(file_name), &nextNode);
		vput(tempBase);
		if(retVal != 0)
		{
			dbg(DBG_PRINT,"(GRADING2B)\n");
			return retVal;
		}
		
		KASSERT(NULL != nextNode);
		dbg(DBG_PRINT,"(GRADING2A 2.b)\n");
		dbg(DBG_PRINT,"(GRADING2D)\n");
		
		tempBase = nextNode;
		local_pathname = next_pathname;
		slash_end = strchr(local_pathname, '/');
		dbg(DBG_PRINT,"(GRADING2A)\n");
	}

	if(!S_ISDIR(tempBase->vn_mode))
	{
		KASSERT(tempBase);
		vput(tempBase);
		dbg(DBG_PRINT,"(GRADING2B)\n");
		return -ENOTDIR;
	}

	*name = &pathname[local_pathname - pathname];

	if(slash_end != NULL)
	{
		*namelen = slash_end - local_pathname;
		dbg(DBG_PRINT,"(GRADING2B)\n");
	}
	else
	{
		*namelen = strlen(local_pathname);
		dbg(DBG_PRINT,"(GRADING2A)\n");
	}
	
	if(*namelen > NAME_LEN)
    	{
        	vput(tempBase);
		dbg(DBG_PRINT,"(GRADING2B)\n");
        	return -ENAMETOOLONG;
    	}

	*res_vnode = tempBase;
	return retVal;
}

/* This returns in res_vnode the vnode requested by the other parameters.
 * It makes use of dir_namev and lookup to find the specified vnode (if it
 * exists).  flag is right out of the parameters to open(2); see
 * <weenix/fcntl.h>.  If the O_CREAT flag is specified and the file does
 * not exist, call create() in the parent directory vnode. However, if the
 * parent directory itself does not exist, this function should fail - in all
 * cases, no files or directories other than the one at the very end of the path
 * should be created.
 *
 * Note: Increments vnode refcount on *res_vnode.
 */
int
open_namev(const char *pathname, int flag, vnode_t **res_vnode, vnode_t *base)
{
	size_t namelen = NULL;
    	const char* file_name;
    	vnode_t *dir_node;
    	int retVal = 0;

    	retVal = dir_namev(pathname, &namelen, &file_name, base, &dir_node);

	if(retVal != 0)
	{
		dbg(DBG_PRINT,"(GRADING2B)\n");
        	return retVal;
	}
	
	if(namelen == 0)
	{
		*res_vnode = dir_node;
		dbg(DBG_PRINT,"(GRADING2B)\n");
		return 0;
	}

	retVal = lookup(dir_node, file_name, namelen, res_vnode);
	if(retVal != 0)
	{
		if((flag & O_CREAT) != 0 && (strlen(pathname) >1 || pathname[0]!='.'))
		{
            		KASSERT(NULL != dir_node->vn_ops->create);
			dbg(DBG_PRINT,"(GRADING2A 2.c)\n");
			dbg(DBG_PRINT,"(GRADING2B)\n");
			retVal = dir_node->vn_ops->create(dir_node, file_name, namelen, res_vnode);
	    	}
		dbg(DBG_PRINT,"(GRADING2B)\n");
    	}
	else if(pathname[strlen(pathname) - 1] == '/' && !S_ISDIR((*res_vnode)->vn_mode))
	{
		vput(dir_node);
		vput(*res_vnode);
		dbg(DBG_PRINT,"(GRADING2B)\n");
		return -ENOTDIR;
	}

	vput(dir_node);
	dbg(DBG_PRINT,"(GRADING2A\n");
    	return retVal;
}

#ifdef __GETCWD__
/* Finds the name of 'entry' in the directory 'dir'. The name is writen
 * to the given buffer. On success 0 is returned. If 'dir' does not
 * contain 'entry' then -ENOENT is returned. If the given buffer cannot
 * hold the result then it is filled with as many characters as possible
 * and a null terminator, -ERANGE is returned.
 *
 * Files can be uniquely identified within a file system by their
 * inode numbers. */
int
lookup_name(vnode_t *dir, vnode_t *entry, char *buf, size_t size)
{
        NOT_YET_IMPLEMENTED("GETCWD: lookup_name");
        return -ENOENT;
}


/* Used to find the absolute path of the directory 'dir'. Since
 * directories cannot have more than one link there is always
 * a unique solution. The path is writen to the given buffer.
 * On success 0 is returned. On error this function returns a
 * negative error code. See the man page for getcwd(3) for
 * possible errors. Even if an error code is returned the buffer
 * will be filled with a valid string which has some partial
 * information about the wanted path. */
ssize_t
lookup_dirpath(vnode_t *dir, char *buf, size_t osize)
{
        NOT_YET_IMPLEMENTED("GETCWD: lookup_dirpath");

        return -ENOENT;
}
#endif /* __GETCWD__ */

