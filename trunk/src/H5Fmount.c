/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at the  *
 * root level of an installed copy of the electronic HDF5 document set and   *
 * is linked from the top-level documents page.  It can also be found at     *
 * http://hdf.ncsa.uiuc.edu/HDF5/doc/Copyright.html.  If you do not have     *
 * access to either file, you may request a copy from hdfhelp@ncsa.uiuc.edu. *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define H5F_PACKAGE		/*suppress error about including H5Fpkg	  */

/* Interface initialization */
#define H5_INTERFACE_INIT_FUNC	H5F_init_mount_interface


/* Packages needed by this file... */
#include "H5private.h"		/* Generic Functions			*/
#include "H5Eprivate.h"		/* Error handling		  	*/
#include "H5Fpkg.h"             /* File access				*/
#include "H5Gprivate.h"		/* Groups				*/
#include "H5Iprivate.h"		/* IDs			  		*/
#include "H5Pprivate.h"		/* Property lists			*/
#include "H5MMprivate.h"	/* Memory management			*/

/* PRIVATE PROTOTYPES */


/*--------------------------------------------------------------------------
NAME
   H5F_init_mount_interface -- Initialize interface-specific information
USAGE
    herr_t H5T_init_mount_interface()
   
RETURNS
    Non-negative on success/Negative on failure
DESCRIPTION
    Initializes any interface-specific data or routines.  (Just calls
    H5F_init_iterface currently).

--------------------------------------------------------------------------*/
static herr_t
H5F_init_mount_interface(void)
{
    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5F_init_mount_interface)

    FUNC_LEAVE_NOAPI(H5F_init())
} /* H5F_init_mount_interface() */


/*-------------------------------------------------------------------------
 * Function:	H5F_close_mounts
 *
 * Purpose:	Close all mounts for a given file
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *              Saturday, July  2, 2005
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5F_close_mounts(H5F_t *f)
{
    unsigned u;                 /* Local index */
    herr_t ret_value=SUCCEED;   /* Return value */
    
    FUNC_ENTER_NOAPI(H5F_close_mounts, FAIL)

    HDassert(f);

    /* Unmount all child files */
    for (u = 0; u < f->mtab.nmounts; u++) {
        /* Detach the child file from the parent file */
        f->mtab.child[u].file->mtab.parent = NULL;

        /* Close the internal group maintaining the mount point */
        if(H5G_close(f->mtab.child[u].group) < 0)
            HGOTO_ERROR(H5E_FILE, H5E_CANTCLOSEOBJ, FAIL, "can't close child group")

        /* Close the child file */
        if(H5F_close(f->mtab.child[u].file) < 0)
            HGOTO_ERROR(H5E_FILE, H5E_CANTCLOSEFILE, FAIL, "can't close child file")
    } /* end if */
    f->mtab.nmounts = 0;

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5F_close_mounts() */


/*-------------------------------------------------------------------------
 * Function:	H5F_term_unmount_cb 
 *
 * Purpose:	H5F_term_interface' callback function.  This routine
 *              unmounts child files from files that are in the "closing"
 *              state.
 *
 * Programmer:  Quincey Koziol
 *              Thursday, Jun 30, 2005
 *
 * Modification:
 *
 *-------------------------------------------------------------------------
 */
int
H5F_term_unmount_cb(void *obj_ptr, hid_t obj_id, void UNUSED *key)
{
    H5F_t *f = (H5F_t *)obj_ptr;    /* Alias for search info */
    int      ret_value = FALSE;     /* Return value */
 
    FUNC_ENTER_NOAPI_NOINIT(H5F_term_unmount_cb)

    assert(f);

    if(f->mtab.nmounts) {
        /* Unmount all child files */
        if(H5F_close_mounts(f) < 0)
            HGOTO_ERROR(H5E_FILE, H5E_CANTCLOSEFILE, FAIL, "can't unmount child file")

        /* Decrement reference count for file */
        H5I_dec_ref(obj_id);
    } /* end if */
    else
        HGOTO_ERROR(H5E_FILE, H5E_CANTCLOSEFILE, FAIL, "no files to unmount")

done:
    FUNC_LEAVE_NOAPI(ret_value);
} /* end H5F_term_unmount_cb() */


