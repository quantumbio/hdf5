/****************************************************************************
* NCSA HDF                                                                 *
* Software Development Group                                               *
* National Center for Supercomputing Applications                          *
* University of Illinois at Urbana-Champaign                               *
* 605 E. Springfield, Champaign IL 61820                                   *
*                                                                          *
* For conditions of distribution and use, see the accompanying             *
* hdf/COPYING file.                                                        *
*                                                                          *
****************************************************************************/

#ifdef RCSID
static char RcsId[] = "@(#)$Revision$";
#endif

/* $Id$ */

/*LINTLIBRARY */
/*+
   FILE
       H5D.c
   HDF5 Dataset routines

   EXPORTED ROUTINES
       H5Dcreate    -- Create a dataset
       H5Drelease   -- Release access to a dataset

   LIBRARY-SCOPED ROUTINES

   LOCAL ROUTINES
       H5P_init_interface    -- initialize the interface
   + */

#include <H5private.h>  /* Generic Functions */
#include <H5Aprivate.h> /* Atoms */
#include <H5Dprivate.h> /* Dataset functions */
#include <H5Eprivate.h> /* Error handling */
#include <H5Gprivate.h> /* Group headers */
#include <H5Mprivate.h> /* Meta data */
#include <H5MFprivate.h> /* File space allocation header */
#include <H5MMprivate.h> /* Memory management */
#include <H5Mprivate.h> /* Meta-Object API */
#include <H5Oprivate.h> /* Object headers */

#define PABLO_MASK	H5D_mask

/*
 * A dataset is the following struct.
 */
typedef struct H5D_t {
   hdf5_file_t	*file; 		/* File store for this object		   */
   H5G_entry_t	*ent; 		/* Cached object header stuff		   */
   h5_datatype_t *type; 	/* Datatype of this dataset		   */
   H5P_dim_t	*dim; 		/* Dimensionality of this dataset	   */
   haddr_t	data_addr; 	/* Data storage address			   */
   hbool_t	dirty;		/* Header messages not updated yet	   */
} H5D_t;

/*--------------------- Locally scoped variables -----------------------------*/

/* Whether we've installed the library termination function yet for this interface */
static intn interface_initialize_g = FALSE;
static herr_t H5D_init_interface(void);

/*--------------------------------------------------------------------------
NAME
   H5D_init_interface -- Initialize interface-specific information
USAGE
    herr_t H5D_init_interface()
   
RETURNS
   SUCCEED/FAIL
DESCRIPTION
    Initializes any interface-specific data or routines.

--------------------------------------------------------------------------*/
static herr_t H5D_init_interface(void)
{
    herr_t ret_value = SUCCEED;
    FUNC_ENTER (H5D_init_interface, NULL, FAIL);

    /* Initialize the atom group for the file IDs */
    if((ret_value=H5Ainit_group(H5_DATASET,H5A_DATASETID_HASHSIZE,H5D_RESERVED_ATOMS))!=FAIL)
        ret_value=H5_add_exit(&H5D_term_interface);

    FUNC_LEAVE(ret_value);
}	/* H5D_init_interface */

/*--------------------------------------------------------------------------
 NAME
    H5D_term_interface
 PURPOSE
    Terminate various H5D objects
 USAGE
    void H5D_term_interface()
 RETURNS
    SUCCEED/FAIL
 DESCRIPTION
    Release the atom group and any other resources allocated.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
     Can't report errors...
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
void H5D_term_interface (void)
{
    H5Adestroy_group(H5_DATASET);
} /* end H5D_term_interface() */

