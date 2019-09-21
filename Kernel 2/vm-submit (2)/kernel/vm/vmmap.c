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
#include "errno.h"
#include "globals.h"

#include "vm/vmmap.h"
#include "vm/shadow.h"
#include "vm/anon.h"

#include "proc/proc.h"

#include "util/debug.h"
#include "util/list.h"
#include "util/string.h"
#include "util/printf.h"

#include "fs/vnode.h"
#include "fs/file.h"
#include "fs/fcntl.h"
#include "fs/vfs_syscall.h"

#include "mm/slab.h"
#include "mm/page.h"
#include "mm/mm.h"
#include "mm/mman.h"
#include "mm/mmobj.h"

static slab_allocator_t *vmmap_allocator;
static slab_allocator_t *vmarea_allocator;

void
vmmap_init(void)
{
        vmmap_allocator = slab_allocator_create("vmmap", sizeof(vmmap_t));
        KASSERT(NULL != vmmap_allocator && "failed to create vmmap allocator!");
        vmarea_allocator = slab_allocator_create("vmarea", sizeof(vmarea_t));
        KASSERT(NULL != vmarea_allocator && "failed to create vmarea allocator!");
}

vmarea_t *
vmarea_alloc(void)
{
        vmarea_t *newvma = (vmarea_t *) slab_obj_alloc(vmarea_allocator);
        if (newvma) {
                newvma->vma_vmmap = NULL;
        }
        return newvma;
}

void
vmarea_free(vmarea_t *vma)
{
        KASSERT(NULL != vma);
        slab_obj_free(vmarea_allocator, vma);
}

/* a debugging routine: dumps the mappings of the given address space. */
size_t
vmmap_mapping_info(const void *vmmap, char *buf, size_t osize)
{
        KASSERT(0 < osize);
        KASSERT(NULL != buf);
        KASSERT(NULL != vmmap);

        vmmap_t *map = (vmmap_t *)vmmap;
        vmarea_t *vma;
        ssize_t size = (ssize_t)osize;

        int len = snprintf(buf, size, "%21s %5s %7s %8s %10s %12s\n",
                           "VADDR RANGE", "PROT", "FLAGS", "MMOBJ", "OFFSET",
                           "VFN RANGE");

        list_iterate_begin(&map->vmm_list, vma, vmarea_t, vma_plink) {
                size -= len;
                buf += len;
                if (0 >= size) {
                        goto end;
                }

                len = snprintf(buf, size,
                               "%#.8x-%#.8x  %c%c%c  %7s 0x%p %#.5x %#.5x-%#.5x\n",
                               vma->vma_start << PAGE_SHIFT,
                               vma->vma_end << PAGE_SHIFT,
                               (vma->vma_prot & PROT_READ ? 'r' : '-'),
                               (vma->vma_prot & PROT_WRITE ? 'w' : '-'),
                               (vma->vma_prot & PROT_EXEC ? 'x' : '-'),
                               (vma->vma_flags & MAP_SHARED ? " SHARED" : "PRIVATE"),
                               vma->vma_obj, vma->vma_off, vma->vma_start, vma->vma_end);
        } list_iterate_end();

end:
        if (size <= 0) {
                size = osize;
                buf[osize - 1] = '\0';
        }
        /*
        KASSERT(0 <= size);
        if (0 == size) {
                size++;
                buf--;
                buf[0] = '\0';
        }
        */
        return osize - size;
}

/* Create a new vmmap, which has no vmareas and does
 * not refer to a process. */
vmmap_t *
vmmap_create(void)
{
        vmmap_t* new_vmmap = (vmmap_t *) slab_obj_alloc(vmmap_allocator);

        if (new_vmmap)
	{
            list_init(&(new_vmmap -> vmm_list));
            new_vmmap -> vmm_proc = NULL;
            dbg(DBG_PRINT, "(GRADING3D 1)\n");
        }
        dbg(DBG_PRINT, "(GRADING3D 1)\n");
        return new_vmmap;
}