/*-------------------------------------------------------------------------
 * Function:	H5F_mount
 *
 * Purpose:	Mount file CHILD onto the group specified by LOC and NAME,
 *		using mount properties in PLIST.  CHILD must not already be
 *		mouted and must not be a mount ancestor of the mount-point.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Robb Matzke
 *              Tuesday, October  6, 1998
 *
 * Modifications:
 *
 * 	Robb Matzke, 1998-10-14
 *	The reference count for the mounted H5F_t is incremented.
 *
 *      Pedro Vicente, <pvn@ncsa.uiuc.edu> 22 Aug 2002
 *      Added `id to name' support.
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5F_mount(H5G_entry_t *loc, const char *name, H5F_t *child,
	  hid_t UNUSED plist_id, hid_t dxpl_id)
{
    H5G_t	*mount_point = NULL;	/*mount point group		*/
    H5G_entry_t	*mp_ent = NULL;		/*mount point symbol table entry*/
    H5F_t	*ancestor = NULL;	/*ancestor files		*/
    H5F_t	*parent = NULL;		/*file containing mount point	*/
    unsigned	lt, rt, md;		/*binary search indices		*/
    int		cmp;			/*binary search comparison value*/
    H5G_entry_t	*ent = NULL;		/*temporary symbol table entry	*/
    H5G_entry_t  mp_open_ent;     /* entry of moint point to be opened */
    H5RS_str_t  *name_r;                /* Ref-counted version of name */
    herr_t	ret_value = SUCCEED;	/*return value			*/
    
    FUNC_ENTER_NOAPI_NOINIT(H5F_mount)

    assert(loc);
    assert(name && *name);
    assert(child);
    assert(TRUE==H5P_isa_class(plist_id,H5P_MOUNT));

    /*
     * Check that the child isn't mounted, that the mount point exists, and
     * that the mount wouldn't introduce a cycle in the mount tree.
     */
    if (child->mtab.parent)
        HGOTO_ERROR(H5E_FILE, H5E_MOUNT, FAIL, "file is already mounted")
    if (H5G_find(loc, name, NULL, &mp_open_ent/*out*/, H5AC_dxpl_id) < 0)
        HGOTO_ERROR(H5E_SYM, H5E_NOTFOUND, FAIL, "group not found")
    if (NULL==(mount_point=H5G_open(&mp_open_ent, dxpl_id)))
        HGOTO_ERROR(H5E_FILE, H5E_MOUNT, FAIL, "mount point not found")

    parent = H5G_fileof(mount_point);
    mp_ent = H5G_entof(mount_point);
    for (ancestor=parent; ancestor; ancestor=ancestor->mtab.parent) {
	if (ancestor==child)
	    HGOTO_ERROR(H5E_FILE, H5E_MOUNT, FAIL, "mount would introduce a cycle")
    }
    
    /*
     * Use a binary search to locate the position that the child should be
     * inserted into the parent mount table.  At the end of this paragraph
     * `md' will be the index where the child should be inserted.
     */
    lt = md = 0;
    rt=parent->mtab.nmounts;
    cmp = -1;
    while (lt<rt && cmp) {
	md = (lt+rt)/2;
	ent = H5G_entof(parent->mtab.child[md].group);
	cmp = H5F_addr_cmp(mp_ent->header, ent->header);
	if (cmp<0) {
	    rt = md;
	} else if (cmp>0) {
	    lt = md+1;
	}
    }
    if (cmp>0)
        md++;
    if (!cmp)
	HGOTO_ERROR(H5E_FILE, H5E_MOUNT, FAIL, "mount point is already in use")
    
    /* Make room in the table */
    if (parent->mtab.nmounts>=parent->mtab.nalloc) {
	unsigned n = MAX(16, 2*parent->mtab.nalloc);
	H5F_mount_t *x = H5MM_realloc(parent->mtab.child,
				      n*sizeof(parent->mtab.child[0]));
	if (!x)
	    HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, FAIL, "memory allocation failed for mount table")
	parent->mtab.child = x;
	parent->mtab.nalloc = n;
    }

    /* Insert into table */
    HDmemmove(parent->mtab.child+md+1, parent->mtab.child+md,
        (parent->mtab.nmounts-md)*sizeof(parent->mtab.child[0]));
    parent->mtab.nmounts++;
    parent->mtab.child[md].group = mount_point;
    parent->mtab.child[md].file = child;
    child->mtab.parent = parent;
    child->nrefs++;

    /* Search the open IDs and replace names for mount operation */
    /* We pass H5G_UNKNOWN as object type; search all IDs */
    name_r=H5RS_wrap(name);
    assert(name_r);
    if (H5G_replace_name( H5G_UNKNOWN, loc, name_r, NULL, NULL, NULL, OP_MOUNT )<0)
	HGOTO_ERROR(H5E_FILE, H5E_MOUNT, FAIL, "unable to replace name")
    if(H5RS_decr(name_r)<0)
	HGOTO_ERROR(H5E_FILE, H5E_CANTDEC, FAIL, "unable to decrement name string")