/*--------------------------------------------------------------------------
 NAME
    H5D_create
 PURPOSE
    Create a new HDF5 dataset object
 USAGE
    hatom_t H5D_create(owner_id, type, name)
        hatom_t owner_id;       IN: Group/file which owns this object
        hobjtype_t type;        IN: Type of object to create
        const char *name;       IN: Name of the object
 RETURNS
    Returns ID (atom) on success, FAIL on failure
 DESCRIPTION
    This function actually creates a dataset object in a file (of course,
    output might not happen for some time).
--------------------------------------------------------------------------*/
hatom_t H5D_create(hatom_t owner_id, hobjtype_t type, const char *name)
{
    H5D_t *new_dset;        /* new dataset object to create */
    hatom_t ret_value = SUCCEED;
    hdf5_file_t *file = NULL;
    H5G_entry_t	tmp_ent;
    
    FUNC_ENTER(H5D_create, H5D_init_interface, FAIL);

    /* Clear errors and check args and all the boring stuff. */
    H5ECLEAR;

    /* Convert atom arguments to pointers */
    if(H5Aatom_group(owner_id)!=H5_FILE)
        HGOTO_ERROR(H5E_ATOM, H5E_BADTYPE, FAIL);
    file = H5Aatom_object (owner_id);
    
    /* Allocate space for the new dataset */
    if((new_dset=HDcalloc(1, sizeof(H5D_t)))==NULL)
        HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, FAIL);
    
    /* Initialize the dataset object */
    new_dset->file = file;
    new_dset->type=NULL;		/* No type yet */
    new_dset->dim=NULL;			/* No dimensions yet */
    new_dset->data_addr = -1;		/* No data yet */
    new_dset->dirty = FALSE;		/* There are no messages yet */

    /* Create the (empty) object header in the file */
    memset (&tmp_ent, 0, sizeof(tmp_ent));
    if ((tmp_ent.header = H5O_new (file, 0, H5D_MINHDR_SIZE))<0) {
       HGOTO_ERROR (H5E_SYM, H5E_CANTINIT, FAIL); /*can't create header*/
    }

    /*
     * Link the (empty) object header into the symbol table so others can
     * access it.  At least that way no one will try to create another one of
     * these things with the same name.  We also hold the object open while
     * H5D has a handle, preventing others from moving or deleting it from
     * under us.
     */
#ifdef LATER
    /* We should use the real CWD instead of always the root object! */
#else
    H5G_shadow_sync (file->root_sym);
    if (H5G_insert (file, file->root_sym, NULL, name, &tmp_ent)<0) {
       /* Something by that name already exists, or some other failure */
       HGOTO_ERROR (H5E_SYM, H5E_EXISTS, FAIL);
    }
    if (NULL==(new_dset->ent = H5G_open (file, file->root_sym, name))) {
       /* Can't open the header we just created -- should never happen */
       HGOTO_ERROR (H5E_SYM, H5E_NOTFOUND, FAIL);
    }
#endif

    /* Register the new datatype and get an ID for it */
    if((ret_value=H5Aregister_atom(H5_DATASET, (const VOIDP)new_dset))<0)
        HGOTO_ERROR(H5E_ATOM, H5E_CANTREGISTER, FAIL);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

    /* Normal function cleanup */

    FUNC_LEAVE(ret_value);
} /* end H5D_create() */

/*--------------------------------------------------------------------------
 NAME
    H5D_find_name
 PURPOSE
    Get the OID for accessing an existing HDF5 dataset object
 USAGE
    hoid_t H5D_find_name(grp_id, type, name)
        hatom_t grp_id;         IN: Atom for directory to search for dataset
        hobjtype_t type;        IN: Type of object to search for (dataset in
 	                        this case)
        const char *name;       IN: Name of the object to search for
 RETURNS
    Returns ID (atom) on success, FAIL on failure
 DESCRIPTION
        This function finds for a dataset by name in a directory.
--------------------------------------------------------------------------*/
hatom_t H5D_find_name(hatom_t grp_id, hobjtype_t type, const char *name)
{
    hdf5_file_t *file;          /* Pointer to the file-store of this object */
    H5D_t	*dset = NULL;	/* The dataset				    */
    hatom_t ret_value = SUCCEED;
    H5O_std_store_t store;

    FUNC_ENTER(H5D_find_name, H5D_init_interface, FAIL);

    /* Clear errors and check args and all the boring stuff. */
    H5ECLEAR;

    /* Convert atom arguments to pointers */
    if(H5Aatom_group(grp_id)!=H5_FILE)
        HGOTO_ERROR(H5E_ATOM, H5E_BADTYPE, FAIL);
    file=H5Aatom_object(grp_id);

    /* Allocate space for the dataset */
    if(NULL==(dset=HDcalloc(1, sizeof(H5D_t))))
        HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, FAIL);

    /* Initialize file, directory, name fields */
    dset->file = file;
    dset->dirty = FALSE;
    
#ifdef LATER
    /* We should really use the real CWD instead of the root object! */
#else
    /*
     *                     WARNING! WARNING! WARNING!
     * The following line explicitly uses the root symbol as the current
     * working directory.  This should be changed to something more
     * appropriate and is only hacked in here to get the prototype working.
     * -QAK
     *                     WARNING! WARNING! WARNING!
     */
    H5G_shadow_sync (file->root_sym);
    if (NULL==(dset->ent=H5G_open (file, file->root_sym, name))) {
       HGOTO_ERROR (H5E_DATASET, H5E_NOTFOUND, FAIL);
    }