/* Removes all vmareas from the address space and frees the
 * vmmap struct. */
void
vmmap_destroy(vmmap_t *map)
{
        KASSERT(NULL != map); 
        dbg(DBG_PRINT, "(GRADING3A 3.a)\n");
	dbg(DBG_PRINT, "(GRADING3D 1)\n");

        vmarea_t *vmarea_temp;
	mmobj_t *mmobj_temp_1 = NULL;

        list_iterate_begin(&map->vmm_list, vmarea_temp, vmarea_t, vma_plink) 
	{
		list_remove(&(vmarea_temp->vma_olink));
		
		mmobj_t *mmobj_temp_2 = vmarea_temp->vma_obj;

		while (mmobj_temp_2)
		{
			if(mmobj_temp_2->mmo_shadowed != NULL) 
			{
				mmobj_temp_1 = mmobj_temp_2->mmo_shadowed;
				if((mmobj_temp_2->mmo_refcount - 1) != mmobj_temp_2->mmo_nrespages)
				{
					mmobj_temp_2->mmo_ops->put(mmobj_temp_2);
                			dbg(DBG_PRINT, "(GRADING3D 1)\n");
					break;
				}
				else
				{
					mmobj_temp_2->mmo_ops->put(mmobj_temp_2);
					mmobj_temp_2 = mmobj_temp_1;
                			dbg(DBG_PRINT, "(GRADING3D 1)\n");
				}
            			dbg(DBG_PRINT, "(GRADING3D 1)\n");
			}
			else
			{
				mmobj_temp_2->mmo_ops->put(mmobj_temp_2);
           			dbg(DBG_PRINT, "(GRADING3D 1)\n");
				break;
			}
        		dbg(DBG_PRINT, "(GRADING3D 1)\n");
		}
        	dbg(DBG_PRINT, "(GRADING3D 1)\n");

                vmarea_free(vmarea_temp);
                dbg(DBG_PRINT, "(GRADING3D 1)\n");		 
        } list_iterate_end();

        slab_obj_free(vmmap_allocator, map);
        dbg(DBG_PRINT, "(GRADING3D 1)\n");
}

/* Add a vmarea to an address space. Assumes (i.e. asserts to some extent)
 * the vmarea is valid.  This involves finding where to put it in the list
 * of VM areas, and adding it. Don't forget to set the vma_vmmap for the
 * area. */
void
vmmap_insert(vmmap_t *map, vmarea_t *newvma)
{
        KASSERT(NULL != map && NULL != newvma);
        dbg(DBG_PRINT, "(GRADING3A 3.b)\n");
	dbg(DBG_PRINT, "(GRADING3D 1)\n");

	KASSERT(NULL == newvma->vma_vmmap);
        dbg(DBG_PRINT, "(GRADING3A 3.b)\n");
	dbg(DBG_PRINT, "(GRADING3D 1)\n");

	KASSERT(newvma->vma_start < newvma->vma_end);
	dbg(DBG_PRINT, "(GRADING3A 3.b)\n");
       	dbg(DBG_PRINT, "(GRADING3D 1)\n");

	KASSERT(ADDR_TO_PN(USER_MEM_LOW) <= newvma->vma_start && ADDR_TO_PN(USER_MEM_HIGH) >= newvma->vma_end);
	dbg(DBG_PRINT, "(GRADING3A 3.b)\n");
        dbg(DBG_PRINT, "(GRADING3D 1)\n");

	vmarea_t *iterator = NULL;
	list_iterate_begin(&map->vmm_list, iterator, vmarea_t, vma_plink)
	{
		dbg(DBG_PRINT, "(GRADING3D 1)\n");
		if(newvma->vma_start < iterator->vma_start)
		{
			list_insert_before(&iterator->vma_plink, &newvma->vma_plink);
			newvma->vma_vmmap = map;
			dbg(DBG_PRINT, "(GRADING3D 1)\n");
			return;
		}
		dbg(DBG_PRINT, "(GRADING3D 1)\n");
	}list_iterate_end();
	

	list_insert_tail(&map->vmm_list, &newvma->vma_plink);
	newvma->vma_vmmap = map;
	dbg(DBG_PRINT, "(GRADING3D 1)\n");
}

