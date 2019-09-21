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

#include "types.h"
#include "globals.h"
#include "errno.h"

#include "util/debug.h"
#include "util/string.h"

#include "proc/proc.h"
#include "proc/kthread.h"

#include "mm/mm.h"
#include "mm/mman.h"
#include "mm/page.h"
#include "mm/pframe.h"
#include "mm/mmobj.h"
#include "mm/pagetable.h"
#include "mm/tlb.h"

#include "fs/file.h"
#include "fs/vnode.h"

#include "vm/shadow.h"
#include "vm/vmmap.h"

#include "api/exec.h"

#include "main/interrupt.h"

/* Pushes the appropriate things onto the kernel stack of a newly forked thread
 * so that it can begin execution in userland_entry.
 * regs: registers the new thread should have on execution
 * kstack: location of the new thread's kernel stack
 * Returns the new stack pointer on success. */
static uint32_t
fork_setup_stack(const regs_t *regs, void *kstack)
{
        /* Pointer argument and dummy return address, and userland dummy return
         * address */
        uint32_t esp = ((uint32_t) kstack) + DEFAULT_STACK_SIZE - (sizeof(regs_t) + 12);
        *(void **)(esp + 4) = (void *)(esp + 8); /* Set the argument to point to location of struct on stack */
        memcpy((void *)(esp + 8), regs, sizeof(regs_t)); /* Copy over struct */
        return esp;
}


/*
 * The implementation of fork(2). Once this works,
 * you're practically home free. This is what the
 * entirety of Weenix has been leading up to.
 * Go forth and conquer.
 */
int
do_fork(struct regs *regs)
{
	int index;
        file_t *files_list;

	KASSERT(regs != NULL); 
	dbg(DBG_PRINT, "(GRADING3A 7.a)\n");
	dbg(DBG_PRINT, "(GRADING3D 1)\n");

	KASSERT(curproc != NULL); 
	dbg(DBG_PRINT, "(GRADING3A 7.a)\n");
	dbg(DBG_PRINT, "(GRADING3D 1)\n");

	KASSERT(curproc->p_state == PROC_RUNNING);
    	dbg(DBG_PRINT, "(GRADING3A 7.a)\n");
	dbg(DBG_PRINT, "(GRADING3D 1)\n");

        proc_t* newproc = proc_create("child");

	KASSERT(newproc != NULL);
	dbg(DBG_PRINT, "(GRADING3D 1)\n");

        KASSERT(newproc->p_state == PROC_RUNNING);
	dbg(DBG_PRINT, "(GRADING3A 7.a)\n");
	dbg(DBG_PRINT, "(GRADING3D 1)\n");

        KASSERT(newproc->p_pagedir != NULL);  
        dbg(DBG_PRINT, "(GRADING3A 7.a)\n");
	dbg(DBG_PRINT, "(GRADING3D 1)\n");

        vmmap_t *cur_map = curproc->p_vmmap;
        vmmap_t *clone_map = vmmap_clone(cur_map);

	KASSERT(clone_map != NULL);
        dbg(DBG_PRINT, "(GRADING3D 1)\n");  

        newproc->p_vmmap = clone_map;
        newproc->p_vmmap->vmm_proc = newproc;

	newproc->p_start_brk = curproc->p_start_brk;
	newproc->p_brk = curproc->p_brk;

        pt_unmap_range(curproc->p_pagedir, USER_MEM_LOW, USER_MEM_HIGH);
        pt_unmap_range(newproc->p_pagedir, USER_MEM_LOW, USER_MEM_HIGH); 

        tlb_flush_all();

        for (index = 0; index < NFILES; index++)
	{
            	files_list = curproc->p_files[index];
            	newproc->p_files[index] = files_list;
		if (files_list != NULL)
		{
	            	fref(files_list);
               		dbg(DBG_PRINT, "(GRADING3D 1)\n");
		}
            	dbg(DBG_PRINT, "(GRADING3D 1)\n");
        }

        kthread_t* newthr = kthread_clone(curthr);
	KASSERT(newthr != NULL);
	dbg(DBG_PRINT, "(GRADING3D 1)\n");

        KASSERT(newthr->kt_kstack != NULL);
        dbg(DBG_PRINT, "(GRADING3A 7.a)\n");
	dbg(DBG_PRINT, "(GRADING3D 1)\n");
		  
	regs->r_eax = 0;
	newthr->kt_ctx.c_eip = (uintptr_t)userland_entry;
        newthr->kt_ctx.c_esp = fork_setup_stack(regs, newthr->kt_kstack);
	newthr->kt_ctx.c_kstack = (uintptr_t)newthr->kt_kstack;
	newthr->kt_ctx.c_pdptr = newproc->p_pagedir;
	
	KASSERT(newthr->kt_ctx.c_pdptr != NULL);          
	dbg(DBG_PRINT, "(GRADING3D 1)\n");

        list_insert_head(&(newproc->p_threads), &(newthr->kt_plink));
        newthr->kt_proc = newproc;
		  
        sched_make_runnable(newthr);
		 
	dbg(DBG_PRINT, "(GRADING3D 1)\n");  
        return newproc->p_pid;
}