#endif

    /* Get the dataset's type (currently only atomic types) */
    if((dset->type=HDcalloc(1,sizeof(h5_datatype_t)))==NULL)
        HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, FAIL);
    if (NULL==H5O_read (dset->file, NO_ADDR, dset->ent, H5O_SIM_DTYPE, 0,
			dset->type))
        HGOTO_ERROR(H5E_OHDR, H5E_NOTFOUND, FAIL);
    
    /* Get the dataset's dimensionality (currently only simple dataspaces) */
    if((dset->dim=HDcalloc(1,sizeof(H5P_dim_t)))==NULL)
        HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, FAIL);
    dset->dim->type=H5P_TYPE_SIMPLE;    /* for now... */
    if (NULL==(dset->dim->s=H5O_read (dset->file, NO_ADDR, dset->ent,
				      H5O_SIM_DIM, 0, NULL)))
        HGOTO_ERROR(H5E_OHDR, H5E_NOTFOUND, FAIL);

    /* Get the dataset's data offset (currently only standard storage) */
    if (NULL==H5O_read (dset->file, NO_ADDR, dset->ent, H5O_STD_STORE, 0,
			&store))
        HGOTO_ERROR(H5E_OHDR, H5E_NOTFOUND, FAIL);
    dset->data_addr=store.off;

    /* Register the new OID and get an ID for it */
    if((ret_value=H5Aregister_atom(H5_DATASET, (const VOIDP)dset))==FAIL)
        HGOTO_ERROR(H5E_ATOM, H5E_CANTREGISTER, FAIL);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */
       if (dset) {
#ifdef LATER
	  /* We might need to free the `type' and `dim' fields also... */
#endif
	  H5MM_xfree (dset);
       }
    } /* end if */

    /* Normal function cleanup */

    FUNC_LEAVE(ret_value);
} /* end H5D_find_name() */

/*--------------------------------------------------------------------------
 NAME
    H5Dset_info
 PURPOSE
    Set the type and dimensionality of a dataset.
 USAGE
    herr_t H5Dset_info(oid)
        hatom_t oid;       IN: Dataset object to modify
        hatom_t tid;       IN: Datatype object to use as node element
        hatom_t did;       IN: Dimensionality object to use as dataspace
 RETURNS
    SUCCEED/FAIL
 DESCRIPTION
        This function sets the datatype and dataspace of a dataset.
--------------------------------------------------------------------------*/
herr_t H5Dset_info(hatom_t oid, hatom_t tid, hatom_t did)
{
    H5D_t	*dataset = NULL;         /* dataset object to modify */
    herr_t      ret_value = SUCCEED;

    FUNC_ENTER(H5Dset_info, H5D_init_interface, FAIL);

    /* Clear errors and check args and all the boring stuff. */
    H5ECLEAR;

    /* Check that we've received correctly typed parameters */
    if(H5Aatom_group(tid)!=H5_DATATYPE || H5Aatom_group(did)!=H5_DATASPACE)
        HGOTO_ERROR(H5E_ATOM, H5E_BADTYPE, FAIL);

    /* Get the object */
    if((dataset=H5Aatom_object(oid))==NULL)
        HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL);
    /* Check that the datatype & dataspace haven't already been initialized */
    if(dataset->type || dataset->dim)
        HGOTO_ERROR(H5E_FUNC, H5E_ALREADYINIT, FAIL);

    /* Update the dataset in memory */
    dataset->type=H5Aatom_object(tid);
    dataset->dim=H5Aatom_object(did);
    dataset->dirty = TRUE;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

    /* Normal function cleanup */

    FUNC_LEAVE(ret_value);
} /* end H5Dset_info() */

