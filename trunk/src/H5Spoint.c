/*
 * Copyright (C) 1998 NCSA
 *                    All rights reserved.
 *
 * Programmer:  Quincey Koziol <koziol@ncsa.uiuc.edu>
 *              Tuesday, June 16, 1998
 *
 * Purpose:	Point selection data space I/O functions.
 */
#include <H5private.h>
#include <H5Eprivate.h>
#include <H5MMprivate.h>
#include <H5Sprivate.h>
#include <H5Vprivate.h>

/* Interface initialization */
#define PABLO_MASK      H5S_point_mask
#define INTERFACE_INIT  NULL
static intn             interface_initialize_g = FALSE;

/*-------------------------------------------------------------------------
 * Function:	H5S_point_init
 *
 * Purpose:	Initializes iteration information for point selection.
 *
 * Return:	non-negative on success, negative on failure.
 *
 * Programmer:	Quincey Koziol
 *              Tuesday, June 16, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5S_point_init (const struct H5O_layout_t __unused__ *layout,
	       const H5S_t *space, H5S_sel_iter_t *sel_iter)
{
    FUNC_ENTER (H5S_point_init, FAIL);

    /* Check args */
    assert (layout);
    assert (space && H5S_SEL_POINTS==space->select.type);
    assert (sel_iter);

    /* Initialize the number of points to iterate over */
    sel_iter->pnt.elmt_left=space->select.num_elem;

    /* Start at the head of the list of points */
    sel_iter->pnt.curr=space->select.sel_info.pnt_lst->head;
    
    FUNC_LEAVE (SUCCEED);
}

/*--------------------------------------------------------------------------
 NAME
    H5S_point_add
 PURPOSE
    Add a series of elements to a point selection
 USAGE
    herr_t H5S_point_add(space, num_elem, coord)
        H5S_t *space;           IN: Dataspace of selection to modify
        size_t num_elem;        IN: Number of elements in COORD array.
        const hssize_t *coord[];    IN: The location of each element selected
 RETURNS
    SUCCEED/FAIL
 DESCRIPTION
    This function adds elements to the current point selection for a dataspace
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
herr_t H5S_point_add (H5S_t *space, size_t num_elem, const hssize_t *coord[])
{
    H5S_pnt_node_t *top, *curr, *new; /* Point selection nodes */
    uintn i;                 /* Counter */
    herr_t ret_value=FAIL;  /* return value */

    FUNC_ENTER (H5S_point_add, FAIL);

    assert(space);
    assert(num_elem>0);
    assert(coord);

    top=curr=NULL;
    for(i=0; i<num_elem; i++) {
        /* Allocate space for the new node */
        if((new = H5MM_malloc(sizeof(H5S_pnt_node_t)))==NULL)
            HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, FAIL,
                "can't allocate point node");
        if((new->pnt = H5MM_malloc(space->extent.u.simple.rank*sizeof(hssize_t)))==NULL)
            HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, FAIL,
                "can't allocate coordinate information");

        /* Copy over the coordinates */
        HDmemcpy(new->pnt,coord[i],(space->extent.u.simple.rank*sizeof(hssize_t)));

        /* Link into list */
        new->next=NULL;
        if(top==NULL)
            top=new;
        else
            curr->next=new;
        curr=new;
    } /* end for */

    /* Append current list, if there is one */
    if(space->select.sel_info.pnt_lst->head!=NULL)
        curr->next=space->select.sel_info.pnt_lst->head;

    /* Put new list in point selection */
    space->select.sel_info.pnt_lst->head=top;

    ret_value=SUCCEED;
    
done:
    FUNC_LEAVE (ret_value);
}   /* H5S_point_add() */

/*-------------------------------------------------------------------------
 * Function:	H5S_point_favail
 *
 * Purpose:	Figure out the optimal number of elements to transfer to/from the file
 *
 * Return:	non-negative number of elements on success, negative on failure
 *
 * Programmer:	Quincey Koziol
 *              Tuesday, June 16, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
size_t
H5S_point_favail (const H5S_t __unused__ *space, const H5S_sel_iter_t *sel_iter, size_t max)
{
    FUNC_ENTER (H5S_point_favail, FAIL);

    /* Check args */
    assert (space && H5S_SEL_POINTS==space->select.type);
    assert (sel_iter);

    FUNC_LEAVE (MIN(sel_iter->pnt.elmt_left,max));
}   /* H5S_point_favail() */

