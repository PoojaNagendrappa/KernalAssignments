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

#include "globals.h"
#include "errno.h"
#include "types.h"

#include "mm/mm.h"
#include "mm/tlb.h"
#include "mm/mman.h"
#include "mm/page.h"

#include "proc/proc.h"

#include "util/string.h"
#include "util/debug.h"

#include "fs/vnode.h"
#include "fs/vfs.h"
#include "fs/file.h"

#include "vm/vmmap.h"
#include "vm/mmap.h"

/*
 * This function implements the mmap(2) syscall, but only
 * supports the MAP_SHARED, MAP_PRIVATE, MAP_FIXED, and
 * MAP_ANON flags.
 *
 * Add a mapping to the current process's address space.
 * You need to do some error checking; see the ERRORS section
 * of the manpage for the problems you should anticipate.
 * After error checking most of the work of this function is
 * done by vmmap_map(), but remember to clear the TLB.
 */
int
do_mmap(void *addr, size_t len, int prot, int flags,
        int fd, off_t off, void **ret)
{
	int mode = flags % 4;
	file_t *mmap_file = NULL;
	vmarea_t *vmarea_temp = NULL;
        vnode_t *vnode_temp = NULL;
	vmmap_t *vp = NULL;
	int retVal = 0;
	uint32_t addr_temp;

	if((addr == NULL && (flags & MAP_FIXED)) || len <= 0 ||!PAGE_ALIGNED(addr) || !PAGE_ALIGNED(off)) 
	{
		dbg(DBG_PRINT, "(GRADING3D 1)\n");
                return -EINVAL;
        }

        if (!(mode == MAP_SHARED || mode == MAP_PRIVATE)) 
	{
		dbg(DBG_PRINT, "(GRADING3D 1)\n");
                return -EINVAL;
        }

        if ((flags & MAP_ANON) == 0) 
	{

                mmap_file = fget(fd);

                if (mmap_file == NULL) 
		{
			dbg(DBG_PRINT, "(GRADING3D 1)\n");
                        return -EBADF;
                }

                if ((mmap_file->f_mode & FMODE_READ) == 0) 
		{
                        fput(mmap_file);
			dbg(DBG_PRINT, "(GRADING3D 1)\n");
                        return -EACCES;
                }
                if (mode == MAP_SHARED && (prot & PROT_WRITE)) 
		{
                        if ((mmap_file->f_mode & FMODE_WRITE) == 0 || (mmap_file->f_mode & FMODE_APPEND)) 
			{
                                fput(mmap_file);
				dbg(DBG_PRINT, "(GRADING3D 1)\n");
                                return -EACCES;
                        }
			dbg(DBG_PRINT, "(GRADING3D 1)\n");
                }
		dbg(DBG_PRINT, "(GRADING3D 1)\n");
        }

        uint32_t start_vfn = ADDR_TO_PN(addr);
        size_t npages = (len - 1) / PAGE_SIZE + 1;

        if (mmap_file != NULL) 
	{
                vnode_temp = mmap_file->f_vnode;
		dbg(DBG_PRINT, "(GRADING3D 1)\n");
        }

	vp = curproc->p_vmmap;
        retVal = vmmap_map(vp, vnode_temp, start_vfn, npages, prot, flags, off, VMMAP_DIR_HILO, &vmarea_temp);

        if (retVal >= 0) 
	{
                if(addr != NULL)
		{
			addr_temp = (uint32_t)addr;
			dbg(DBG_PRINT, "(GRADING3D 1)\n");
		}
		else
		{
			addr_temp = (uint32_t)PN_TO_ADDR(vmarea_temp->vma_start);
			dbg(DBG_PRINT, "(GRADING3D 1)\n");
		}

                *ret = (void*)addr_temp;

                tlb_flush_range(addr_temp, npages);

                pt_unmap_range(pt_get(), (uint32_t)PN_TO_ADDR(vmarea_temp->vma_start), (uint32_t)PN_TO_ADDR(vmarea_temp->vma_start) + (unsigned int)PAGE_ALIGN_UP(len));
		dbg(DBG_PRINT, "(GRADING3D 1)\n");
        }

        if (mmap_file) 
	{
                fput(mmap_file);
		dbg(DBG_PRINT, "(GRADING3D 1)\n");
        }

        KASSERT(NULL != curproc->p_pagedir);
        dbg(DBG_PRINT, "(GRADING3A 2.a)\n");
	dbg(DBG_PRINT, "(GRADING3D 1)\n");
        return retVal;
}


/*
 * This function implements the munmap(2) syscall.
 *
 * As with do_mmap() it should perform the required error checking,
 * before calling upon vmmap_remove() to do most of the work.
 * Remember to clear the TLB.
 */
int
do_munmap(void *addr, size_t len)
{
      	int np = 0;

      	if (addr == NULL || (!PAGE_ALIGNED(addr)))
	{
		dbg(DBG_PRINT, "(GRADING3D 1)\n");
		return -EINVAL;
	}

	if (!PAGE_ALIGNED((size_t)addr)
		|| (addr != NULL && (size_t)addr < USER_MEM_LOW)
		|| ((size_t)addr + len > USER_MEM_HIGH) || (((size_t)addr + len) <= (size_t)addr))
	{
		dbg(DBG_PRINT, "(GRADING3D 1)\n");
		return -EINVAL;
	}

    	np = ADDR_TO_PN(len);
	if (!PAGE_ALIGNED(len))
	{
		np = ADDR_TO_PN(len) + 1;
		dbg(DBG_PRINT, "(GRADING3D 1)\n");
	}

	tlb_flush_all();

	dbg(DBG_PRINT, "(GRADING3D 1)\n");
	return vmmap_remove(curproc->p_vmmap, ADDR_TO_PN(addr), np);
}