/*--------------------------------------------------------------------------
 NAME
    H5Dget_info
 PURPOSE
    Get the type and dimensionality of a dataset.
 USAGE
    herr_t H5Dget_info(oid, tid, sid)
        hatom_t oid;       IN: Dataset object to query
        hatom_t *tid;      OUT: Datatype object to use as node element
        hatom_t *sid;      OUT: Dimensionality object to use as dataspace
 RETURNS
    SUCCEED/FAIL
 DESCRIPTION
        This function starts access to the datatype and dataspace of an
    existing dataset.  H5Mendaccess must be called to release the datatype and
    dataspace returned from this function.
--------------------------------------------------------------------------*/
herr_t H5Dget_info(hatom_t oid, hatom_t *tid, hatom_t *sid)
{
    H5D_t 	  *dataset;         /* dataset object to query */
    herr_t        ret_value = SUCCEED;

    FUNC_ENTER(H5Dget_info, H5D_init_interface, FAIL);

    /* Clear errors and check args and all the boring stuff. */
    H5ECLEAR;

    /* Check that we've received correctly typed parameters */
    if(H5Aatom_group(oid)!=H5_DATASET)
        HGOTO_ERROR(H5E_ATOM, H5E_BADTYPE, FAIL);

    /* Get the object */
    if((dataset=H5Aatom_object(oid))==NULL)
        HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL);
    /* Check that the datatype & dataspace have already been initialized */
    if(dataset->type==NULL || dataset->dim==NULL)
        HGOTO_ERROR(H5E_DATASET, H5E_UNINITIALIZED, FAIL);

    if((*tid=H5Aregister_atom(H5_DATATYPE, (const VOIDP)(dataset->type)))<0)
        HGOTO_ERROR(H5E_ATOM, H5E_CANTREGISTER, FAIL);
    if((*sid=H5Aregister_atom(H5_DATASPACE, (const VOIDP)(dataset->dim)))<0)
        HGOTO_ERROR(H5E_ATOM, H5E_CANTREGISTER, FAIL);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

    /* Normal function cleanup */

    FUNC_LEAVE(ret_value);
} /* end H5Dset_info() */

/*--------------------------------------------------------------------------
 NAME
    H5Dread
 PURPOSE
    Read data from a dataset
 USAGE
    herr_t H5Dread(oid)
        hatom_t oid;       IN: Dataset to read
        hatom_t did;       IN: Dimensionality object to use as dataspace for I/O
        VOIDP buf;         IN: Buffer to fill with data from the file
 RETURNS
    SUCCEED/FAIL
 DESCRIPTION
        This function reads dataset object data from the file.  The dataspace
    ID determines the slice/hyper-slab/portion of the dataset to write.
    H5P_SCALAR is a special value which indicates that the entire dataset is
    to be written out.  (For datasets which have a scalar dataspace for the
    entire dataset, this is somewhat redundant.... :-)
--------------------------------------------------------------------------*/
herr_t H5Dread(hatom_t oid, hatom_t did, VOIDP buf)
{
    H5D_t *dataset; /* dataset object to do I/O on */
    void *readbuf;         /* pointer to buffer to write out */
    uintn   free_buf=0;     /* if temporary conversion buffer needs to be free'd */
    uintn   toread;         /* number of bytes to read in */
    herr_t  ret_value = SUCCEED;

    FUNC_ENTER(H5Dread, H5D_init_interface, FAIL);

    /* Clear errors and check args and all the boring stuff. */
    H5ECLEAR;

    /* Check that we've received correctly typed parameters */
    if(H5Aatom_group(did)!=H5_DATASPACE || H5Aatom_group(oid)!=H5_DATASET)
        HGOTO_ERROR(H5E_ATOM, H5E_BADTYPE, FAIL);
    if(buf==NULL)
        HGOTO_ERROR(H5E_ARGS, H5E_BADRANGE, FAIL);

    /* Get the object */
    if((dataset=H5Aatom_object(oid))==NULL)
        HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL);

    /* Check that the datatype & dataspace have already been initialized */
    if(dataset->type==NULL || dataset->dim==NULL || dataset->data_addr<0)
        HGOTO_ERROR(H5E_FUNC, H5E_UNINITIALIZED, FAIL);

    /* Compute the number of bytes to read */
    if(did==H5P_SCALAR) /* Check if we are reading the entire dataset */
        toread=H5T_size(dataset->type,BTRUE)*H5P_nelem(dataset->dim);
    else
        HGOTO_ERROR(H5E_INTERNAL, H5E_UNSUPPORTED, FAIL);

    /* Check memory to disk datatype conversions, etc. */
/* This is totally hacked up code, but I'm in a hurry. ;-/ -QAK */
    if(dataset->type->dt.arch!=H5T_ARCH_TYPE)
      {
        if((readbuf=HDmalloc(toread))==NULL)
            HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, FAIL);
        free_buf=1;
      } /* end if */
    else
        readbuf=buf;
    
    
    /* Read data from disk */
    if(H5F_block_read(dataset->file,dataset->data_addr,toread,readbuf)==FAIL)
        HGOTO_ERROR(H5E_IO, H5E_READERROR, FAIL);
    
    if(free_buf!=0)
        H5D_convert_buf(buf,readbuf,toread,dataset->type->dt.len);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

    /* Normal function cleanup */
    if(free_buf!=0) /* check if we need to release the conversion buffer */
        HDfree(readbuf);

    FUNC_LEAVE(ret_value);
} /* end H5Dread() */