/*-------------------------------------------------------------------------
 * Function:	H5S_point_fgath
 *
 * Purpose:	Gathers data points from file F and accumulates them in the
 *		type conversion buffer BUF.  The LAYOUT argument describes
 *		how the data is stored on disk and EFL describes how the data
 *		is organized in external files.  ELMT_SIZE is the size in
 *		bytes of a datum which this function treats as opaque.
 *		FILE_SPACE describes the data space of the dataset on disk
 *		and the elements that have been selected for reading (via
 *		hyperslab, etc).  This function will copy at most NELMTS elements.
 *
 *  Notes: This could be optimized by gathering selected elements near (how
 *      near?) each other into one I/O request and then moving the correct
 *      elements into the return buffer
 *
 * Return:	Success:	Number of elements copied.
 *
 *		Failure:	0
 *
 * Programmer:	Quincey Koziol
 *              Tuesday, June 16, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
size_t
H5S_point_fgath (H5F_t *f, const struct H5O_layout_t *layout,
		const struct H5O_compress_t *comp, const struct H5O_efl_t *efl,
		size_t elmt_size, const H5S_t *file_space, H5S_sel_iter_t *file_iter,
		size_t nelmts,
		const H5D_transfer_t xfer_mode, void *_buf/*out*/)
{
    hssize_t	file_offset[H5O_LAYOUT_NDIMS];	/*offset of slab in file*/
    hsize_t	hsize[H5O_LAYOUT_NDIMS];	/*size of hyperslab	*/
    hssize_t	zero[H5O_LAYOUT_NDIMS];		/*zero			*/
    uint8 *buf=(uint8 *)_buf;   /* Alias for pointer arithmetic */
    intn	i;				/*counters		*/
    size_t  num_read;       /* number of elements read into buffer */

    FUNC_ENTER (H5S_point_fgath, 0);

    /* Check args */
    assert (f);
    assert (layout);
    assert (elmt_size>0);
    assert (file_space);
    assert (file_iter);
    assert (nelmts>0);
    assert (buf);

    /* initialize hyperslab size and offset in memory buffer */
    for(i=0; i<layout->ndims; i++) {
        hsize[i]=1;     /* hyperslab size is 1, except for last element */
        zero[i]=0;      /* memory offset is 0 */
    } /* end for */
    hsize[layout->ndims] = elmt_size;

    /* Walk though and request each element we need and put it into the buffer */
    num_read=0;
    while(num_read<nelmts) {
        if(file_iter->pnt.elmt_left>0) {
            /* Copy the location of the point to get */
            HDmemcpy(file_offset,file_iter->pnt.curr->pnt,layout->ndims);
            file_offset[layout->ndims] = 0;

            /* Go read the point */
            if (H5F_arr_read (f, layout, comp, efl, hsize, hsize, zero, file_offset,
                      xfer_mode, buf/*out*/)<0) {
                HRETURN_ERROR (H5E_DATASPACE, H5E_READERROR, 0, "read error");
            }

            /* Increment the offset of the buffer */
            buf+=elmt_size;

            /* Increment the count read */
            num_read++;

            /* Advance the point iterator */
            file_iter->pnt.elmt_left--;
            file_iter->pnt.curr=file_iter->pnt.curr->next;
        } else {
            break;      /* out of elements in the selection */
        } /* end else */
    } /* end while */
    
    FUNC_LEAVE (num_read);
} /* H5S_point_fgath() */

