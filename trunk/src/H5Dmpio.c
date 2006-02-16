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

/*
 * Programmer:  rky 980813
 *
 * Purpose:	Functions to read/write directly between app buffer and file.
 *
 * 		Beware of the ifdef'ed print statements.
 *		I didn't make them portable.
 */

/****************/
/* Module Setup */
/****************/

#define H5D_PACKAGE		/*suppress error about including H5Dpkg	  */
/*#define KENT */


/***********/
/* Headers */
/***********/
#include "H5private.h"		/* Generic Functions			*/
#include "H5Iprivate.h"
#include "H5Dpkg.h"		/* Datasets				*/
#include "H5Eprivate.h"		/* Error handling		  	*/
#include "H5Fprivate.h"		/* File access				*/
#include "H5FDprivate.h"	/* File drivers				*/
#include "H5MMprivate.h"
#include "H5Oprivate.h"		/* Object headers		  	*/
#include "H5Pprivate.h"         /* Property lists                       */
#include "H5Sprivate.h"		/* Dataspaces 				*/
#include "H5Vprivate.h"         /* Vector                               */

#ifdef H5_HAVE_PARALLEL

/****************/
/* Local Macros */
/****************/

/* Macros to represent different IO options */
#define H5D_ONE_LINK_CHUNK_IO 0
#define H5D_MULTI_CHUNK_IO 1

/***** Macros for One linked collective IO case. *****/
/* The default value to do one linked collective IO for all chunks.
   If the average number of chunks per process is greater than this value,
      the library will create an MPI derived datatype to link all chunks to do collective IO.
      The user can set this value through an API. */
#define H5D_ONE_LINK_CHUNK_IO_THRESHOLD 0 /* always set this option with value 0*/

/* Macros to represent options on how to obtain chunk address for one linked-chunk IO case */
#define H5D_OBTAIN_ONE_CHUNK_ADDR_IND 0
#define H5D_OBTAIN_ALL_CHUNK_ADDR_IND 1
#define H5D_OBTAIN_ALL_CHUNK_ADDR_COL 2

/* Macros to define the default ratio of obtaining all chunk addresses for one linked-chunk IO case */
#define H5D_ALL_CHUNK_ADDR_THRES_IND  10
#define H5D_ALL_CHUNK_ADDR_THRES_COL  20

/***** Macros for multi-chunk collective IO case. *****/
/* The default value of the threshold to do collective IO for this chunk.
   If the average number of processes per chunk is greater than the default value,
   collective IO is done for this chunk.
*/
#define H5D_MULTI_CHUNK_IO_COL_THRESHOLD 50

/* Macros to represent different IO modes(NONE, Independent or collective)for multiple chunk IO case */
#define H5D_CHUNK_IO_MODE_IND         0
#define H5D_CHUNK_IO_MODE_COL         1

/* Macros to represent the regularity of the selection for multiple chunk IO case. */
#define H5D_CHUNK_SELECT_REG          1
#define H5D_CHUNK_SELECT_IRREG        2
#define H5D_CHUNK_SELECT_NONE         0

/******************/
/* Local Typedefs */
/******************/
/* Combine chunk address and chunk info into a struct for better performance. */
typedef struct H5D_chunk_addr_info_t {
  haddr_t chunk_addr;
  H5D_chunk_info_t chunk_info;
} H5D_chunk_addr_info_t;

/* Combine all information that needs to know for collective MPI-IO of this selection. */
typedef struct H5D_common_coll_info_t {
  hbool_t mbt_is_derived;
  hbool_t mft_is_derived;
  size_t  mpi_buf_count;
  haddr_t chunk_addr;
} H5D_common_coll_info_t;
  

/********************/
/* Local Prototypes */
/********************/

static herr_t 
H5D_multi_chunk_collective_io(H5D_io_info_t *io_info,fm_map *fm,void *buf, 
			      hbool_t do_write);

static herr_t
H5D_link_chunk_collective_io(H5D_io_info_t *io_info,fm_map *fm,void *buf, 
			     hbool_t do_write,int sum_chunk);

static herr_t 
H5D_inter_collective_io(H5D_io_info_t *io_info,const H5S_t *file_space,
			const H5S_t *mem_space,haddr_t addr, 
		        void *buf, hbool_t do_write );

static herr_t 
H5D_final_collective_io(H5D_io_info_t *io_info,MPI_Datatype*mpi_file_type,
			 MPI_Datatype *mpi_buf_type,
			 H5D_common_coll_info_t* coll_info, 
			 void *buf, hbool_t do_write);
#ifdef OLD_WAY
static herr_t
H5D_pre_sort_chunk(H5D_io_info_t *io_info,int total_chunks,
		   haddr_t total_chunk_addr_array[]);
#endif

static herr_t 
H5D_sort_chunk(H5D_io_info_t * io_info,
	       fm_map *fm,
	       H5D_chunk_addr_info_t chunk_addr_info_array[],
	       int many_chunk_opt);

static herr_t 
H5D_obtain_mpio_mode(H5D_io_info_t* io_info, 
		     fm_map *fm,
		     uint8_t assign_io_mode[],
		     haddr_t chunk_addr[]);

static herr_t H5D_ioinfo_make_ind(H5D_io_info_t *io_info);
static herr_t H5D_ioinfo_make_coll(H5D_io_info_t *io_info);
static herr_t H5D_mpio_get_min_chunk(const H5D_io_info_t *io_info,
    const fm_map *fm, int *min_chunkf);
static int H5D_cmp_chunk_addr(const void *addr1, const void *addr2);
static herr_t 
H5D_mpio_get_sum_chunk(const H5D_io_info_t *io_info,
		       const fm_map *fm, int *sum_chunkf);


/*********************/
/* Package Variables */
/*********************/

/*******************/
/* Local Variables */
/*******************/


/*-------------------------------------------------------------------------
 * Function:	H5D_mpio_opt_possible
 *
 * Purpose:	Checks if an direct I/O transfer is possible between memory and
 *                  the file.
 *
 * Return:	Success:        Non-negative: TRUE or FALSE
 *		Failure:	Negative
 *
 * Programmer:	Quincey Koziol
 *              Wednesday, April 3, 2002
 *
 *-------------------------------------------------------------------------
 */
htri_t
H5D_mpio_opt_possible( const H5D_io_info_t *io_info,
    const H5S_t *mem_space, const H5S_t *file_space, const H5T_path_t *tpath)
{
    int         local_opinion = TRUE;   /* This process's idea of whether to perform collective I/O or not */
    int         consensus;              /* Consensus opinion of all processes */
    int         mpi_code;               /* MPI error code */
    htri_t ret_value=TRUE;

    FUNC_ENTER_NOAPI(H5D_mpio_opt_possible, FAIL);

    /* Check args */
    assert(io_info);
    assert(mem_space);
    assert(file_space);

    /* For independent I/O, get out quickly and don't try to form consensus */
    if (io_info->dxpl_cache->xfer_mode==H5FD_MPIO_INDEPENDENT)
        HGOTO_DONE(FALSE);

    /* Optimized MPI types flag must be set and it must be collective IO */
    /* (Don't allow parallel I/O for the MPI-posix driver, since it doesn't do real collective I/O) */
    if (!(H5S_mpi_opt_types_g && io_info->dxpl_cache->xfer_mode==H5FD_MPIO_COLLECTIVE && !IS_H5FD_MPIPOSIX(io_info->dset->oloc.file))) {
        local_opinion = FALSE;
        goto broadcast;
    } /* end if */

    /* Check whether these are both simple or scalar dataspaces */
    if (!((H5S_SIMPLE==H5S_GET_EXTENT_TYPE(mem_space) || H5S_SCALAR==H5S_GET_EXTENT_TYPE(mem_space))
            && (H5S_SIMPLE==H5S_GET_EXTENT_TYPE(file_space) || H5S_SCALAR==H5S_GET_EXTENT_TYPE(file_space)))) {
        local_opinion = FALSE;
        goto broadcast;
    } /* end if */

    /* Can't currently handle point selections */
    if (H5S_SEL_POINTS==H5S_GET_SELECT_TYPE(mem_space) || H5S_SEL_POINTS==H5S_GET_SELECT_TYPE(file_space)) {
        local_opinion = FALSE;
        goto broadcast;
    } /* end if */

    /* Dataset storage must be contiguous or chunked */
    if (!(io_info->dset->shared->layout.type == H5D_CONTIGUOUS ||
            io_info->dset->shared->layout.type == H5D_CHUNKED)) {
        local_opinion = FALSE;
        goto broadcast;
    } /* end if */

    /*    The handling of memory space is different for chunking
	  and contiguous storage,
	  For contigous storage, mem_space and file_space won't
	  change when it it is doing disk IO.
	  For chunking storage, mem_space will change for different
	  chunks. So for chunking storage, whether we can use
	  collective IO will defer until each chunk IO is reached.
	  For contiguous storage, if we find MPI-IO cannot
	  support complicated MPI derived data type, we will
	  set use_par_opt_io = FALSE.
	*/
#ifndef H5_MPI_COMPLEX_DERIVED_DATATYPE_WORKS
    if(io_info->dset->shared->layout.type == H5D_CONTIGUOUS)
        if((H5S_SELECT_IS_REGULAR(file_space) != TRUE) ||
                (H5S_SELECT_IS_REGULAR(mem_space) != TRUE)) {
            local_opinion = FALSE;
            goto broadcast;
        } /* end if */
#endif

    /* Don't allow collective operations if filters need to be applied */
    if(io_info->dset->shared->layout.type == H5D_CHUNKED)
        if(io_info->dset->shared->dcpl_cache.pline.nused>0) {
            local_opinion = FALSE;
            goto broadcast;
        } /* end if */

    /* Don't allow collective operations if datatype conversions need to happen */
    if(!H5T_path_noop(tpath)) {
        local_opinion = FALSE;
        goto broadcast;
    } /* end if */

    /* Don't allow collective operations if data transform operations should occur */
    if(!H5Z_xform_noop(io_info->dxpl_cache->data_xform_prop)) {
        local_opinion = FALSE;
        goto broadcast;
    } /* end if */

broadcast:
    /* Form consensus opinion among all processes about whether to perform
     * collective I/O */
    if (MPI_SUCCESS != (mpi_code = MPI_Allreduce(&local_opinion, &consensus, 1, MPI_INT, MPI_LAND, io_info->comm)))
        HMPI_GOTO_ERROR(FAIL, "MPI_Allreduce failed", mpi_code)

    ret_value = consensus > 0 ? TRUE : FALSE;

done:
    FUNC_LEAVE_NOAPI(ret_value);
} /* H5D_mpio_opt_possible() */