/*--------------------------------------------------------------------------
 NAME
    H5Dwrite
 PURPOSE
    Write data for a dataset
 USAGE
    herr_t H5Dwrite(oid)
        hatom_t oid;       IN: Dataset object to modify
        hatom_t did;       IN: Dimensionality object to use as dataspace for I/O
        VOIDP buf;         IN: Buffer with data to write to the file
 RETURNS
    SUCCEED/FAIL
 DESCRIPTION
        This function writes dataset object data to the file.  The dataspace
    ID determines the slice/hyper-slab/portion of the dataset to write.
    H5P_SCALAR is a special value which indicates that the entire dataset is
    to be written out.  (For datasets which have a scalar dataspace for the
    entire dataset, this is somewhat redundant.... :-)
--------------------------------------------------------------------------*/
herr_t H5Dwrite(hatom_t oid, hatom_t did, VOIDP buf)
{
    H5D_t *dataset;         /* dataset object to do I/O on */
    uintn   towrite;        /* number of bytes to write out */
    void *writebuf;         /* pointer to buffer to write out */
    uintn   free_buf=0;     /* if temporary conversion buffer needs to be free'd */
    herr_t        ret_value = SUCCEED;

    FUNC_ENTER(H5Dwrite, H5D_init_interface, FAIL);

    /* Clear errors and check args and all the boring stuff. */
    H5ECLEAR;

    /* Check that we've received correctly typed parameters */
    if(H5Aatom_group(did)!=H5_DATASPACE || H5Aatom_group(oid)!=H5_DATASET)
        HGOTO_ERROR(H5E_ATOM, H5E_BADTYPE, FAIL);
    if(buf==NULL)
        HGOTO_ERROR(H5E_ARGS, H5E_BADRANGE, FAIL);

    /* Get the object */
    if((dataset=H5Aatom_object(oid))==NULL)
        HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL);
        
    /* Check that the datatype & dataspace have already been initialized */
    if(dataset->type==NULL || dataset->dim==NULL)
        HGOTO_ERROR(H5E_FUNC, H5E_UNINITIALIZED, FAIL);

    /* Compute the number of bytes to write out */
    if(did==H5P_SCALAR) /* Check if we are writing out the entire dataset */
        towrite=H5T_size(dataset->type,BTRUE)*H5P_nelem(dataset->dim);
    else
        HGOTO_ERROR(H5E_INTERNAL, H5E_UNSUPPORTED, FAIL);

    /* Check if we have space for the dataset yet */
    if(dataset->data_addr<0) {
        if((dataset->data_addr=H5MF_alloc(dataset->file,towrite))<0)
            HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, FAIL);
	dataset->dirty = TRUE;
    }

    /* Check memory to disk datatype conversions, etc. */
/* This is totally hacked up code, but I'm in a hurry. ;-/ -QAK */
    if(dataset->type->dt.arch!=H5T_ARCH_TYPE)
      {
        if((writebuf=HDmalloc(towrite))==NULL)
            HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, FAIL);
        H5D_convert_buf(writebuf,buf,towrite,dataset->type->dt.len);
        free_buf=1;
      } /* end if */
    else
        writebuf=buf;
    
    /* Write the data out to disk */
    if(H5F_block_write(dataset->file,dataset->data_addr,towrite,writebuf)<0)
        HGOTO_ERROR(H5E_IO, H5E_WRITEERROR, FAIL);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

    /* Normal function cleanup */
    if(free_buf!=0) /* check if we need to release the conversion buffer */
        HDfree(writebuf);

    FUNC_LEAVE(ret_value);
} /* end H5Dwrite() */