/* Find a contiguous range of free virtual pages of length npages in
 * the given address space. Returns starting vfn for the range,
 * without altering the map. Returns -1 if no such range exists.
 *
 * Your algorithm should be first fit. If dir is VMMAP_DIR_HILO, you
 * should find a gap as high in the address space as possible; if dir
 * is VMMAP_DIR_LOHI, the gap should be as low as possible. */
int
vmmap_find_range(vmmap_t *map, uint32_t npages, int dir)
{
        vmarea_t *vmarea;
        int start = ADDR_TO_PN(USER_MEM_HIGH);
        int beforeend = ADDR_TO_PN(USER_MEM_LOW);
		  KASSERT(dir == VMMAP_DIR_HILO);
		  dbg(DBG_PRINT, "(GRADING3D 1)\n");
            list_iterate_reverse(&map->vmm_list, vmarea, vmarea_t, vma_plink) 
         {
                    if (start - vmarea->vma_end >= npages)
                   {
                       dbg(DBG_PRINT, "(GRADING3D 1)\n");
                        return start - npages;
                    }else
                    {
                       start = vmarea->vma_start;
                       dbg(DBG_PRINT, "(GRADING3D 1)\n");
                    }
                    dbg(DBG_PRINT, "(GRADING3D 1)\n");
                    
            } list_iterate_end();
				if (start - npages >= ADDR_TO_PN(USER_MEM_LOW))
                    {
                    dbg(DBG_PRINT, "(GRADING3D 1)\n");
		    return start - npages;
		    }
        dbg(DBG_PRINT, "(GRADING3D 2)\n");
        return -1;
}

/* Find the vm_area that vfn lies in. Simply scan the address space
 * looking for a vma whose range covers vfn. If the page is unmapped,
 * return NULL. */
vmarea_t *
vmmap_lookup(vmmap_t *map, uint32_t vfn)
{
        KASSERT(NULL != map);
	dbg(DBG_PRINT, "(GRADING3A 3.c)\n");
        dbg(DBG_PRINT, "(GRADING3D 1)\n");

	vmarea_t *vmarea;
        list_iterate_begin(&map->vmm_list, vmarea, vmarea_t, vma_plink) {
		dbg(DBG_PRINT, "(GRADING3D 1)\n");
		if((vfn >= vmarea->vma_start) && (vfn < vmarea->vma_end))
		{
			dbg(DBG_PRINT, "(GRADING3D 1)\n");
			return vmarea;	
		}
		dbg(DBG_PRINT, "(GRADING3D 1)\n");
	} list_iterate_end();
	dbg(DBG_PRINT, "(GRADING3D 2)\n");
	return NULL; 
}

/* Allocates a new vmmap containing a new vmarea for each area in the
 * given map. The areas should have no mmobjs set yet. Returns pointer
 * to the new vmmap on success, NULL on failure. This function is
 * called when implementing fork(2). */