/*-------------------------------------------------------------------------
 * Function:	H5D_mpio_chunk_adjust_iomode
 *
 * Purpose:	Checks if it is possible to do collective IO 
 *
 * Return:	Success:        Non-negative: TRUE or FALSE
 *		Failure:	Negative
 *
 * Programmer:	Muqun Yang
 *              Monday, Feb. 13th, 2006
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5D_mpio_chunk_adjust_iomode(H5D_io_info_t *io_info, const fm_map *fm) {

   int   min_chunk;
   herr_t ret_value = SUCCEED;

   FUNC_ENTER_NOAPI_NOINIT(H5D_mpio_chunk_adjust_iomode)

#ifndef H5_MPI_SPECIAL_COLLECTIVE_IO_WORKS
    if(H5D_mpio_get_min_chunk(io_info,fm,&min_chunk)<0)
         HGOTO_ERROR(H5E_DATASPACE, H5E_CANTSWAP, FAIL, "unable to obtain the min chunk number of all processes"); 
    if(min_chunk == 0) {                
       H5P_genplist_t *dx_plist;           /* Data transer property list */

       /* Get the dataset transfer property list */
       if (NULL == (dx_plist = H5I_object(io_info->dxpl_id)))
         HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a dataset creation property list")

       /* Change the xfer_mode to independent for handling the I/O */
       io_info->dxpl_cache->xfer_mode = H5FD_MPIO_INDEPENDENT;
       if(H5P_set (dx_plist, H5D_XFER_IO_XFER_MODE_NAME, &io_info->dxpl_cache->xfer_mode) < 0)
                    HGOTO_ERROR(H5E_PLIST, H5E_CANTSET, FAIL, "can't set transfer mode")

       /* Indicate that the transfer mode should be restored before returning
                 * to user.
                 */
       io_info->xfer_mode_changed = TRUE;
       io_info->ops.read = H5D_select_read;
       io_info->ops.write = H5D_select_write;
     } /* end if */
#endif
done:
    FUNC_LEAVE_NOAPI(ret_value)
}



/*-------------------------------------------------------------------------
 * Function:	H5D_mpio_select_read
 *
 * Purpose:	MPI-IO function to read directly from app buffer to file.
 *
 * Return:	non-negative on success, negative on failure.
 *
 * Programmer:
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5D_mpio_select_read(H5D_io_info_t *io_info,
                     size_t mpi_buf_count, 
		     size_t elmt_size,
		     const H5S_t UNUSED *file_space, 
		     const H5S_t UNUSED *mem_space,
		     haddr_t addr,		     
		     void *buf/*out*/)
{
    herr_t ret_value = SUCCEED;

    FUNC_ENTER_NOAPI(H5D_mpio_select_read,FAIL);

    if(H5F_block_read (io_info->dset->oloc.file, H5FD_MEM_DRAW, addr, mpi_buf_count, io_info->dxpl_id, buf) < 0)
	HGOTO_ERROR(H5E_IO,H5E_READERROR,FAIL,"can't finish collective parallel read");
done:
    FUNC_LEAVE_NOAPI(ret_value);
} /* end H5D_mpio_select_read() */


/*-------------------------------------------------------------------------
 * Function:	H5D_mpio_select_write
 *
 * Purpose:	MPI-IO function to write directly from app buffer to file.
 *
 * Return:	non-negative on success, negative on failure.
 *
 * Programmer:
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5D_mpio_select_write(H5D_io_info_t *io_info,
		      size_t mpi_buf_count, 
		      size_t elmt_size,
		      const H5S_t UNUSED *file_space, 
		      const H5S_t UNUSED *mem_space,
		      haddr_t addr,
		      const void *buf)
{
    herr_t ret_value;

    FUNC_ENTER_NOAPI(H5D_mpio_select_write,FAIL);

    /*OKAY: CAST DISCARDS CONST QUALIFIER*/
    if(H5F_block_write (io_info->dset->oloc.file, H5FD_MEM_DRAW, addr, mpi_buf_count, io_info->dxpl_id, buf)<0)
       HGOTO_ERROR(H5E_IO,H5E_WRITEERROR,FAIL,"can't finish collective parallel write");
done:
    FUNC_LEAVE_NOAPI(ret_value);
} /* end H5D_mpio_select_write() */