done:
    if (ret_value<0 && mount_point)
	if(H5G_close(mount_point)<0)
            HDONE_ERROR(H5E_FILE, H5E_CANTCLOSEOBJ, FAIL, "unable to close mounted group")

    FUNC_LEAVE_NOAPI(ret_value)
}


/*-------------------------------------------------------------------------
 * Function:	H5F_unmount
 *
 * Purpose:	Unmount the child which is mounted at the group specified by
 *		LOC and NAME or fail if nothing is mounted there.  Neither
 *		file is closed.
 *
 *		Because the mount point is specified by name and opened as a
 *		group, the H5G_namei() will resolve it to the root of the
 *		mounted file, not the group where the file is mounted.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Robb Matzke
 *              Tuesday, October  6, 1998
 *
 * Modifications:
 *
 *      Robb Matzke, 1998-10-14
 *      The ref count for the child is decremented by calling H5F_close().
 *
 *      Pedro Vicente, <pvn@ncsa.uiuc.edu> 22 Aug 2002
 *      Added `id to name' support.
 * 
 *-------------------------------------------------------------------------
 */
static herr_t
H5F_unmount(H5G_entry_t *loc, const char *name, hid_t dxpl_id)
{
    H5G_t	*mounted = NULL;	/*mount point group		*/
    H5G_entry_t	*mnt_ent = NULL;	/*mounted symbol table entry	*/
    H5F_t	*child = NULL;		/*mounted file			*/
    H5F_t	*parent = NULL;		/*file where mounted		*/
    H5G_entry_t	*ent = NULL;		/*temporary symbol table entry	*/
    H5G_entry_t mnt_open_ent;       /* entry used to open mount point*/
    herr_t	ret_value = FAIL;	/*return value			*/
    unsigned	i;			/*coutners			*/
    unsigned	lt, rt, md=0;	        /*binary search indices		*/
    int 	cmp;		        /*binary search comparison value*/
    
    FUNC_ENTER_NOAPI_NOINIT(H5F_unmount)

    assert(loc);
    assert(name && *name);

    /*
     * Get the mount point, or more precisely the root of the mounted file.
     * If we get the root group and the file has a parent in the mount tree,
     * then we must have found the mount point.
     */
    if (H5G_find(loc, name, NULL, &mnt_open_ent/*out*/, H5AC_dxpl_id) < 0)
        HGOTO_ERROR(H5E_SYM, H5E_NOTFOUND, FAIL, "group not found")
    if (NULL==(mounted=H5G_open(&mnt_open_ent, dxpl_id)))
        HGOTO_ERROR(H5E_FILE, H5E_MOUNT, FAIL, "mount point not found")
    child = H5G_fileof(mounted);
    mnt_ent = H5G_entof(mounted);
    ent = H5G_entof(child->shared->root_grp);

    if (child->mtab.parent &&
	H5F_addr_eq(mnt_ent->header, ent->header)) {
	/*
	 * We've been given the root group of the child.  We do a reverse
	 * lookup in the parent's mount table to find the correct entry.
	 */
	parent = child->mtab.parent;
	for (i=0; i<parent->mtab.nmounts; i++) {
	    if (parent->mtab.child[i].file==child) {
                /* Search the open IDs replace names to reflect unmount operation */
                if (H5G_replace_name( H5G_UNKNOWN, mnt_ent, mnt_ent->user_path_r, NULL, NULL, NULL, OP_UNMOUNT )<0)
                    HGOTO_ERROR(H5E_SYM, H5E_CANTINIT, FAIL, "unable to replace name ")

		/* Unmount the child */
		parent->mtab.nmounts -= 1;
		if(H5G_close(parent->mtab.child[i].group)<0)
                    HGOTO_ERROR(H5E_FILE, H5E_CANTCLOSEOBJ, FAIL, "unable to close unmounted group")
		child->mtab.parent = NULL;
		if(H5F_close(child)<0)
                    HGOTO_ERROR(H5E_FILE, H5E_CANTCLOSEFILE, FAIL, "unable to close unmounted file")
		HDmemmove(parent->mtab.child+i, parent->mtab.child+i+1,
                    (parent->mtab.nmounts-i)* sizeof(parent->mtab.child[0]));
		ret_value = SUCCEED;
	    }
	}
	assert(ret_value>=0);
	
    } else {
	/*
	 * We've been given the mount point in the parent.  We use a binary
	 * search in the parent to locate the mounted file, if any.
	 */
	parent = child; /*we guessed wrong*/
	lt = 0;
        rt = parent->mtab.nmounts;
	cmp = -1;
	while (lt<rt && cmp) {
	    md = (lt+rt)/2;
	    ent = H5G_entof(parent->mtab.child[md].group);
	    cmp = H5F_addr_cmp(mnt_ent->header, ent->header);
	    if (cmp<0) {
		rt = md;
	    } else {
		lt = md+1;
	    }
	}
	if (cmp)
	    HGOTO_ERROR(H5E_FILE, H5E_MOUNT, FAIL, "not a mount point")

	/* Unmount the child */
	parent->mtab.nmounts -= 1;
	if(H5G_close(parent->mtab.child[md].group)<0)
            HGOTO_ERROR(H5E_FILE, H5E_CANTCLOSEOBJ, FAIL, "unable to close unmounted group")
	parent->mtab.child[md].file->mtab.parent = NULL;
	if(H5F_close(parent->mtab.child[md].file)<0)
            HGOTO_ERROR(H5E_FILE, H5E_CANTCLOSEFILE, FAIL, "unable to close unmounted file")
	HDmemmove(parent->mtab.child+md, parent->mtab.child+md+1,
            (parent->mtab.nmounts-md)*sizeof(parent->mtab.child[0]));
	ret_value = SUCCEED;
    }
    
done:
    if (mounted)
        if(H5G_close(mounted)<0 && ret_value>=0)
	    HDONE_ERROR(H5E_FILE, H5E_CANTCLOSEOBJ, FAIL, "can't close group")

    FUNC_LEAVE_NOAPI(ret_value)
}