vmmap_t *
vmmap_clone(vmmap_t *map)
{
        vmmap_t * clonemap;
        clonemap = vmmap_create();
        
        KASSERT(clonemap != NULL);
	dbg(DBG_PRINT, "(GRADING3D 1)\n");
        vmarea_t *curr;
        vmarea_t *clonevmarea;
       mmobj_t* oldobj = NULL;
        list_iterate_begin(&map->vmm_list, curr, vmarea_t, vma_plink)
         {
            clonevmarea =  vmarea_alloc();
	    KASSERT(curr != NULL);
            dbg(DBG_PRINT, "(GRADING3D 1)\n");

            clonevmarea->vma_start = curr->vma_start;
            clonevmarea->vma_end = curr->vma_end;
            clonevmarea->vma_flags = curr->vma_flags;
            clonevmarea->vma_prot = curr->vma_prot;
            clonevmarea->vma_off = curr->vma_off;
            clonevmarea->vma_vmmap = clonemap;

            list_insert_tail(&(clonemap->vmm_list),&(clonevmarea->vma_plink));
            dbg(DBG_PRINT, "(GRADING3D 1)\n");


          if (clonevmarea->vma_flags & MAP_SHARED)
          {
                clonevmarea->vma_obj = curr->vma_obj;
                clonevmarea->vma_obj->mmo_ops->ref(curr->vma_obj);
                list_insert_tail(&(curr->vma_obj->mmo_un.mmo_vmas),&(clonevmarea->vma_olink));
               dbg(DBG_PRINT, "(GRADING3D 1)\n");
            }
            else if (clonevmarea->vma_flags & MAP_PRIVATE)
             {
                oldobj = curr->vma_obj;
                clonevmarea->vma_obj = shadow_create(); 
		 KASSERT(clonevmarea->vma_obj != NULL);
		dbg(DBG_PRINT, "(GRADING3D 1)\n");
		 KASSERT(oldobj->mmo_shadowed != NULL); 
                    dbg(DBG_PRINT, "(GRADING3D 1)\n");	
                	clonevmarea->vma_obj->mmo_un.mmo_bottom_obj = oldobj->mmo_un.mmo_bottom_obj;
				 	
					 	dbg(DBG_PRINT, "(GRADING3D 1)\n");
					 
                clonevmarea->vma_obj->mmo_shadowed = oldobj;
                oldobj->mmo_ops->ref(oldobj);
                mmobj_t *newobj = shadow_create(); 
		 KASSERT(newobj != NULL);
dbg(DBG_PRINT, "(GRADING3D 1)\n");
		KASSERT(oldobj->mmo_shadowed != NULL); 
                  dbg(DBG_PRINT, "(GRADING3D 1)\n");
	    newobj->mmo_un.mmo_bottom_obj = oldobj->mmo_un.mmo_bottom_obj;
		 newobj->mmo_shadowed = oldobj;
                curr->vma_obj = newobj; 
            	 list_insert_tail(&(curr->vma_obj->mmo_un.mmo_bottom_obj->mmo_un.mmo_vmas),&(clonevmarea->vma_olink));
               
            }
              dbg(DBG_PRINT, "(GRADING3D 1)\n");
                
        } list_iterate_end();
       dbg(DBG_PRINT, "(GRADING3D 1)\n");
        return clonemap;
}

/* Insert a mapping into the map starting at lopage for npages pages.
 * If lopage is zero, we will find a range of virtual addresses in the
 * process that is big enough, by using vmmap_find_range with the same
 * dir argument.  If lopage is non-zero and the specified region
 * contains another mapping that mapping should be unmapped.
 *
 * If file is NULL an anon mmobj will be used to create a mapping
 * of 0's.  If file is non-null that vnode's file will be mapped in
 * for the given range.  Use the vnode's mmap operation to get the
 * mmobj for the file; do not assume it is file->vn_obj. Make sure all
 * of the area's fields except for vma_obj have been set before
 * calling mmap.
 *
 * If MAP_PRIVATE is specified set up a shadow object for the mmobj.
 *
 * All of the input to this function should be valid (KASSERT!).
 * See mmap(2) for for description of legal input.
 * Note that off should be page aligned.
 *
 * Be very careful about the order operations are performed in here. Some
 * operation are impossible to undo and should be saved until there
 * is no chance of failure.
 *
 * If 'new' is non-NULL a pointer to the new vmarea_t should be stored in it.
 */