/*--------------------------------------------------------------------------
 NAME
    H5D_flush
 PURPOSE
    Flush an an HDF5 dataset object to disk.
 USAGE
    herr_t H5D_flush(oid)
        hatom_t oid;       IN: Object to flush to disk
 RETURNS
    SUCCEED/FAIL
 DESCRIPTION
        This function flushes a dataset to disk.  (i.e. makes the disk version
    agree with what's in memory, it does _not_ update the memory version with
    any changes on disk)  This function is primarily called from H5Mflush, but
    internal library routines may call it also.
--------------------------------------------------------------------------*/
herr_t H5D_flush(hatom_t oid)
{
    H5D_t 	*dataset;         /* dataset object to release */
    herr_t	ret_value = SUCCEED;

    FUNC_ENTER(H5D_flush, H5D_init_interface, FAIL);

    /* Clear errors and check args and all the boring stuff. */
    H5ECLEAR;

    /* Get the object */
    if((dataset=H5Aatom_object(oid))==NULL)
        HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL);
    
    if (dataset->dirty) {

	/*
	 * Modify/create messages for this dataset.  Begin with the
	 * type information.
	 */
	if (H5T_is_atomic (dataset->type)) {
	   if (H5O_modify (dataset->file, NO_ADDR, dataset->ent,
			   H5O_SIM_DTYPE, 0, dataset->type)<0) {
	      /* Can't create/update type message */
	      HGOTO_ERROR (H5E_INTERNAL, H5E_CANTCREATE, FAIL);
	   }
	} else {
	   /* Not an atomic datatype */
	   HGOTO_ERROR (H5E_INTERNAL, H5E_UNSUPPORTED, FAIL);
	}

	/*
	 * Modify/create the dimensionality information.
	 */
	if (H5P_is_simple (dataset->dim)) {
	   if (H5O_modify (dataset->file, NO_ADDR, dataset->ent,
			   H5O_SIM_DIM, 0, dataset->dim->s)<0) {
	      /* Can't create/update dimensionality message */
	      HGOTO_ERROR (H5E_INTERNAL, H5E_CANTCREATE, FAIL);
	   }
	} else {
	   /* Not an atomic datatype */
	   HGOTO_ERROR (H5E_INTERNAL, H5E_UNSUPPORTED, FAIL);
	}

	/*
	 * Modify/create the dataset's storage information.
	 */
	if (dataset->data_addr>=0) {
	   H5O_std_store_t store;  /* standard storage info */

	   store.len = H5T_size (dataset->type, BTRUE) *
	               H5P_nelem (dataset->dim);
	   store.off = dataset->data_addr;
	   if (H5O_modify (dataset->file, NO_ADDR, dataset->ent,
			   H5O_STD_STORE, 0, &store)<0) {
	      /* Can't create/modify storage information */
	      HGOTO_ERROR (H5E_INTERNAL, H5E_CANTCREATE, FAIL);
	   }
	}

#if 1
	/* Flush dataset header to disk -- just for debugging */
	if (H5AC_flush (dataset->file, NULL, dataset->ent->header, FALSE)<0) {
          HRETURN_ERROR (H5E_OHDR, H5E_CANTFLUSH, FAIL);
       }
#endif
       
	dataset->dirty = FALSE;	/*it's clean now*/
    }
       
done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

    /* Normal function cleanup */

    FUNC_LEAVE(ret_value);
} /* end H5D_flush() */

/*--------------------------------------------------------------------------
 NAME
    H5D_release
 PURPOSE
    Release access to an HDF5 dataset object.
 USAGE
    herr_t H5D_release(oid)
        hatom_t oid;       IN: Object to release access to
 RETURNS
    SUCCEED/FAIL
 DESCRIPTION
        This function releases a dataset from active use by a user.
--------------------------------------------------------------------------*/
herr_t H5D_release(hatom_t oid)
{
    H5D_t 	*dataset;         /* dataset object to release */
    herr_t      ret_value = SUCCEED;

    FUNC_ENTER(H5D_release, H5D_init_interface, FAIL);

    /* Clear errors and check args and all the boring stuff. */
    H5ECLEAR;

    /* Get the dataset so we can check for changes and release it */
    if((dataset=H5Aatom_object(oid))==NULL)
        HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL);

    /* Check if we have information to flush to the file... */
    if(dataset->dirty && H5D_flush(oid)<0) {
       /* Can't flush dataset */
       HGOTO_ERROR (H5E_OHDR, H5E_CANTFLUSH, FAIL);
    }

    /* Close the dataset object */
    H5G_close (dataset->file, dataset->ent);
    dataset->ent = NULL;
    
    /* release the memory used for the dataset */
    H5MM_xfree (dataset);

    /* Delete the dataset from the atom group */
    if(H5Aremove_atom(oid)==NULL)
        HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

    /* Normal function cleanup */

    FUNC_LEAVE(ret_value);
} /* end H5D_release() */