/*-------------------------------------------------------------------------
 * Function:	H5F_mountpoint
 *
 * Purpose:	If ENT is a mount point then copy the entry for the root
 *		group of the mounted file into ENT.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Robb Matzke
 *              Tuesday, October  6, 1998
 *
 * Modifications:
 *
 *      Pedro Vicente, <pvn@ncsa.uiuc.edu> 22 Aug 2002
 *      Added `id to name' support.
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5F_mountpoint(H5G_entry_t *find/*in,out*/)
{
    H5F_t	*parent = find->file;
    unsigned	lt, rt, md=0;
    int cmp;
    H5G_entry_t	*ent = NULL;
    herr_t ret_value=SUCCEED;   /* Return value */
    
    FUNC_ENTER_NOAPI(H5F_mountpoint, FAIL)

    assert(find);

    /*
     * The loop is necessary because we might have file1 mounted at the root
     * of file2, which is mounted somewhere in file3.
     */
    do {
	/*
	 * Use a binary search to find the potential mount point in the mount
	 * table for the parent
	 */
	lt = 0;
	rt = parent->mtab.nmounts;
	cmp = -1;
	while (lt<rt && cmp) {
	    md = (lt+rt)/2;
	    ent = H5G_entof(parent->mtab.child[md].group);
	    cmp = H5F_addr_cmp(find->header, ent->header);
	    if (cmp<0) {
		rt = md;
	    } else {
		lt = md+1;
	    }
	}

	/* Copy root info over to ENT */
	if (0==cmp) {
            /* Get the entry for the root group in the child's file */
	    ent = H5G_entof(parent->mtab.child[md].file->shared->root_grp);

            /* Don't lose the user path of the group when we copy the root group's entry */
            if(H5G_ent_copy(find,ent,H5G_COPY_LIMITED)<0)
                HGOTO_ERROR(H5E_FILE, H5E_CANTCOPY, FAIL, "unable to copy group entry")

            /* Switch to child's file */
	    parent = ent->file;
	}
    } while (!cmp);
    
done:
    FUNC_LEAVE_NOAPI(ret_value)
}