/*-------------------------------------------------------------------------
 * Function:	H5S_point_fscat
 *
 * Purpose:	Scatters dataset elements from the type conversion buffer BUF
 *		to the file F where the data points are arranged according to
 *		the file data space FILE_SPACE and stored according to
 *		LAYOUT and EFL. Each element is ELMT_SIZE bytes.
 *		The caller is requesting that NELMTS elements are copied.
 *
 * Return:	Success:	SUCCEED
 *
 *		Failure:	FAIL
 *
 * Programmer:	Quincey Koziol
 *              Tuesday, June 16, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5S_point_fscat (H5F_t *f, const struct H5O_layout_t *layout,
		const struct H5O_compress_t *comp, const struct H5O_efl_t *efl,
		size_t elmt_size, const H5S_t *file_space, H5S_sel_iter_t *file_iter,
		size_t nelmts,
		const H5D_transfer_t xfer_mode, const void *_buf)
{
    hssize_t	file_offset[H5O_LAYOUT_NDIMS];	/*offset of hyperslab	*/
    hsize_t	hsize[H5O_LAYOUT_NDIMS];	/*size of hyperslab	*/
    hssize_t	zero[H5O_LAYOUT_NDIMS];		/*zero vector		*/
    const uint8 *buf=(const uint8 *)_buf;   /* Alias for pointer arithmetic */
    intn	i;				/*counters		*/
    size_t  num_written;    /* number of elements written from buffer */

    FUNC_ENTER (H5S_point_fscat, FAIL);

    /* Check args */
    assert (f);
    assert (layout);
    assert (elmt_size>0);
    assert (file_space);
    assert (file_iter);
    assert (nelmts>0);
    assert (buf);

    /* initialize hyperslab size and offset in memory buffer */
    for(i=0; i<layout->ndims; i++) {
        hsize[i]=1;     /* hyperslab size is 1, except for last element */
        zero[i]=0;      /* memory offset is 0 */
    } /* end for */
    hsize[layout->ndims] = elmt_size;

    /* Walk though and request each element we need and put it into the buffer */
    num_written=0;
    while(num_written<nelmts) {
        if(file_iter->pnt.elmt_left>0) {
            /* Copy the location of the point to get */
            HDmemcpy(file_offset,file_iter->pnt.curr->pnt,layout->ndims);
            file_offset[layout->ndims] = 0;

            /* Go read the point */
            if (H5F_arr_write (f, layout, comp, efl, hsize, hsize, zero, file_offset,
                      xfer_mode, buf)<0) {
                HRETURN_ERROR (H5E_DATASPACE, H5E_READERROR, 0, "read error");
            }

            /* Increment the offset of the buffer */
            buf+=elmt_size;

            /* Increment the count read */
            num_written++;

            /* Advance the point iterator */
            file_iter->pnt.elmt_left--;
            file_iter->pnt.curr=file_iter->pnt.curr->next;
        } else {
            break;      /* out of elements in the selection */
        } /* end else */
    } /* end while */

    FUNC_LEAVE (num_written);
}   /* H5S_point_fscat() */

/*-------------------------------------------------------------------------
 * Function:	H5S_point_mgath
 *
 * Purpose:	Gathers dataset elements from application memory BUF and
 *		copies them into the data type conversion buffer TCONV_BUF.
 *		Each element is ELMT_SIZE bytes and arranged in application
 *		memory according to MEM_SPACE.  
 *		The caller is requesting that at most NELMTS be gathered.
 *
 * Return:	Success:	Number of elements copied.
 *
 *		Failure:	0
 *
 * Programmer:	Quincey Koziol
 *              Tuesday, June 16, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
size_t
H5S_point_mgath (const void *_buf, size_t elmt_size,
		const H5S_t *mem_space, H5S_sel_iter_t *mem_iter,
		size_t nelmts, void *_tconv_buf/*out*/)
{
    hsize_t	mem_size[H5O_LAYOUT_NDIMS];	/*total size of app buf	*/
    const uint8 *buf=(const uint8 *)_buf;   /* Get local copies for address arithmetic */
    uint8 *tconv_buf=(uint8 *)_tconv_buf;
    hsize_t	acc;				/*accumulator		*/
    intn	space_ndims;			/*dimensionality of space*/
    intn	i;				/*counters		*/
    size_t num_gath;        /* number of elements gathered */

    FUNC_ENTER (H5S_point_mgath, 0);

    /* Check args */
    assert (buf);
    assert (elmt_size>0);
    assert (mem_space && H5S_SEL_POINTS==mem_space->select.type);
    assert (nelmts>0);
    assert (tconv_buf);

    if ((space_ndims=H5S_get_dims (mem_space, mem_size, NULL))<0) {
        HRETURN_ERROR (H5E_DATASPACE, H5E_CANTINIT, 0,
		       "unable to retrieve data space dimensions");
    }

    for(num_gath=0; num_gath<nelmts; num_gath++) {
        if(mem_iter->pnt.elmt_left>0) {
            /* Compute the location of the point to get */
            for(i=0,acc=0; i<space_ndims; i++)
                acc+=mem_size[i]*mem_iter->pnt.curr->pnt[i];

            /* Copy the elements into the type conversion buffer */
            *tconv_buf=*(buf+acc);

            /* Increment the offset of the buffers */
            tconv_buf+=elmt_size;

            /* Advance the point iterator */
            mem_iter->pnt.elmt_left--;
            mem_iter->pnt.curr=mem_iter->pnt.curr->next;
        } else {
            break;      /* out of elements in the selection */
        } /* end else */
    } /* end for */

    FUNC_LEAVE (num_gath);
}   /* H5S_point_mgath() */