int
vmmap_map(vmmap_t *map, vnode_t *file, uint32_t lopage, uint32_t npages,
          int prot, int flags, off_t off, int dir, vmarea_t **new)
{
        KASSERT(NULL != map);
	dbg(DBG_PRINT, "(GRADING3A.d)\n");
	dbg(DBG_PRINT, "(GRADING3D 1)\n");

	KASSERT(0 < npages);
	dbg(DBG_PRINT, "(GRADING3A.d)\n");
	dbg(DBG_PRINT, "(GRADING3D 1)\n");

	KASSERT((MAP_SHARED & flags) || (MAP_PRIVATE & flags));
	dbg(DBG_PRINT, "(GRADING3A.d)\n");
	dbg(DBG_PRINT, "(GRADING3D 1)\n");

	KASSERT((0 == lopage) || (ADDR_TO_PN(USER_MEM_LOW) <= lopage));
	dbg(DBG_PRINT, "(GRADING3A.d)\n");
	dbg(DBG_PRINT, "(GRADING3D 1)\n");

	KASSERT((0 == lopage) || (ADDR_TO_PN(USER_MEM_HIGH) >= (lopage + npages)));
	dbg(DBG_PRINT, "(GRADING3A.d)\n");
	dbg(DBG_PRINT, "(GRADING3D 1)\n");

	KASSERT(PAGE_ALIGNED(off));
	dbg(DBG_PRINT, "(GRADING3A.d)\n");
	dbg(DBG_PRINT, "(GRADING3D 1)\n");

	vmarea_t *va = vmarea_alloc();
	mmobj_t *obj = NULL;
	int check_range=0;
	int retval=0;

	if(lopage == 0)
	{
    		if ((check_range = vmmap_find_range(map, npages, dir)) < 0)
		{
			dbg(DBG_PRINT, "(GRADING3D 1)\n");
      			return check_range;
		}
		va->vma_start = check_range;
		dbg(DBG_PRINT, "(GRADING3D 1)\n");
	}
	else
	{
		if(!vmmap_is_range_empty(map, lopage, npages))
		{
  			retval = vmmap_remove(map, lopage, npages);
			dbg(DBG_PRINT, "(GRADING3D 1)\n");
  		}
	
		va->vma_start = lopage;
		dbg(DBG_PRINT, "(GRADING3D 1)\n");
	}

	va->vma_end = va->vma_start + npages;
	va->vma_prot = prot;
	va->vma_off = off;
	va->vma_flags = flags;
	va->vma_obj = NULL;


 	if(file != NULL)
	{
   		retval = file->vn_ops->mmap(file, va, &(va->vma_obj));
		KASSERT(retval == 0);
		dbg(DBG_PRINT, "(GRADING3D 1)\n");
 	}
 	else
	{
   		va->vma_obj = anon_create();
		dbg(DBG_PRINT, "(GRADING3D 1)\n");
	}

	list_insert_tail(&(va->vma_obj->mmo_un.mmo_vmas), &va->vma_olink);

	if((MAP_TYPE & flags) & MAP_PRIVATE)
	{
		obj = va->vma_obj;
		va->vma_obj = shadow_create();
		va->vma_obj->mmo_un.mmo_bottom_obj = obj;
		va->vma_obj->mmo_shadowed = obj;
		dbg(DBG_PRINT, "(GRADING3D 1)\n");
	}


	if(new != NULL)
	{
		*new = va;
		dbg(DBG_PRINT, "(GRADING3D 1)\n");
 	}
 	vmmap_insert(map, va);
	dbg(DBG_PRINT, "(GRADING3D 1)\n");
 	return 0;
}

/*
 * We have no guarantee that the region of the address space being
 * unmapped will play nicely with our list of vmareas.
 *
 * You must iterate over each vmarea that is partially or wholly covered
 * by the address range [addr ... addr+len). The vm-area will fall into one
 * of four cases, as illustrated below:
 *
 * key:
 *          [             ]   Existing VM Area
 *        *******             Region to be unmapped
 *
 * Case 1:  [   ******    ]
 * The region to be unmapped lies completely inside the vmarea. We need to
 * split the old vmarea into two vmareas. be sure to increment the
 * reference count to the file associated with the vmarea.
 *
 * Case 2:  [      *******]**
 * The region overlaps the end of the vmarea. Just shorten the length of
 * the mapping.
 *
 * Case 3: *[*****        ]
 * The region overlaps the beginning of the vmarea. Move the beginning of
 * the mapping (remember to update vma_off), and shorten its length.
 *
 * Case 4: *[*************]**
 * The region completely contains the vmarea. Remove the vmarea from the
 * list.
 */