/*-------------------------------------------------------------------------
 * Function:	H5F_has_mount
 *
 * Purpose:	Check if a file has mounted files within it.
 *
 * Return:	Success:	TRUE/FALSE
 *		Failure:	Negative
 *
 * Programmer:	Quincey Koziol
 *              Thursday, January  2, 2002
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
htri_t
H5F_has_mount(const H5F_t *file)
{
    htri_t ret_value;   /* Return value */
    
    FUNC_ENTER_NOAPI(H5F_has_mount, FAIL)

    assert(file);

    if(file->mtab.nmounts>0)
        ret_value=TRUE;
    else
        ret_value=FALSE;
    
done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5F_has_mount() */


/*-------------------------------------------------------------------------
 * Function:	H5F_is_mount
 *
 * Purpose:	Check if a file is mounted within another file.
 *
 * Return:	Success:	TRUE/FALSE
 *		Failure:	Negative
 *
 * Programmer:	Quincey Koziol
 *              Thursday, January  2, 2002
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
htri_t
H5F_is_mount(const H5F_t *file)
{
    htri_t ret_value;   /* Return value */
    
    FUNC_ENTER_NOAPI(H5F_is_mount, FAIL)

    assert(file);

    if(file->mtab.parent!=NULL)
        ret_value=TRUE;
    else
        ret_value=FALSE;
    
done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5F_is_mount() */


/*-------------------------------------------------------------------------
 * Function:	H5Fmount
 *
 * Purpose:	Mount file CHILD_ID onto the group specified by LOC_ID and
 *		NAME using mount properties PLIST_ID.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Robb Matzke
 *              Tuesday, October  6, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Fmount(hid_t loc_id, const char *name, hid_t child_id, hid_t plist_id)
{
    H5G_entry_t		*loc = NULL;
    H5F_t		*child = NULL;
    herr_t      ret_value=SUCCEED;       /* Return value */
    
    FUNC_ENTER_API(H5Fmount, FAIL)
    H5TRACE4("e","isii",loc_id,name,child_id,plist_id);

    /* Check arguments */
    if (NULL==(loc=H5G_loc(loc_id)))
	HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a location")
    if (!name || !*name)
	HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "no name")
    if (NULL==(child=H5I_object_verify(child_id,H5I_FILE)))
	HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a file")
    if(H5P_DEFAULT == plist_id)
        plist_id = H5P_MOUNT_DEFAULT;
    else
        if(TRUE != H5P_isa_class(plist_id, H5P_MOUNT))
            HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not property list")

    /* Do the mount */
    if (H5F_mount(loc, name, child, plist_id, H5AC_dxpl_id)<0)
	HGOTO_ERROR(H5E_FILE, H5E_MOUNT, FAIL, "unable to mount file")

done:
    FUNC_LEAVE_API(ret_value)
}