/*-------------------------------------------------------------------------
 * Function:	H5S_point_mscat
 *
 * Purpose:	Scatters NELMTS data points from the type conversion buffer
 *		TCONV_BUF to the application buffer BUF.  Each element is
 *		ELMT_SIZE bytes and they are organized in application memory
 *		according to MEM_SPACE.
 *
 * Return:	Success:	SUCCEED
 *
 *		Failure:	FAIL
 *
 * Programmer:	Quincey Koziol
 *              Wednesday, June 17, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5S_point_mscat (const void *_tconv_buf, size_t elmt_size,
		const H5S_t *mem_space, H5S_sel_iter_t *mem_iter,
		size_t nelmts, void *_buf/*out*/)
{
    hsize_t	mem_size[H5O_LAYOUT_NDIMS];	/*total size of app buf	*/
    uint8 *buf=(uint8 *)_buf;   /* Get local copies for address arithmetic */
    const uint8 *tconv_buf=(const uint8 *)_tconv_buf;
    hsize_t	acc;				/*accumulator		*/
    intn	space_ndims;			/*dimensionality of space*/
    intn	i;				/*counters		*/
    size_t num_scat;        /* Number of elements scattered */

    FUNC_ENTER (H5S_point_mscat, FAIL);

    /* Check args */
    assert (tconv_buf);
    assert (elmt_size>0);
    assert (mem_space && H5S_SEL_POINTS==mem_space->select.type);
    assert (nelmts>0);
    assert (buf);

    /*
     * Retrieve hyperslab information to determine what elements are being
     * selected (there might be other selection methods in the future).  We
     * only handle hyperslabs with unit sample because there's currently no
     * way to pass sample information to H5V_hyper_copy().
     */
    if ((space_ndims=H5S_get_dims (mem_space, mem_size, NULL))<0) {
        HRETURN_ERROR (H5E_DATASPACE, H5E_CANTINIT, FAIL,
		       "unable to retrieve data space dimensions");
    }

    for(num_scat=0; num_scat<nelmts; num_scat++) {
        if(mem_iter->pnt.elmt_left>0) {
            /* Compute the location of the point to get */
            for(i=0,acc=0; i<space_ndims; i++)
                acc+=mem_size[i]*mem_iter->pnt.curr->pnt[i];

            /* Copy the elements into the type conversion buffer */
            *(buf+acc)=*tconv_buf;

            /* Increment the offset of the buffers */
            tconv_buf+=elmt_size;

            /* Advance the point iterator */
            mem_iter->pnt.elmt_left--;
            mem_iter->pnt.curr=mem_iter->pnt.curr->next;
        } else {
            break;      /* out of elements in the selection */
        } /* end else */
    } /* end for */

    FUNC_LEAVE (SUCCEED);
}   /* H5S_point_mscat() */

/*--------------------------------------------------------------------------
 NAME
    H5S_point_release
 PURPOSE
    Release point selection information for a dataspace
 USAGE
    herr_t H5S_point_release(space)
        H5S_t *space;       IN: Pointer to dataspace
 RETURNS
    SUCCEED/FAIL
 DESCRIPTION
    Releases all point selection information for a dataspace
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
herr_t
H5S_point_release (H5S_t *space)
{
    FUNC_ENTER (H5S_point_release, FAIL);

    /* Check args */
    assert (space);

    FUNC_LEAVE (SUCCEED);
}   /* H5S_point_release() */

/*--------------------------------------------------------------------------
 NAME
    H5S_point_npoints
 PURPOSE
    Compute number of elements in current selection
 USAGE
    hsize_t H5S_point_npoints(space)
        H5S_t *space;       IN: Pointer to dataspace
 RETURNS
    The number of elements in selection on success, 0 on failure
 DESCRIPTION
    Compute number of elements in current selection.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
hsize_t
H5S_hyper_npoints (const H5S_t *space)
{
    FUNC_ENTER (H5S_hyper_npoints, 0);

    /* Check args */
    assert (space);

    FUNC_LEAVE (space->select.num_elem);
}   /* H5S_hyper_npoints() */