int
vmmap_remove(vmmap_t *map, uint32_t lopage, uint32_t npages)
{
        vmarea_t *vma; 
	uint32_t end;
        
	list_iterate_begin(&map->vmm_list, vma, vmarea_t, vma_plink)
	{
            	if(vma->vma_start < lopage && vma->vma_end > (lopage + npages))
		{
			vmarea_t *new_vmarea;
			end = vma->vma_end;
                	vma->vma_end = lopage;
                	new_vmarea = vmarea_alloc();
			
			KASSERT(new_vmarea != NULL);
                     	dbg(DBG_PRINT, "(GRADING3D 2)\n");

                	new_vmarea->vma_start = lopage + npages;
                	new_vmarea->vma_end = end;
                	new_vmarea->vma_prot = vma->vma_prot;
                	new_vmarea->vma_flags = vma->vma_flags;
                	new_vmarea->vma_obj = vma->vma_obj;
                	new_vmarea->vma_off = (vma->vma_end - vma->vma_start + npages) + vma->vma_off; 
			vmmap_insert(map, new_vmarea);

			KASSERT(vma->vma_obj->mmo_shadowed != NULL);
                     	dbg(DBG_PRINT, "(GRADING3D 2)\n");
					 
			list_insert_tail(&(vma->vma_obj->mmo_un.mmo_bottom_obj->mmo_un.mmo_vmas), &new_vmarea->vma_olink);
			dbg(DBG_PRINT, "(GRADING3D 2)\n");
					
                	pt_unmap_range(curproc->p_pagedir, (uintptr_t)PN_TO_ADDR(lopage), (uintptr_t)PN_TO_ADDR(lopage+npages));

                	vma->vma_obj->mmo_ops->ref(vma->vma_obj); 
                	dbg(DBG_PRINT, "(GRADING3D 2)\n");
            	}
		else if(vma->vma_start < lopage && lopage < vma->vma_end && vma->vma_end <= (lopage + npages) )
		{
                	pt_unmap_range(curproc->p_pagedir, (uintptr_t)PN_TO_ADDR(lopage), (uintptr_t)PN_TO_ADDR(vma->vma_end));
                	vma->vma_end = lopage;
               		dbg(DBG_PRINT, "(GRADING3D 1)\n");
            	}
		else if(vma->vma_start >= lopage && vma->vma_end > (lopage + npages) && vma->vma_start < (lopage + npages))
		{
                	pt_unmap_range(curproc->p_pagedir, (uintptr_t)PN_TO_ADDR(vma->vma_start), (uintptr_t)PN_TO_ADDR(lopage+npages));
			vma->vma_off += (lopage + npages - vma->vma_start);
                	vma->vma_start = lopage + npages;
               		dbg(DBG_PRINT, "(GRADING3D 2)\n");
            	}
		else if(vma->vma_start >= lopage && vma->vma_end <= (lopage + npages))
		{
			list_remove(&(vma->vma_olink));
			mmobj_t *mmobj_temp_1 = NULL;	 
			mmobj_t *mmobj_temp_2 = vma->vma_obj;

			while (mmobj_temp_2)
			{
				if(mmobj_temp_2->mmo_shadowed != NULL) 
				{
					mmobj_temp_1 = mmobj_temp_2->mmo_shadowed;
					if((mmobj_temp_2->mmo_refcount - 1) != mmobj_temp_2->mmo_nrespages)
					{
						mmobj_temp_2->mmo_ops->put(mmobj_temp_2);
                				dbg(DBG_PRINT, "(GRADING3D 1)\n");
						break;
					}
					else
					{
						mmobj_temp_2->mmo_ops->put(mmobj_temp_2);
						mmobj_temp_2 = mmobj_temp_1;
                				dbg(DBG_PRINT, "(GRADING3D 1)\n");
					}
            				dbg(DBG_PRINT, "(GRADING3D 1)\n");
				}
				else
				{
					mmobj_temp_2->mmo_ops->put(mmobj_temp_2);
           				dbg(DBG_PRINT, "(GRADING3D 1)\n");
					break;
				}
        			dbg(DBG_PRINT, "(GRADING3D 1)\n");
			}
			dbg(DBG_PRINT, "(GRADING3D 1)\n");
					 
			list_remove(&(vma->vma_plink));
                	pt_unmap_range(curproc->p_pagedir, (uintptr_t)PN_TO_ADDR(vma->vma_start), (uintptr_t)PN_TO_ADDR(vma->vma_end));
                	vmarea_free(vma);
                	dbg(DBG_PRINT, "(GRADING3D 1)\n");
            	}
            	dbg(DBG_PRINT, "(GRADING3D 1)\n");

        }list_iterate_end();
        
	dbg(DBG_PRINT, "(GRADING3D 1)\n");
        return 0; 
}