/*-------------------------------------------------------------------------
 * Function:	H5Funmount
 *
 * Purpose:	Given a mount point, dissassociate the mount point's file
 *		from the file mounted there.  Do not close either file.
 *
 *		The mount point can either be the group in the parent or the
 *		root group of the mounted file (both groups have the same
 *		name).  If the mount point was opened before the mount then
 *		it's the group in the parent, but if it was opened after the
 *		mount then it's the root group of the child.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Robb Matzke
 *              Tuesday, October  6, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Funmount(hid_t loc_id, const char *name)
{
    H5G_entry_t		*loc = NULL;
    herr_t      ret_value=SUCCEED;       /* Return value */
    
    FUNC_ENTER_API(H5Funmount, FAIL)
    H5TRACE2("e","is",loc_id,name);

    /* Check args */
    if (NULL==(loc=H5G_loc(loc_id)))
	HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a location")
    if (!name || !*name)
	HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "no name")

    /* Unmount */
    if (H5F_unmount(loc, name, H5AC_dxpl_id)<0)
	HGOTO_ERROR(H5E_FILE, H5E_MOUNT, FAIL, "unable to unmount file")

done:
    FUNC_LEAVE_API(ret_value)
}


/*-------------------------------------------------------------------------
 * Function:	H5F_check_mounts_recurse
 *
 * Purpose:	Helper routine for checking for file mounting hierarchies to
 *              close.
 *
 * Return:	TRUE if entire hierarchy can be closed / FALSE otherwise
 *
 * Programmer:	Quincey Koziol
 *              Saturday, July  2, 2005
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static hbool_t
H5F_check_mounts_recurse(H5F_t *f)
{
    hbool_t ret_value = FALSE;          /* Return value */

    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5F_check_mounts_recurse)

    /* Check if this file is closing and if the only objects left open are
     * the mount points */
    if((f->closing || (f->nrefs == 1 && f->mtab.parent)) && f->nopen_objs == f->mtab.nmounts) {
        unsigned u;

        /* Iterate over files mounted in this file and check if all can be closed */
        for(u = 0; u < f->mtab.nmounts; u++) {
            if(H5G_get_shared_count(f->mtab.child[u].group) > 1
                    || !H5F_check_mounts_recurse(f->mtab.child[u].file))
                HGOTO_DONE(FALSE)
        } /* end for */

        /* Set return value */
        ret_value = TRUE;
    } /* end if */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5F_check_mounts_recurse() */


/*-------------------------------------------------------------------------
 * Function:	H5F_check_mounts
 *
 * Purpose:	Check for file mounting hierarchies that have been created
 *              and that now are composed completely of files that are closing
 *              and have no more open objects in them.
 *
 *              When such a mounting hierarchy is detected, unmount and close
 *              all the files involved.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *              Saturday, July  2, 2005
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5F_check_mounts(H5F_t *f)
{
    herr_t      ret_value=SUCCEED;       /* Return value */
    
    FUNC_ENTER_NOAPI(H5F_check_mounts, FAIL)

    /* Only try to close files for files involved in a mounting hierarchy */
    if(f->mtab.parent || f->mtab.nmounts) {
        H5F_t *top = f;         /* Pointer to the top file in the hierarchy */

        /* Find the top file in the mounting hierarchy */
        while(top->mtab.parent) {
            /* Get out early if we detect that this hierarchy won't close */
            if(top->nopen_objs != top->mtab.nmounts || !top->closing)
                HGOTO_DONE(SUCCEED)

            /* Advance toward the top of the hierarchy */
            top = top->mtab.parent;
        } /* end while */

        /* Check for closing the hierarchy */
        if(H5F_check_mounts_recurse(top)) {
            /* Unmount all child files */
            if(H5F_close_mounts(top) < 0)
                HGOTO_ERROR(H5E_FILE, H5E_CANTCLOSEFILE, FAIL, "can't unmount child file")

            if(H5I_dec_ref(top->closing) < 0)
                HGOTO_ERROR(H5E_FILE, H5E_CANTDEC, FAIL, "can't decrement file closing ID")
        } /* end if */
    } /* end if */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5F_check_mounts() */