/*-------------------------------------------------------------------------
 * Function:	H5D_ioinfo_make_ind
 *
 * Purpose:	Switch to MPI independent I/O
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		Friday, August 12, 2005
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5D_ioinfo_make_ind(H5D_io_info_t *io_info)
{
    H5P_genplist_t *dx_plist;           /* Data transer property list */
    herr_t	ret_value = SUCCEED;	/*return value		*/

    FUNC_ENTER_NOAPI_NOINIT(H5D_ioinfo_make_ind)

    /* Get the dataset transfer property list */
    if (NULL == (dx_plist = H5I_object(io_info->dxpl_id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a dataset transfer property list")

    /* Change the xfer_mode to independent, handle the request,
     * then set xfer_mode before return.
     */
    io_info->dxpl_cache->xfer_mode = H5FD_MPIO_INDEPENDENT;
    if(H5P_set (dx_plist, H5D_XFER_IO_XFER_MODE_NAME, &io_info->dxpl_cache->xfer_mode) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTSET, FAIL, "can't set transfer mode")

    /* Set the pointers to the non-MPI-specific routines */
    io_info->ops.read = H5D_select_read;
    io_info->ops.write = H5D_select_write;

    /* Indicate that the transfer mode should be restored before returning
     * to user.
     */
    io_info->xfer_mode_changed=TRUE;

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5D_ioinfo_make_ind() */


/*-------------------------------------------------------------------------
 * Function:	H5D_ioinfo_make_coll
 *
 * Purpose:	Switch to MPI collective I/O
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		Friday, August 12, 2005
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5D_ioinfo_make_coll(H5D_io_info_t *io_info)
{
    H5P_genplist_t *dx_plist;           /* Data transer property list */
    herr_t	ret_value = SUCCEED;	/*return value		*/

    FUNC_ENTER_NOAPI_NOINIT(H5D_ioinfo_make_coll)

    /* Get the dataset transfer property list */
    if (NULL == (dx_plist = H5I_object(io_info->dxpl_id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a dataset transfer property list")

    /* Change the xfer_mode to independent, handle the request,
     * then set xfer_mode before return.
     */
    io_info->dxpl_cache->xfer_mode = H5FD_MPIO_COLLECTIVE;
    if(H5P_set (dx_plist, H5D_XFER_IO_XFER_MODE_NAME, &io_info->dxpl_cache->xfer_mode) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTSET, FAIL, "can't set transfer mode")

    /* Set the pointers to the MPI-specific routines */
    io_info->ops.read = H5D_mpio_select_read;
    io_info->ops.write = H5D_mpio_select_write;

    /* Indicate that the transfer mode should _NOT_ be restored before returning
     * to user.
     */
    io_info->xfer_mode_changed=FALSE;

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5D_ioinfo_make_coll() */




/*-------------------------------------------------------------------------
 * Function:	H5D_mpio_get_min_chunk
 *
 * Purpose:	Routine for obtaining minimum number of chunks to cover
 *              hyperslab selection selected by all processors.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5D_mpio_get_min_chunk(const H5D_io_info_t *io_info,
    const fm_map *fm, int *min_chunkf)
{
    int num_chunkf;             /* Number of chunks to iterate over */
    int mpi_code;               /* MPI return code */
    herr_t ret_value = SUCCEED;

    FUNC_ENTER_NOAPI_NOINIT(H5D_mpio_get_min_chunk);

    /* Get the number of chunks to perform I/O on */
    num_chunkf = H5SL_count(fm->fsel);

    /* Determine the minimum # of chunks for all processes */
    if (MPI_SUCCESS != (mpi_code = MPI_Allreduce(&num_chunkf, min_chunkf, 1, MPI_INT, MPI_MIN, io_info->comm)))
        HMPI_GOTO_ERROR(FAIL, "MPI_Allreduce failed", mpi_code)

done:
    FUNC_LEAVE_NOAPI(ret_value);
} /* end H5D_mpio_get_min_chunk() */
/*-------------------------------------------------------------------------
 * Function:	H5D_mpio_get_sum_chunk
 *
 * Purpose:	Routine for obtaining total number of chunks to cover
 *              hyperslab selection selected by all processors.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5D_mpio_get_sum_chunk(const H5D_io_info_t *io_info,
    const fm_map *fm, int *sum_chunkf)
{
    int num_chunkf;             /* Number of chunks to iterate over */
    size_t ori_num_chunkf;
    int mpi_code;               /* MPI return code */
    herr_t ret_value = SUCCEED;

    FUNC_ENTER_NOAPI_NOINIT(H5D_mpio_get_sum_chunk);

    /* Get the number of chunks to perform I/O on */
    num_chunkf = 0;
    ori_num_chunkf = H5SL_count(fm->fsel);
    H5_ASSIGN_OVERFLOW(num_chunkf,ori_num_chunkf,size_t,int);

    /* Determine the minimum # of chunks for all processes */
    if (MPI_SUCCESS != (mpi_code = MPI_Allreduce(&num_chunkf, sum_chunkf, 1, MPI_INT, MPI_SUM, io_info->comm)))
        HMPI_GOTO_ERROR(FAIL, "MPI_Allreduce failed", mpi_code)

done:
    FUNC_LEAVE_NOAPI(ret_value);
} /* end H5D_mpio_get_sum_chunk() */


/*-------------------------------------------------------------------------
 * Function:	H5D_contig_collective_io
 *
 * Purpose:	Routine for 
 *              1) building up MPI derived datatype
 *              2) setting up collective IO property list
 *              3) Do IO 
 *               
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5D_contig_collective_io(H5D_io_info_t *io_info, 
			 const H5S_t *file_space,
			 const H5S_t *mem_space,
			 void *buf,
			 hbool_t do_write) 
{


    haddr_t	 addr = HADDR_UNDEF;                  /* Address of dataset (or selection) within file */
    hbool_t      select_valid;
    herr_t       ret_value = SUCCEED;  /* return value */

    FUNC_ENTER_NOAPI_NOINIT(H5D_contig_collective_io)
    assert (IS_H5FD_MPIO(io_info->dset->oloc.file));

    /* Make certain we have the correct type of property list */
    assert(TRUE==H5P_isa_class(io_info->dxpl_id,H5P_DATASET_XFER));

    /* Get the base address of the contiguous dataset */
    if(io_info->dset->shared->layout.type == H5D_CONTIGUOUS)
       addr = H5D_contig_get_addr(io_info->dset);

    if(H5D_inter_collective_io(io_info,file_space,mem_space,addr,buf,do_write)<0)
	HGOTO_ERROR(H5E_IO, H5E_CANTGET, FAIL,"couldn't finish shared collective MPI-IO");
      
 done: 

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5D_contig_collective_io */

/*-------------------------------------------------------------------------
 * Function:	H5D_chunk_collective_io
 *
 * Purpose:	Routine for 
                1) choose an IO option: 
		      a) One collective IO defined by one MPI derived datatype to link through all chunks
		or    b) multiple chunk IOs,to do MPI-IO for each chunk, the IO mode may be adjusted 
                         due to the selection pattern for each chunk.
 *              For option a)
			1. Sort the chunk address, obtain chunk info according to the sorted chunk address
                        2. Build up MPI derived datatype for each chunk
                        3. Build up the final MPI derived datatype
                        4. Set up collective IO property list
                        5. Do IO
 *             For option b)
                        1. Use MPI_gather and MPI_Bcast to obtain information of *collective/independent/none*
                           IO mode for each chunk of the data space
                        2. Depending on whether the IO mode is collective or independent or none,
                           Create either MPI derived datatype for each chunk to do collective IO or just do independent IO
                        3. Set up collective IO property list for collective mode
                        4. DO IO               
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
herr_t 
H5D_chunk_collective_io(H5D_io_info_t *io_info,fm_map *fm,void *buf, hbool_t do_write) 
{

    int               io_option = H5D_MULTI_CHUNK_IO;
    int               min_chunk,sum_chunk,mpi_size;
    int               one_link_chunk_io_threshold;
    herr_t            ret_value = SUCCEED;    


    FUNC_ENTER_NOAPI_NOINIT(H5D_chunk_collective_io)

    assert (IS_H5FD_MPIO(io_info->dset->oloc.file));

    if(H5D_mpio_get_sum_chunk(io_info,fm,&sum_chunk)<0)   
         HGOTO_ERROR(H5E_DATASPACE, H5E_CANTSWAP, FAIL, "unable to obtain the total chunk number of all processes"); 
    if((mpi_size = H5F_mpi_get_size(io_info->dset->oloc.file))<0)
	 HGOTO_ERROR (H5E_IO, H5E_MPI, FAIL, "unable to obtain mpi size");
    one_link_chunk_io_threshold = H5D_ONE_LINK_CHUNK_IO_THRESHOLD;/*This should be replaced by the user inputting value from API. */

    /* step 1: choose an IO option */
    /* If the average number of chunk per process is greater than a threshold, we will do one link chunked IO. */
    if(sum_chunk/mpi_size >= one_link_chunk_io_threshold) io_option = H5D_ONE_LINK_CHUNK_IO;

/* If this MPI-IO package doesn't support collective IO when no IO is done for one or more processes,
   use MULTIPLE CHUNK IO */
/*
#ifndef H5_MPI_SPECIAL_COLLECTIVE_IO_WORKS
    if(H5D_mpio_get_min_chunk(io_info,fm,&min_chunk)<0)
         HGOTO_ERROR(H5E_DATASPACE, H5E_CANTSWAP, FAIL, "unable to obtain the min chunk number of all processes"); 
    if(min_chunk == 0) io_option = H5D_MULTI_CHUNK_IO;
#endif
*/

#ifndef H5_MPI_COMPLEX_DERIVED_DATATYPE_WORKS
    if(io_option == H5D_ONE_LINK_CHUNK_IO ) io_option = H5D_MULTI_CHUNK_IO ;/* We can not do this with one chunk IO. */
#endif

    /* step 2:  Go ahead to do IO.*/
    if(io_option == H5D_ONE_LINK_CHUNK_IO) {
      if(H5D_link_chunk_collective_io(io_info,fm,buf,do_write,sum_chunk)<0)
	HGOTO_ERROR(H5E_IO, H5E_CANTGET, FAIL,"couldn't finish linked chunk MPI-IO");
    }
    else {/* multiple chunk IOs */
      printf("coming into multiple chunk \n");
      if(H5D_multi_chunk_collective_io(io_info,fm,buf,do_write)<0)
	HGOTO_ERROR(H5E_IO, H5E_CANTGET, FAIL,"couldn't finish multiple chunk MPI-IO");
    }

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5D_chunk_collective_io */

/*-------------------------------------------------------------------------
 * Function:	H5D_link_chunk_collective_io
 *
 * Purpose:	Routine for one collective IO with one MPI derived datatype to link with all chunks

			1. Sort the chunk address and chunk info
                        2. Build up MPI derived datatype for each chunk
                        3. Build up the final MPI derived datatype
			4. Use common collective IO routine to do MPI-IO 

 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */

static herr_t
H5D_link_chunk_collective_io(H5D_io_info_t *io_info,fm_map *fm,void *buf, hbool_t do_write,int sum_chunk)
{


      size_t	       src_type_size;		/*size of source type	*/
      size_t	       dst_type_size;	        /*size of destination type*/
      hsize_t          mpi_buf_extra_offset;
      hsize_t          mpi_file_extra_offset;
      size_t           mpi_buf_count;
      size_t           mpi_file_count;
      hbool_t	       mbt_is_derived=0,      /* Whether the buffer (memory) type is derived and needs to be free'd */
		       mft_is_derived=0;      /* Whether the file type is derived and needs to be free'd */  
     
      int              mpi_size,mpi_code;              /* MPI return code */ 

      int               i,num_chunk,total_chunks;
      size_t            ori_num_chunk;
      hsize_t           ori_total_chunks;
      haddr_t           chunk_base_addr;
      haddr_t*          total_chunk_addr_array;
      MPI_Datatype     *chunk_mtype;
      MPI_Datatype     *chunk_ftype;
      MPI_Datatype      chunk_final_mtype;
      MPI_Datatype      chunk_final_ftype;
      MPI_Aint         *chunk_disp_array;
      MPI_Aint         *chunk_mem_disp_array;
      int              *blocklen;
      int               blocklen_value;
      int               actual_bsearch_coll_chunk_threshold;
      int               bsearch_coll_chunk_threshold;
      int               bsearch_chunk_ratio;
      int               bsearch_chunk_threshold;
      int               many_chunk_opt = 0;

      H5D_common_coll_info_t coll_info;
      H5D_chunk_addr_info_t*  chunk_addr_info_array;

#ifdef CC_PERF
      char *bc_percent = NULL;
      char *bcc_percent = NULL;
#endif
      herr_t            ret_value = SUCCEED;    
      
      FUNC_ENTER_NOAPI_NOINIT(H5D_link_chunk_collective_io)
      ori_total_chunks = fm->total_chunks;
      H5_ASSIGN_OVERFLOW(total_chunks,ori_total_chunks,hsize_t,int);

      /* Handle with a special case when only one chunk is covered by all processes */
      if(total_chunks == 1){
        H5SL_node_t *chunk_node;
        H5D_chunk_info_t *chunk_info;
        H5D_storage_t  store;
 
        chunk_node = H5SL_first(fm->fsel);
	if(chunk_node == NULL) {
	  if(H5D_istore_chunkmap(io_info,total_chunks,&chunk_base_addr,fm->down_chunks)<0)
	     HGOTO_ERROR(H5E_DATASET, H5E_CANTGET, FAIL, "can't get chunk address");
	  if(H5D_inter_collective_io(io_info,NULL,NULL,chunk_base_addr,buf,do_write)<0)
	  HGOTO_ERROR(H5E_STORAGE, H5E_CANTGET, FAIL,"couldn't finish shared collective MPI-IO");
	}
	else {
	  if(NULL ==(chunk_info = H5SL_item(chunk_node)))
	    HGOTO_ERROR(H5E_STORAGE, H5E_CANTGET, FAIL,"couldn't get chunk info from skipped list");
	  io_info->store = &store;
	  store.chunk.offset = chunk_info->coords;
	  store.chunk.index  = chunk_info->index;

	  if(HADDR_UNDEF==(chunk_base_addr = H5D_istore_get_addr(io_info,NULL)))
	    HGOTO_ERROR(H5E_STORAGE, H5E_CANTGET, FAIL,"couldn't get chunk info from skipped list");
	
#ifdef KENT
printf("before inter_collective_io for total chunk = 1 \n");
#endif
	  if(H5D_inter_collective_io(io_info,chunk_info->fspace,chunk_info->mspace,chunk_base_addr,buf,do_write)<0)
	    HGOTO_ERROR(H5E_STORAGE, H5E_CANTGET, FAIL,"couldn't finish shared collective MPI-IO");
	}
	goto done;
      }

      /* Allocate chunking information */

      ori_num_chunk        = H5SL_count(fm->fsel);
      H5_ASSIGN_OVERFLOW(num_chunk,ori_num_chunk,size_t,int);

         
      if(num_chunk == 0) total_chunk_addr_array = H5MM_malloc(sizeof(haddr_t)*total_chunks);
      else
      {
	chunk_addr_info_array= H5MM_malloc(num_chunk*sizeof(H5D_chunk_addr_info_t));
	chunk_mtype          = H5MM_malloc(num_chunk*sizeof(MPI_Datatype));
	chunk_ftype          = H5MM_malloc(num_chunk*sizeof(MPI_Datatype));
	chunk_disp_array     = H5MM_malloc(num_chunk*sizeof(MPI_Aint));
	chunk_mem_disp_array = H5MM_calloc(num_chunk*sizeof(MPI_Aint));
	blocklen             = H5MM_malloc(num_chunk*sizeof(int));
      }

      /* Obtain information to do collective IO,
         in order to do collective IO, no datatype conversion should happen. */
      if((src_type_size = H5T_get_size(io_info->dset->shared->type))==0)
	HGOTO_ERROR(H5E_DATATYPE, H5E_BADSIZE, FAIL, "datatype size invalid");
      dst_type_size = src_type_size;


#ifdef CC_PERF
      /* "bcc" means 'b-tree iterately obtain all chunk addresses collectively',
	 "bc" means 'b-tree iterately obtain all chunk addresses individually',
          the default one means 'obtaining the chunk address individually',
      */
	 
      if(bcc_percent=getenv("BCC_PERCENT")){
         bsearch_coll_chunk_threshold  = atoi(bcc_percent);
         assert((bsearch_coll_chunk_threshold >=0) &&(bsearch_coll_chunk_threshold <=100));
      }
      else 
         bsearch_coll_chunk_threshold  = H5D_ALL_CHUNK_ADDR_THRES_COL;
#else
      bsearch_coll_chunk_threshold  = H5D_ALL_CHUNK_ADDR_THRES_COL; /*This number may be changed according to the performance study */
#endif

      if((mpi_size = H5F_mpi_get_size(io_info->dset->oloc.file))<0)
	 HGOTO_ERROR (H5E_IO, H5E_MPI, FAIL, "unable to obtain mpi size");

      /* Calculate the actual threshold to obtain all chunk addresses collectively 
         The bigger this number is, the more possible the use of obtaining chunk address collectively. */
      actual_bsearch_coll_chunk_threshold = sum_chunk*100/(total_chunks*mpi_size);

      if(actual_bsearch_coll_chunk_threshold >= bsearch_coll_chunk_threshold) 
	many_chunk_opt = H5D_OBTAIN_ALL_CHUNK_ADDR_COL;

      else {

#ifdef CC_PERF
	if(bc_percent=getenv("BC_PERCENT")){
	  bsearch_chunk_ratio  = atoi(bc_percent);
	  assert((bsearch_chunk_ratio<=100)&&(bsearch_chunk_ratio>=0));
	}
	else 
	  bsearch_chunk_ratio  = H5D_ALL_CHUNK_ADDR_THRES_IND;
#else
         bsearch_chunk_ratio = H5D_ALL_CHUNK_ADDR_THRES_IND; /*This number may be changed according to the performance study */
#endif

	 /* This threshold is to check whether we can use iterator to obtain all chunk addresses.
	    The unit of the threshold is the number of chunks. The value should be at least 1.
            It can be calculated as follows:

	    if(total_chunks*bsearch_chunk_ratio/100 <=1) 
	      bsearch_chunk_threahold = 1;
            else                                     
	      bsearch_chunk_threshold = total_chunks*bsearch_chunk_ratio/100;
	    In order to make the caluculation more efficient, 
	    we use the following approximate formula to calculate the threshold.

	    bsearch_chunk_threshold = 1+ (total_chunks*bsearch_chunk_ratio-99)/100;

	    The only difference is when total_chunks* besearch_chunk_ratio == 100n+99;
            the approximate formula will give value (n+1) instead of n for threshold. 
	    That shouldn't matter much from our persective.
	 */          
            
        bsearch_chunk_threshold = 1 +(total_chunks*bsearch_chunk_ratio-99)/100;
	if(num_chunk > bsearch_chunk_threshold) many_chunk_opt = H5D_OBTAIN_ALL_CHUNK_ADDR_IND;
      }

      /* Sort the chunk address 
         when chunk optimization selection is either H5D_OBTAIN_*/
      if(num_chunk == 0){
         if(H5D_istore_chunkmap(io_info,total_chunks,total_chunk_addr_array,fm->down_chunks)<0)
             HGOTO_ERROR(H5E_DATASET, H5E_CANTGET, FAIL, "can't get chunk address");
         chunk_base_addr = total_chunk_addr_array[0];
      }
 
      else {
         if(H5D_sort_chunk(io_info,fm,chunk_addr_info_array,many_chunk_opt)<0)
        	 HGOTO_ERROR (H5E_DATASPACE, H5E_CANTSWAP, FAIL, "unable to sort chunk address");
         chunk_base_addr = chunk_addr_info_array[0].chunk_addr;
      }
      
      /* Obtain MPI derived datatype from all individual chunks */ 
      for ( i = 0; i < num_chunk; i++) {
	  /* Disk MPI derived datatype */
          if(H5S_mpio_space_type(chunk_addr_info_array[i].chunk_info.fspace,src_type_size,&chunk_ftype[i],
				    &mpi_file_count,&mpi_file_extra_offset,&mft_is_derived)<0)
	       	HGOTO_ERROR(H5E_DATASPACE, H5E_BADTYPE, FAIL,"couldn't create MPI file type");

	  /* Buffer MPI derived datatype */
          if(H5S_mpio_space_type(chunk_addr_info_array[i].chunk_info.mspace,dst_type_size,&chunk_mtype[i],
                                       &mpi_buf_count,&mpi_buf_extra_offset,&mbt_is_derived)<0)
	       	HGOTO_ERROR(H5E_DATASPACE, H5E_BADTYPE, FAIL,"couldn't create MPI buf type");
           
          /* Chunk address relative to the first chunk */
	  chunk_addr_info_array[i].chunk_addr -= chunk_base_addr;
          H5_ASSIGN_OVERFLOW(chunk_disp_array[i],chunk_addr_info_array[i].chunk_addr,haddr_t,MPI_Aint);
          /*chunk_disp_array[i]  = (MPI_Aint)chunk_addr_array[i];*/
      }

      blocklen_value = 1;
      if(num_chunk){
	
	/* initialize the buffer with the constant value 1 */
	H5V_array_fill(blocklen,&blocklen_value,sizeof(int),(size_t)num_chunk);

      /* Create final MPI derived datatype */
	if(MPI_SUCCESS != (mpi_code = MPI_Type_struct(num_chunk,blocklen,chunk_disp_array,chunk_ftype,&chunk_final_ftype)))
	   HMPI_GOTO_ERROR(FAIL, "MPI_Type_struct failed", mpi_code);
	if(MPI_SUCCESS != (mpi_code = MPI_Type_commit(&chunk_final_ftype)))
	   HMPI_GOTO_ERROR(FAIL, "MPI_Type_commit failed", mpi_code);

	if(MPI_SUCCESS != (mpi_code = MPI_Type_struct(num_chunk,blocklen,chunk_mem_disp_array,chunk_mtype,&chunk_final_mtype)))
	   HMPI_GOTO_ERROR(FAIL, "MPI_Type_struct failed", mpi_code);
	if(MPI_SUCCESS != (mpi_code = MPI_Type_commit(&chunk_final_mtype)))
	   HMPI_GOTO_ERROR(FAIL, "MPI_Type_commit failed", mpi_code);

	for ( i = 0; i< num_chunk;i++){
	  if (MPI_SUCCESS != (mpi_code= MPI_Type_free( chunk_mtype+i )))
            HMPI_DONE_ERROR(FAIL, "MPI_Type_free failed", mpi_code);
	  if (MPI_SUCCESS != (mpi_code= MPI_Type_free( chunk_ftype+i )))
            HMPI_DONE_ERROR(FAIL, "MPI_Type_free failed", mpi_code);
	}

	/* buffer, file derived datatypes should be true */
	coll_info.mbt_is_derived = 1;
	coll_info.mft_is_derived = 1;
	coll_info.mpi_buf_count  = 1;
	coll_info.chunk_addr     = chunk_base_addr;

      }

      else {/* no selection at all for this process */
	chunk_final_ftype = MPI_BYTE;
	chunk_final_mtype = MPI_BYTE;
	
	/* buffer, file derived datatypes should be true */
	coll_info.mbt_is_derived = 0;
	coll_info.mft_is_derived = 0;
	coll_info.mpi_buf_count  = 0;
	coll_info.chunk_addr     = chunk_base_addr;
      }
      if(H5D_final_collective_io(io_info,&chunk_final_ftype,&chunk_final_mtype,&coll_info,buf,do_write)<0)
	HGOTO_ERROR(H5E_IO, H5E_CANTGET, FAIL,"couldn't finish MPI-IO");

 done:
      if(num_chunk == 0) HDfree(total_chunk_addr_array);
      else {
        if (fm->total_chunks != 1) {
          HDfree(chunk_addr_info_array);
	  HDfree(chunk_mtype);
	  HDfree(chunk_ftype);
	  HDfree(chunk_disp_array);
	  HDfree(chunk_mem_disp_array);
	  HDfree(blocklen);
        }
      }

      FUNC_LEAVE_NOAPI(ret_value)
}
/* end H5D_link_chunk_collective_io */


/*-------------------------------------------------------------------------
 * Function:	H5D_multi_chunk_collective_io
 *
 * Purpose:	To do IO per chunk according to IO mode(collective/independent/none)

                1. Use MPI_gather and MPI_Bcast to obtain IO mode in each chunk(collective/independent/none)
                2. Depending on whether the IO mode is collective or independent or none,
                   Create either MPI derived datatype for each chunk or just do independent IO
                3. Use common collective IO routine to do MPI-IO               
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t 
H5D_multi_chunk_collective_io(H5D_io_info_t *io_info,fm_map *fm,void *buf, hbool_t do_write) 
{

      int               i,total_chunk;
      int               mpi_rank;
      hsize_t           ori_total_chunk;
      uint8_t          *chunk_io_option;

      H5SL_node_t      *chunk_node;           /* Current node in chunk skip list */
      H5D_chunk_info_t *chunk_info;
      haddr_t          *chunk_addr;
      H5D_storage_t     store;                /* union of EFL and chunk pointer in file space */
      hbool_t           select_chunk;
      hbool_t 	        last_io_mode_coll = TRUE;
      herr_t            ret_value = SUCCEED;    


      FUNC_ENTER_NOAPI_NOINIT(H5D_multi_chunk_collective_io)
#ifdef KENT
      mpi_rank = H5F_mpi_get_rank(io_info->dset->oloc.file);
#endif
      /* Allocate memories */
      ori_total_chunk     = fm->total_chunks;
      H5_ASSIGN_OVERFLOW(total_chunk,ori_total_chunk,hsize_t,int);
      HDassert(total_chunk!=0);
      chunk_io_option = (uint8_t *)H5MM_calloc(total_chunk*sizeof(MPI_BYTE));
      chunk_addr      = (haddr_t *)H5MM_calloc(total_chunk*sizeof(haddr_t));
#ifdef KENT
   printf("total_chunk %d\n",total_chunk);
#endif

      /* obtain IO option for each chunk */
      if(H5D_obtain_mpio_mode(io_info,fm,chunk_io_option,chunk_addr)<0) 
	HGOTO_ERROR (H5E_DATASET, H5E_CANTRECV, FAIL, "unable to obtain MPIO mode");

      for( i = 0; i<total_chunk;i++){

#ifdef KENT
printf("mpi_rank = %d, chunk index = %d\n",mpi_rank,i);
#endif
	select_chunk = fm->select_chunk[i];
	if(select_chunk == 1){/* Have selection elements in this chunk. Find the chunk info. */
#ifdef NEW_WAY
	  if(NULL ==(chunk_info = H5SL_item(chunk_node)))
	  HGOTO_ERROR(H5E_STORAGE, H5E_CANTGET, FAIL,"couldn't get chunk info from skipped list");
	  HDassert(chunk_info->index == i);

	  /* Set dataset storage for I/O info */
	  io_info->store=&store;
	  /* Pass in chunk's coordinates in a union. */
	  store.chunk.offset  = chunk_info->coords;
	  store.chunk.index   = chunk_info->index;
          if(NULL ==(chunk_node = H5SL_first(fm->fsel)))
	    HGOTO_ERROR(H5E_STORAGE, H5E_CANTGET, FAIL,"couldn't get chunk node from skipped list");
#else
	  
	   if(NULL ==(chunk_node = H5SL_first(fm->fsel)))
	    HGOTO_ERROR(H5E_STORAGE, H5E_CANTGET, FAIL,"couldn't get chunk node from skipped list");
	    while(chunk_node){

	      if(NULL ==(chunk_info = H5SL_item(chunk_node)))
		HGOTO_ERROR(H5E_STORAGE, H5E_CANTGET, FAIL,"couldn't get chunk info from skipped list");
	      if(chunk_info->index == i) {
		/* Set dataset storage for I/O info */
		io_info->store=&store;
		/* Pass in chunk's coordinates in a union. */
		store.chunk.offset  = chunk_info->coords;
		store.chunk.index   = chunk_info->index;
		break;
	      }

	      chunk_node = H5SL_next(chunk_node);
            }
#endif
	}

        if(chunk_io_option[i] == 1){ /*collective IO for this chunk, 
				       note: even there is no selection for this process,
                                             the process still needs to contribute MPI NONE TYPE.*/
#ifdef KENT
printf("inside collective chunk IO mpi_rank = %d, chunk index = %d\n",mpi_rank,i);
#endif
	
	  if(!last_io_mode_coll)
	  /* Switch back to collective I/O */
              if(H5D_ioinfo_make_coll(io_info) < 0)
                 HGOTO_ERROR(H5E_DATASET, H5E_CANTGET, FAIL, "can't switch to collective I/O")
          
	    if(select_chunk){
	      if(H5D_inter_collective_io(io_info,chunk_info->fspace,chunk_info->mspace,
			             chunk_addr[i],buf,do_write )<0)
	        HGOTO_ERROR(H5E_IO, H5E_CANTGET, FAIL,"couldn't finish shared collective MPI-IO");
	       
	    }
	    else{
	     if(H5D_inter_collective_io(io_info,NULL,NULL,
			             chunk_addr[i],buf,do_write )<0)
	        HGOTO_ERROR(H5E_IO, H5E_CANTGET, FAIL,"couldn't finish shared collective MPI-IO");
	       
	    } 
	      

              last_io_mode_coll = TRUE;

	}
	else {/*possible independent IO for this chunk*/
#ifdef KENT
printf("inside independent IO mpi_rank = %d, chunk index = %d\n",mpi_rank,i);
#endif
	
	  HDassert(chunk_io_option[i] == 0);
	  if(!select_chunk) continue; /* this process has nothing to do with this chunk, continue! */
	  if(last_io_mode_coll)
	  /* Switch to independent I/O */
            if(H5D_ioinfo_make_ind(io_info) < 0)
               HGOTO_ERROR(H5E_DATASET, H5E_CANTGET, FAIL, "can't switch to independent I/O")

	  if(do_write) {
 	    ret_value = (io_info->ops.write)(io_info,
			 chunk_info->chunk_points,H5T_get_size(io_info->dset->shared->type),
			 chunk_info->fspace,chunk_info->mspace,0,
			 buf);
	      /* Check return value of the write */
	    if (ret_value<0) 
	      HGOTO_ERROR(H5E_DATASET, H5E_WRITEERROR, FAIL, "optimized write failed")
	  }
	  else {
	     ret_value = (io_info->ops.read)(io_info,
			  chunk_info->chunk_points,H5T_get_size(io_info->dset->shared->type),
			  chunk_info->fspace,chunk_info->mspace,0,
	        	  buf);			   
	      /* Check return value from optimized write */
	      if (ret_value<0) 
		HGOTO_ERROR(H5E_DATASET, H5E_READERROR, FAIL, "optimized read failed")
	  }

	last_io_mode_coll = FALSE;
	}
      }

	if(!last_io_mode_coll)
	  /* Switch back to collective I/O */
              if(H5D_ioinfo_make_coll(io_info) < 0)
                 HGOTO_ERROR(H5E_DATASET, H5E_CANTGET, FAIL, "can't switch to collective I/O")

  done:
    HDfree(chunk_io_option);
    HDfree(chunk_addr);

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5D_multi_chunk_collective_io */

/*-------------------------------------------------------------------------
 * Function:	H5D_inter_collective_io
 *
 * Purpose:	Routine for the shared part of collective IO between multiple chunk
                collective IO and contiguous collective IO
		
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t 
H5D_inter_collective_io(H5D_io_info_t *io_info,const H5S_t *file_space,const H5S_t *mem_space,
			 haddr_t addr, void *buf, hbool_t do_write ) 
{

      size_t	        mpi_buf_count, mpi_file_count;     /* Number of "objects" to transfer */
      MPI_Datatype      mpi_file_type,mpi_buf_type;
      hsize_t	        mpi_buf_offset, mpi_file_offset;   /* Offset within dataset where selection (ie. MPI type) begins */
      hbool_t	        mbt_is_derived=0,      /* Whether the buffer (memory) type is derived and needs to be free'd */
		        mft_is_derived=0;      /* Whether the file type is derived and needs to be free'd */  
      H5D_common_coll_info_t coll_info;
      herr_t       ret_value = SUCCEED;  /* return value */

      FUNC_ENTER_NOAPI_NOINIT(H5D_inter_collective_io)
      if((file_space!=NULL) &&  (mem_space != NULL)) {
	/*Obtain disk and memory MPI derived datatype */
	if(H5S_mpio_space_type(file_space,H5T_get_size(io_info->dset->shared->type),
			       &mpi_file_type,&mpi_file_count,&mpi_file_offset,&mft_is_derived)<0)
	       HGOTO_ERROR(H5E_DATASPACE, H5E_BADTYPE, FAIL,"couldn't create MPI file type");

	if(H5S_mpio_space_type(mem_space,H5T_get_size(io_info->dset->shared->type),
			       &mpi_buf_type,&mpi_buf_count,&mpi_buf_offset,&mbt_is_derived)<0)
	       HGOTO_ERROR(H5E_DATASPACE, H5E_BADTYPE, FAIL,"couldn't create MPI buffer type");
	    
      }
      else {
	    /* For non-selection, participate with a none MPI derived datatype, the count is 0.  */
	    mpi_buf_type   = MPI_BYTE;   
	    mpi_file_type  = MPI_BYTE;
	    mpi_file_count = 0;
	    mpi_buf_count  = 0;
      }

      coll_info.mbt_is_derived = mbt_is_derived;
      coll_info.mft_is_derived = mft_is_derived;
      coll_info.mpi_buf_count  = mpi_buf_count;
      coll_info.chunk_addr     = addr;

#ifdef KENT
printf("before final collective IO\n");
#endif
      if(H5D_final_collective_io(io_info,&mpi_file_type,&mpi_buf_type,&coll_info,buf,do_write)<0)
	    HGOTO_ERROR(H5E_IO, H5E_CANTGET, FAIL,"couldn't finish collective MPI-IO");
 done:

      FUNC_LEAVE_NOAPI(ret_value)
} /* end H5D_inter_collective_io */
/*-------------------------------------------------------------------------
 * Function:	H5D_final_collective_io
 *
 * Purpose:	Routine for the common part of collective IO with different storages.
		
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t 
H5D_final_collective_io(H5D_io_info_t *io_info,MPI_Datatype*mpi_file_type,MPI_Datatype *mpi_buf_type,
			 H5D_common_coll_info_t* coll_info, void *buf, hbool_t do_write) 
{


    int               mpi_code;              /* MPI return code */ 
    hbool_t	      plist_is_setup=0;      /* Whether the dxpl has been customized */
    herr_t            ret_value = SUCCEED;


    FUNC_ENTER_NOAPI_NOINIT(H5D_final_collective_io)

     /*
	  * Pass buf type, file type to the file driver.
     */

     if(H5FD_mpi_setup_collective(io_info->dxpl_id, *mpi_buf_type, *mpi_file_type)<0)
	HGOTO_ERROR(H5E_PLIST, H5E_CANTSET, FAIL, "can't set MPI-I/O properties");

     plist_is_setup=1;
     /*HDfprintf(stdout,"chunk addr %Hu\n",coll_info->chunk_addr);
     printf("mpi_buf_count %d\n",coll_info->mpi_buf_count);	*/ 
     if(do_write) {
	ret_value = (io_info->ops.write)(io_info,
		coll_info->mpi_buf_count,0,NULL,NULL,coll_info->chunk_addr,
                buf);
	/* Check return value from optimized write */
	if (ret_value<0) 
	    HGOTO_ERROR(H5E_DATASET, H5E_WRITEERROR, FAIL, "optimized write failed")
	      }
     else {
	 ret_value = (io_info->ops.read)(io_info,
	        coll_info->mpi_buf_count,0,NULL,NULL,coll_info->chunk_addr,
                buf);
	   /* Check return value from optimized write */
	 if (ret_value<0) 
	    HGOTO_ERROR(H5E_DATASET, H5E_READERROR, FAIL, "optimized read failed")
	      }
 done:
   
     /* Reset the dxpl settings */
      if(plist_is_setup) {
        if(H5FD_mpi_teardown_collective(io_info->dxpl_id)<0)
    	    HDONE_ERROR(H5E_DATASPACE, H5E_CANTFREE, FAIL, "unable to reset dxpl values");
      } /* end if */

     /* free the MPI buf and file types */
      if (coll_info->mbt_is_derived) {
	if (MPI_SUCCESS != (mpi_code= MPI_Type_free( mpi_buf_type )))
            HMPI_DONE_ERROR(FAIL, "MPI_Type_free failed", mpi_code);
	
      }
      if (coll_info->mft_is_derived) {
	if (MPI_SUCCESS != (mpi_code= MPI_Type_free( mpi_file_type )))
            HMPI_DONE_ERROR(FAIL, "MPI_Type_free failed", mpi_code);
      }

      FUNC_LEAVE_NOAPI(ret_value)
}/* end H5D_final_collective_io */

#ifdef OLD_WAY
/*-------------------------------------------------------------------------
 * Function:	H5D_pre_sort_chunk
 *
 * Purpose:	Routine to obtain addresses of all chunks for all processes

   Description:
                root will collective all chunk addresses and broadcast towards other processes.
   
   Parameters:

                Input: H5D_io_info_t* io_info,
		       int total_chunks,
		Output: haddr_t total_chunk_addr_array[], : array to store sorted total chunk addresses
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5D_pre_sort_chunk(H5D_io_info_t *io_info,int total_chunks,haddr_t total_chunk_addr_array[]){

  int root, mpi_rank;
  MPI_Comm comm;
  int mpi_code;
  MPI_Datatype chunk_addrtype;
  int mpi_type_cleanup = 0;
  herr_t ret_value = SUCCEED;

  FUNC_ENTER_NOAPI_NOINIT(H5D_pre_sort_chunk)

  root = 0;
  comm = io_info->comm;
  if(MPI_SUCCESS !=(mpi_code = MPI_Comm_rank(comm,&mpi_rank)))
    HMPI_GOTO_ERROR(FAIL, "MPI_Comm_rank failed", mpi_code);

    /*Create received  MPI derived datatype */
  if(MPI_SUCCESS !=(mpi_code = MPI_Type_contiguous(sizeof(haddr_t)*total_chunks,MPI_BYTE,&chunk_addrtype)))
    HMPI_GOTO_ERROR(FAIL, "MPI_Type_contiguous failed", mpi_code);
  if(MPI_SUCCESS != (mpi_code = MPI_Type_commit(&chunk_addrtype)))
    HMPI_GOTO_ERROR(FAIL, "MPI_Type_commit failed", mpi_code);

  
  mpi_type_cleanup = 1;

  if(mpi_rank == root) {
    if(H5D_istore_chunkmap(io_info,total_chunks,total_chunk_addr_array)<0)
       HGOTO_ERROR(H5E_DATASET, H5E_CANTGET, FAIL, "can't get chunk address");

  }
  /* Broadcasting the MPI_IO option info. and chunk address info. */
   if(MPI_SUCCESS !=(mpi_code = MPI_Bcast(total_chunk_addr_array,1,chunk_addrtype,root,comm)))
     HMPI_GOTO_ERROR(FAIL, "MPI_BCast failed", mpi_code);
  
done:

   if(mpi_type_cleanup){
      if (MPI_SUCCESS != (mpi_code= MPI_Type_free( &chunk_addrtype )))
            HMPI_DONE_ERROR(FAIL, "MPI_Type_free failed", mpi_code);
   }
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5D_pre_sort_chunk() */

#endif
/*-------------------------------------------------------------------------
 * Function:	H5D_sort_chunk
 *
 * Purpose:	Routine to sort chunks in increasing order of chunk address
                Each chunk address is also obtained.

   Description:
                For most cases, the chunk address has already been sorted in increasing order.
		The special sorting flag is used to optimize this common case.
                quick sort is used for necessary sorting.
   
   Parameters:
                Input: H5D_io_info_t* io_info,
		       fm_map *fm(global chunk map struct)
		Input/Output:  H5D_chunk_addr_info_t chunk_addr_info_array[]   : array to store chunk address and information 
                       many_chunk_opt                         : flag to optimize the way to obtain chunk addresses 
                                                                for many chunks
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */

static herr_t 
H5D_sort_chunk(H5D_io_info_t * io_info,
	       fm_map *fm,
	       H5D_chunk_addr_info_t chunk_addr_info_array[],
	       int many_chunk_opt)
{


    H5SL_node_t      *chunk_node;         /* Current node in chunk skip list */
    H5D_chunk_info_t *chunk_info;         /* Current chunking info. of this node. */
    haddr_t           chunk_addr;         /* Current chunking address of this node */
    haddr_t          *total_chunk_addr_array; /* The array of chunk address for the total number of chunk */
    int               i,j,k,mpi_code;
    int               total_chunks;
    size_t            num_chunks;
    int               mpi_type_cleanup    = 0;
    int               tchunk_addr_cleanup = 0;
    MPI_Datatype      chunk_addrtype;
    H5D_storage_t     store;              /*union of EFL and chunk pointer in file space */
    hbool_t           do_sort = FALSE;
    herr_t	      ret_value = SUCCEED;	/*return value		*/
  
    FUNC_ENTER_NOAPI_NOINIT(H5D_sort_chunk)

    num_chunks =  H5SL_count(fm->fsel);
    /* If we need to optimize the way to obtain the chunk address */
    if(many_chunk_opt != H5D_OBTAIN_ONE_CHUNK_ADDR_IND){

      total_chunks = (int)fm->total_chunks;
      total_chunk_addr_array = H5MM_malloc(sizeof(haddr_t)*total_chunks);
      tchunk_addr_cleanup = 1;

      if(many_chunk_opt == H5D_OBTAIN_ALL_CHUNK_ADDR_COL) {/* We will broadcast the array from the root process */

	int mpi_rank, root,mpi_code;
	root = 0;
	if((mpi_rank = H5F_mpi_get_rank(io_info->dset->oloc.file))<0)
	 HGOTO_ERROR (H5E_IO, H5E_MPI, FAIL, "unable to obtain mpi rank");

	/*Create received  MPI derived datatype */
	if(MPI_SUCCESS !=(mpi_code = MPI_Type_contiguous(sizeof(haddr_t)*total_chunks,MPI_BYTE,&chunk_addrtype)))
	  HMPI_GOTO_ERROR(FAIL, "MPI_Type_contiguous failed", mpi_code);
	if(MPI_SUCCESS !=(mpi_code = MPI_Type_commit(&chunk_addrtype)))
	  HMPI_GOTO_ERROR(FAIL, "MPI_Type_commit failed", mpi_code);

	mpi_type_cleanup = 1;

	if(mpi_rank == root) {
	   if(H5D_istore_chunkmap(io_info,total_chunks,total_chunk_addr_array,fm->down_chunks)<0)
	     HGOTO_ERROR(H5E_DATASET, H5E_CANTGET, FAIL, "can't get chunk address");
	}
	/* Broadcasting the MPI_IO option info. and chunk address info. */
	if(MPI_SUCCESS !=(mpi_code = MPI_Bcast(total_chunk_addr_array,1,chunk_addrtype,root,io_info->comm)))
	   HMPI_GOTO_ERROR(FAIL, "MPI_BCast failed", mpi_code);
      }

      else { /* Obtain all chunk addresses independently */
	if(H5D_istore_chunkmap(io_info,total_chunks,total_chunk_addr_array,fm->down_chunks)<0)
	     HGOTO_ERROR(H5E_DATASET, H5E_CANTGET, FAIL, "can't get chunk address");
      }
    }

    /* Get first node in chunk skip list */
    if(NULL ==(chunk_node = H5SL_first(fm->fsel)))
	  HGOTO_ERROR(H5E_STORAGE, H5E_CANTGET, FAIL,"couldn't get chunk node from skipped list");
    /* Set dataset storage for I/O info */
    io_info->store = &store;
    if(NULL ==(chunk_info = H5SL_item(chunk_node)))
	  HGOTO_ERROR(H5E_STORAGE, H5E_CANTGET, FAIL,"couldn't get chunk info from skipped list");
    store.chunk.offset = chunk_info->coords;
    store.chunk.index  = chunk_info->index;
    i = 0;
    if(many_chunk_opt == H5D_OBTAIN_ONE_CHUNK_ADDR_IND){
      if(HADDR_UNDEF==(chunk_addr = H5D_istore_get_addr(io_info,NULL)))
	  HGOTO_ERROR(H5E_STORAGE, H5E_CANTGET, FAIL,"couldn't get chunk info from skipped list");
    }
    else 
       chunk_addr = total_chunk_addr_array[chunk_info->index];
    chunk_addr_info_array[i].chunk_addr  = chunk_addr;
    chunk_addr_info_array[i].chunk_info  = *chunk_info;

    chunk_node = H5SL_next(chunk_node);
    while(chunk_node) {

            chunk_info         = H5SL_item(chunk_node);
            store.chunk.offset = chunk_info->coords;
            store.chunk.index  = chunk_info->index;
	    
	    if(many_chunk_opt == H5D_OBTAIN_ONE_CHUNK_ADDR_IND){
	      if(HADDR_UNDEF==(chunk_addr = H5D_istore_get_addr(io_info,NULL)))
		HGOTO_ERROR(H5E_STORAGE, H5E_CANTGET, FAIL,"couldn't get chunk info from skipped list");
	    }
	    else 
	      chunk_addr = total_chunk_addr_array[chunk_info->index];

	    if(chunk_addr < chunk_addr_info_array[i].chunk_addr) do_sort = TRUE;
	    chunk_addr_info_array[i+1].chunk_addr = chunk_addr;
            chunk_addr_info_array[i+1].chunk_info =*chunk_info;
            i++;
            chunk_node = H5SL_next(chunk_node);
    }

    if(do_sort)
	    HDqsort(chunk_addr_info_array,num_chunks,sizeof(chunk_addr_info_array),H5D_cmp_chunk_addr);

done:

    if(tchunk_addr_cleanup)
       HDfree(total_chunk_addr_array);
    if(mpi_type_cleanup) {
      if (MPI_SUCCESS != (mpi_code= MPI_Type_free( &chunk_addrtype )))
            HMPI_DONE_ERROR(FAIL, "MPI_Type_free failed", mpi_code);
    }
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5D_sort_chunk() */
    

/*-------------------------------------------------------------------------
 * Function:	H5D_obtain_mpio_mode
 *
 * Purpose:	Routine to obtain each io mode(collective,independent or none) for each chunk;
                Each chunk address is also obtained.

   Description:

                1) Each process provides two piece of information for all chunks with selection
		   a) chunk index 
                   b) wheather this chunk is regular(for MPI derived datatype not working case)

                2) Gather all the information to the root process
		
		3) Root process will do the following:
		   a) Obtain chunk address for all chunks in this data space
		   b) With the consideration of the user option, calculate IO mode for each chunk
		   c) Build MPI derived datatype to combine "chunk address" and "assign_io" information
		      in order to do MPI Bcast only once
                   d) MPI Bcast the IO mode and chunk address information for each chunk.
		4) Each process then retrieves IO mode and chunk address information to assign_io_mode and chunk_addr.
 
   Parameters:

                Input: H5D_io_info_t* io_info,
		       fm_map *fm,(global chunk map struct)
		Output: uint8_t assign_io_mode[], : IO mode, collective, independent or none
		        haddr_t chunk_addr[],     : chunk address array for each chunk
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */

static herr_t 
H5D_obtain_mpio_mode(H5D_io_info_t* io_info, 
		     fm_map *fm,
		     uint8_t assign_io_mode[],
		     haddr_t chunk_addr[])
{

  int               total_chunks;
  hsize_t           ori_total_chunks;
  int               percent_nproc_per_chunk,threshold_nproc_per_chunk;
  uint8_t*          io_mode_info;
  uint8_t*          recv_io_mode_info;
  uint8_t*          mergebuf;
  uint8_t*          tempbuf;

  H5SL_node_t*      chunk_node;  
  H5D_chunk_info_t* chunk_info;

  MPI_Datatype      bastype[2];
  MPI_Datatype      chunk_addrtype;
  int               bascount;
  int               basblock[2];
  MPI_Aint          basdisp[2];
  MPI_Datatype      rtype;
  MPI_Datatype      stype;
  int               mpi_size,mpi_rank;
  MPI_Comm          comm;
  int               root;
  int               mpi_code;
  int               multi_chunk_io_col_threshold;
  int               mem_cleanup      = 0,
                    mpi_type_cleanup = 0;

  herr_t            ret_value = SUCCEED;

  FUNC_ENTER_NOAPI_NOINIT(H5D_obtain_mpio_mode)

  /* Assign the rank 0 to the root */
  root              = 0; 
  comm              = io_info->comm;

  /* Obtain the number of process and the current rank of the process */
  if((mpi_rank = H5F_mpi_get_rank(io_info->dset->oloc.file))<0)
	 HGOTO_ERROR (H5E_IO, H5E_MPI, FAIL, "unable to obtain mpi rank");
  if((mpi_size = H5F_mpi_get_size(io_info->dset->oloc.file))<0)
	 HGOTO_ERROR (H5E_IO, H5E_MPI, FAIL, "unable to obtain mpi size");
  multi_chunk_io_col_threshold = H5D_MULTI_CHUNK_IO_COL_THRESHOLD; /* May replace by user-input */
  percent_nproc_per_chunk = multi_chunk_io_col_threshold;/* For example, above 50%, do collective IO */
  threshold_nproc_per_chunk = mpi_size * percent_nproc_per_chunk/100;

  /* Allocate memory */
  ori_total_chunks      = fm->total_chunks;
  H5_ASSIGN_OVERFLOW(total_chunks,ori_total_chunks,hsize_t,int);

  io_mode_info      = (uint8_t *)H5MM_calloc(total_chunks*sizeof(MPI_BYTE));
  mergebuf          = H5MM_malloc((sizeof(haddr_t)+sizeof(MPI_BYTE))*total_chunks);
  tempbuf           = mergebuf + sizeof(MPI_BYTE)*total_chunks;
  if(mpi_rank == root) 
     recv_io_mode_info = (uint8_t *)H5MM_malloc(total_chunks*sizeof(MPI_BYTE)*mpi_size);

  
  mem_cleanup       = 1;

  chunk_node        = H5SL_first(fm->fsel);

  /*Obtain the regularity and selection information for all chunks in this process. */
  while(chunk_node){

      chunk_info    = H5SL_item(chunk_node);

#ifndef H5_MPI_COMPLEX_DERIVED_DATATYPE_WORKS
      /* regularity information: 1, selection information: 2 */
      if(H5S_SELECT_IS_REGULAR(chunk_info->fspace) == TRUE &&
         H5S_SELECT_IS_REGULAR(chunk_info->mspace) == TRUE)
#endif
	io_mode_info[chunk_info->index] = H5D_CHUNK_SELECT_REG; /* this chunk is selected and is "regular" without defining H5_MPI_COMPLEX_DERIVED_DATATYPE_WORKS. */
#ifndef H5_MPI_COMPLEX_DERIVED_DATATYPE_WORKS
      else
	io_mode_info[chunk_info->index] = H5D_CHUNK_SELECT_IRREG; /* this chunk is selected and is irregular*/
#endif

      chunk_node = H5SL_next(chunk_node);
  }
  
  /*Create sent MPI derived datatype */
  if(MPI_SUCCESS !=(mpi_code = MPI_Type_contiguous(total_chunks,MPI_BYTE,&stype)))
    HMPI_GOTO_ERROR(FAIL, "MPI_Comm_rank failed", mpi_code);
  if(MPI_SUCCESS != (mpi_code = MPI_Type_commit(&stype)))
    HMPI_GOTO_ERROR(FAIL, "MPI_Type_commit failed", mpi_code);

  /*Create received basic MPI derived datatype */
  bascount      = 2;
  basblock[0]   = total_chunks;
  basblock[1]   = total_chunks;
  basdisp[0]    = 0;
  basdisp[1]    = (MPI_Aint)(sizeof(MPI_BYTE)*total_chunks);/* may need to check overflow */
  bastype[0]    = MPI_BYTE;
 
  if(MPI_SUCCESS !=(mpi_code = MPI_Type_contiguous(sizeof(haddr_t),MPI_BYTE,&chunk_addrtype)))
    HMPI_GOTO_ERROR(FAIL, "MPI_Type_contiguous failed", mpi_code);
  if(MPI_SUCCESS != (mpi_code = MPI_Type_commit(&chunk_addrtype)))
    HMPI_GOTO_ERROR(FAIL, "MPI_Type_commit failed", mpi_code);
  bastype[1]    = chunk_addrtype;

  if(MPI_SUCCESS !=(mpi_code = MPI_Type_struct(bascount,basblock,basdisp,bastype,&rtype)))
    HMPI_GOTO_ERROR(FAIL, "MPI_Type_struct failed", mpi_code);
  if(MPI_SUCCESS !=(mpi_code = MPI_Type_commit(&rtype)))
    HMPI_GOTO_ERROR(FAIL, "MPI_Type_commit failed", mpi_code);

  /* Set up a flag to clean up the MPI derived datatype later */
  mpi_type_cleanup = 1;

  /*Gather all the information */
  if(MPI_SUCCESS !=(mpi_code = MPI_Gather(io_mode_info,1,stype,recv_io_mode_info,1,stype,root,comm)))
    HMPI_GOTO_ERROR(FAIL, "MPI_Gather failed", mpi_code);

  /* Calculate the mode for IO(collective, independent or none) at root process */
  if(mpi_rank == root) {

    int               ic,nproc;
    int*              nproc_per_chunk;
#if !defined(H5_MPI_COMPLEX_DERIVED_DATATYPE_WORKS) || !defined(H5_MPI_SPECIAL_COLLECTIVE_IO_WORKS)
    int*              ind_this_chunk;
#endif

    /* pre-computing: calculate number of processes and 
        regularity of the selection occupied in each chunk */
    nproc_per_chunk = (int*)H5MM_calloc(total_chunks*sizeof(int));
#if !defined(H5_MPI_COMPLEX_DERIVED_DATATYPE_WORKS) || !defined(H5_MPI_SPECIAL_COLLECTIVE_IO_WORKS)
    ind_this_chunk   = (int*)H5MM_calloc(total_chunks*sizeof(int));
#endif

    /* calculating the chunk address */
    if(H5D_istore_chunkmap(io_info,total_chunks,chunk_addr,fm->down_chunks)<0){
      HDfree(nproc_per_chunk);
#if !defined(H5_MPI_COMPLEX_DERIVED_DATATYPE_WORKS) || !defined(H5_MPI_SPECIAL_COLLECTIVE_IO_WORKS)
      HDfree(ind_this_chunk);
#endif
      HGOTO_ERROR(H5E_DATASET, H5E_CANTGET, FAIL, "can't get chunk address");
    }

    /* checking for number of process per chunk and regularity of the selection*/
    for (nproc = 0;nproc <mpi_size;nproc++){
	uint8_t *tmp_recv_io_mode_info = recv_io_mode_info + nproc*total_chunks;
        /* calculate the number of process per chunk and adding irregular selection option */
	for(ic = 0; ic < total_chunks; ic++, tmp_recv_io_mode_info++){
		if(*tmp_recv_io_mode_info != 0) {
           		nproc_per_chunk[ic]++;
#ifndef H5_MPI_COMPLEX_DERIVED_DATATYPE_WORKS
			if(*tmp_recv_io_mode_info == H5D_CHUNK_SELECT_IRREG)
				ind_this_chunk[ic] = 1;
#endif
                }
#ifndef H5_MPI_SPECIAL_COLLECTIVE_IO_WORKS
                else {
                /*checking whether we have a selection in this chunk */
		       ind_this_chunk[ic] = 1;
        	}
#endif
         }

    }

    /* Calculating MPIO mode for each chunk (collective, independent, none) */
    for(ic = 0; ic < total_chunks; ic++){
	  if(nproc_per_chunk[ic]>=MAX(2,threshold_nproc_per_chunk)){
#if !defined(H5_MPI_COMPLEX_DERIVED_DATATYPE_WORKS) || !defined(H5_MPI_SPECIAL_COLLECTIVE_IO_WORKS)
	    if(!ind_this_chunk[ic]) assign_io_mode[ic] = H5D_CHUNK_IO_MODE_COL;
#else
	    assign_io_mode[ic] = H5D_CHUNK_IO_MODE_COL;
#endif
	  }
	
    }

    /* merge buffer io_mode info and chunk addr into one */
    HDmemcpy(mergebuf,assign_io_mode,sizeof(MPI_BYTE)*total_chunks);
    HDmemcpy(tempbuf,chunk_addr,sizeof(haddr_t)*total_chunks);

    HDfree(nproc_per_chunk);
#if !defined(H5_MPI_COMPLEX_DERIVED_DATATYPE_WORKS) || !defined(H5_MPI_SPECIAL_COLLECTIVE_IO_WORKS)
    HDfree(ind_this_chunk);
#endif
  }

   /* Broadcasting the MPI_IO option info. and chunk address info. */
   if(MPI_SUCCESS !=(mpi_code = MPI_Bcast(mergebuf,1,rtype,root,comm)))
     HMPI_GOTO_ERROR(FAIL, "MPI_BCast failed", mpi_code);

   HDmemcpy(assign_io_mode,mergebuf,sizeof(MPI_BYTE)*total_chunks);
   HDmemcpy(chunk_addr,tempbuf,sizeof(haddr_t)*total_chunks);

done:

   if(mpi_type_cleanup) {
     if (MPI_SUCCESS != (mpi_code= MPI_Type_free( &chunk_addrtype )))
            HMPI_DONE_ERROR(FAIL, "MPI_Type_free failed", mpi_code);

     if (MPI_SUCCESS != (mpi_code= MPI_Type_free( &stype )))
            HMPI_DONE_ERROR(FAIL, "MPI_Type_free failed", mpi_code);

     if (MPI_SUCCESS != (mpi_code= MPI_Type_free( &rtype )))
            HMPI_DONE_ERROR(FAIL, "MPI_Type_free failed", mpi_code);
   }

   if(mem_cleanup){
     HDfree(io_mode_info);
     HDfree(mergebuf);
     if(mpi_rank == root) 
       HDfree(recv_io_mode_info);
   }

 FUNC_LEAVE_NOAPI(ret_value)
   }/* end H5D_obtain_mpio_mode*/

static int
H5D_cmp_chunk_addr(const void *chunk_addr_info1, const void *chunk_addr_info2)
{
   haddr_t addr1, addr2;

   FUNC_ENTER_NOAPI_NOINIT(H5D_cmp_chunk_addr)
   
   addr1 = ((const H5D_chunk_addr_info_t *)chunk_addr_info1)->chunk_addr;
   addr2 = ((const H5D_chunk_addr_info_t *)chunk_addr_info2)->chunk_addr;

   FUNC_LEAVE_NOAPI(H5F_addr_cmp(addr1, addr2))

   }
#endif  /* H5_HAVE_PARALLEL */