/*
 * Returns 1 if the given address space has no mappings for the
 * given range, 0 otherwise.
 */
int
vmmap_is_range_empty(vmmap_t *map, uint32_t startvfn, uint32_t npages)
{
        uint32_t vmmemlow = ADDR_TO_PN(USER_MEM_LOW);
        uint32_t endvfn = startvfn + npages;


    	KASSERT((startvfn < endvfn) && (ADDR_TO_PN(USER_MEM_LOW) <= startvfn) && (ADDR_TO_PN(USER_MEM_HIGH) >= endvfn));
        dbg(DBG_PRINT, "(GRADING3A 3.e)\n");
        dbg(DBG_PRINT, "(GRADING3D 1)\n");

   vmarea_t* vmptr = NULL;

  list_iterate_begin(&map->vmm_list, vmptr , vmarea_t, vma_plink)
    {
      if((vmptr->vma_start >= endvfn)  && (  vmmemlow <= startvfn) )
	{  
          dbg(DBG_PRINT, "(GRADING3D 1)\n");
	  return 1;
	}
      else if(vmmemlow > startvfn || ((vmptr->vma_start > startvfn) && ( vmptr->vma_start < endvfn) ))
	{ 
          dbg(DBG_PRINT, "(GRADING3D 1)\n");
    	  return 0;
	}	  
      vmmemlow = vmptr->vma_end;
			  
    }list_iterate_end();

  if( (vmmemlow <= startvfn)  && (ADDR_TO_PN(USER_MEM_HIGH) >= endvfn) )
    { 
          dbg(DBG_PRINT, "(GRADING3D 1)\n");
	  return 1;
    }
			
	dbg(DBG_PRINT, "(GRADING3D 1)\n");
	return 0;
}

/* Read into 'buf' from the virtual address space of 'map' starting at
 * 'vaddr' for size 'count'. To do so, you will want to find the vmareas
 * to read from, then find the pframes within those vmareas corresponding
 * to the virtual addresses you want to read, and then read from the
 * physical memory that pframe points to. You should not check permissions
 * of the areas. Assume (KASSERT) that all the areas you are accessing exist.
 * Returns 0 on success, -errno on error.
 */
int
vmmap_read(vmmap_t *map, const void *vaddr, void *buf, size_t count)
{
	uint32_t vfn = ADDR_TO_PN(vaddr);
	uintptr_t offset = PAGE_OFFSET(vaddr);
        pframe_t *pframeread;
	//int pagestat;
	while(count > 0)
	{	
		vmarea_t *vmarearead = vmmap_lookup(map,vfn);
		
		pframe_lookup(vmarearead->vma_obj, vmarearead->vma_off+vfn-vmarearead->vma_start,0, &pframeread);
		/*if(pagestat != 0)
               {
			dbg(DBG_PRINT, "(GRADING3D 1)\n");
			dbg(DBG_TEMP, "******************************************************check46\n");
			return pagestat;
		}*/

		char *tempaddress = (char*)pframeread->pf_addr + offset;
		uintptr_t spaceleft = PAGE_SIZE - offset;
		uintptr_t bytesToread = (count > spaceleft)? spaceleft : count;
		
		memcpy(buf, tempaddress, bytesToread);
		buf = (void*)((char*)buf + bytesToread);
		count = count- bytesToread;
		vfn = vfn+1;
		offset = 0;
		dbg(DBG_PRINT, "(GRADING3D 1)\n");
	}
	dbg(DBG_PRINT, "(GRADING3D 1)\n");
        return 0;
}

/* Write from 'buf' into the virtual address space of 'map' starting at
 * 'vaddr' for size 'count'. To do this, you will need to find the correct
 * vmareas to write into, then find the correct pframes within those vmareas,
 * and finally write into the physical addresses that those pframes correspond
 * to. You should not check permissions of the areas you use. Assume (KASSERT)
 * that all the areas you are accessing exist. Remember to dirty pages!
 * Returns 0 on success, -errno on error.
 */
int
vmmap_write(vmmap_t *map, void *vaddr, const void *buf, size_t count)
{
        KASSERT((size_t)vaddr + count < USER_MEM_HIGH);
        dbg(DBG_PRINT, "(GRADING3D 1)\n");

	vmarea_t *vmarea_temp = NULL;
        int start_vfn = ADDR_TO_PN(vaddr);
        int end_vfn = ADDR_TO_PN((size_t)vaddr + count);
        int page_no;
	struct pframe *pframe_temp;
        size_t c_oset = 0;
	size_t offset = 0;

        for (int index = start_vfn; index <= end_vfn; index++)
	{
            	vmarea_temp = vmmap_lookup(map, ADDR_TO_PN(vaddr));

		KASSERT(NULL != vmarea_temp); 
          	dbg(DBG_PRINT, "(GRADING3D 1)\n");
				
            	offset = 0;
            	if (index == start_vfn)
		{                
			offset = PAGE_OFFSET(vaddr);
                	dbg(DBG_PRINT, "(GRADING3D 1)\n");
            	}
            
            	page_no = index - vmarea_temp->vma_start + vmarea_temp->vma_off;
		int retVal = vmarea_temp->vma_obj->mmo_ops->lookuppage(vmarea_temp->vma_obj, page_no, 1, &pframe_temp);
            	
		KASSERT(retVal == 0); 
                dbg(DBG_PRINT, "(GRADING3D 1)\n");

                if(index == end_vfn)
		{
	  		KASSERT(vmarea_temp->vma_obj->mmo_ops->dirtypage(vmarea_temp->vma_obj, pframe_temp) == 0);
                    	dbg(DBG_PRINT, "(GRADING3D 1)\n");
                    	memcpy((void*)((size_t)pframe_temp->pf_addr + offset), (void*)((size_t)buf+ c_oset),  count - c_oset);
                    	dbg(DBG_PRINT, "(GRADING3D 1)\n");
                }
		else
		{
			KASSERT(vmarea_temp->vma_obj->mmo_ops->dirtypage(vmarea_temp->vma_obj, pframe_temp) == 0);
                    	dbg(DBG_PRINT, "(GRADING3D 1)\n");
                    	memcpy((void*)((size_t)pframe_temp->pf_addr + offset), (void*)((size_t)buf+ c_oset), PAGE_SIZE - offset);
                    	c_oset += PAGE_SIZE - offset;
                   	dbg(DBG_PRINT, "(GRADING3D 1)\n");
                }
            	dbg(DBG_PRINT, "(GRADING3D 1)\n");
        }
       	
	dbg(DBG_PRINT, "(GRADING3D 1)\n");
        return 0;
}
