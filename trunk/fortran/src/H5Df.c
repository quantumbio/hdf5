#include "H5f90.h"

/*----------------------------------------------------------------------------
 * Name:        h5dcreate_c
 * Purpose:     Call H5Dcreate to create a dataset 
 * Inputs:      loc_id - file or group identifier 
 *              name - name of the dataset     
 *              namelen - name length
 *              type_id - datatype identifier
 *              space_id - dataspace identifier
 *              crt_pr  - identifier of creation property list
 * Outputs:     dset_id - dataset identifier
 * Returns:     0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Wednesday, August 4, 1999
 * Modifications:
 *---------------------------------------------------------------------------*/
int_f
nh5dcreate_c (hid_t_f *loc_id, _fcd name, int_f *namelen, hid_t_f *type_id, hid_t_f *space_id, hid_t_f *crt_prp,  hid_t_f *dset_id)
{
     int ret_value = -1;
     char *c_name;
     int c_namelen;
     hid_t c_loc_id;
     hid_t c_type_id;
     hid_t c_space_id;
     hid_t c_dset_id;
     hid_t c_crt_prp;
     
     /*
      * Define creation property
      */
     c_crt_prp = (hid_t)*crt_prp;

     /*
      * Convert FORTRAN name to C name
      */
     c_namelen = *namelen;
     c_name = (char *)HD5f2cstring(name, c_namelen); 
     if (c_name == NULL) return ret_value;

     /*
      * Call H5Dcreate function.
      */
     c_loc_id = (hid_t)*loc_id;
     c_type_id = (hid_t)*type_id;
     c_space_id = (hid_t)*space_id;
     c_dset_id = H5Dcreate(c_loc_id, c_name, c_type_id, c_space_id, c_crt_prp);
     if (c_dset_id < 0) goto DONE;
     *dset_id = (hid_t_f)c_dset_id;
     ret_value = 0;

DONE:
     HDfree(c_name);
     return ret_value;
}      

/*----------------------------------------------------------------------------
 * Name:        h5dopen_c
 * Purpose:     Call H5Dopen to open a dataset 
 * Inputs:      loc_id - file or group identifier 
 *              name - name of the dataset     
 *              namelen - name length
 * Outputs:     dset_id - dataset identifier
 * Returns:     0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Wednesday, August 4, 1999
 * Modifications:
 *---------------------------------------------------------------------------*/
int_f
nh5dopen_c (hid_t_f *loc_id, _fcd name, int_f *namelen, hid_t_f *dset_id)
{
     int ret_value = -1;
     char *c_name;
     int c_namelen;
     hid_t c_loc_id;
     hid_t c_dset_id;

     /*
      * Convert FORTRAN name to C name
      */
     c_namelen = *namelen;
     c_name = (char *)HD5f2cstring(name, c_namelen); 
     if (c_name == NULL) return ret_value;

     /*
      * Call H5Dopen function.
      */
     c_loc_id = (hid_t)*loc_id;
     c_dset_id = H5Dopen(c_loc_id, c_name);

     if (c_dset_id < 0) goto DONE;
     *dset_id = (hid_t_f)c_dset_id;
     ret_value = 0;

DONE:
     HDfree(c_name);
     return ret_value;
}      

/*----------------------------------------------------------------------------
 * Name:        h5dwritec_c
 * Purpose:     Call h5dwrite_c to write a dataset of characters 
 * Inputs:      dset_id - dataset identifier 
 *              mem_type_id - memory datatype identifier
 *              mem_space_id - memory dataspace identifier
 *              file_space_id - memory dataspace identifier
 *              xfer_pr  - identifier of transfer property list
 *              buf      - character data buffer
 * Returns:     0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Wednesday, August 6, 1999
 * Modifications:
 *---------------------------------------------------------------------------*/
int_f
nh5dwritec_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, _fcd buf, int_f *dims)
{
     int ret_value = -1;
     
     /*
      * Call h5dwrite_c  function.
      */
     ret_value = nh5dwrite_c(dset_id, mem_type_id, mem_space_id, file_space_id, xfer_prp, _fcdtocp(buf), dims);

     return ret_value;
}

/*----------------------------------------------------------------------------
 * Name:        h5dwrite_c
 * Purpose:     Call H5Dwrite to write a dataset 
 * Inputs:      dset_id - dataset identifier 
 *              mem_type_id - memory datatype identifier
 *              mem_space_id - memory dataspace identifier
 *              file_space_id - memory dataspace identifier
 *              xfer_pr  - identifier of transfer property list
 *              buf      - data buffer
 * Returns:     0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Wednesday, August 6, 1999
 * Modifications:
 *---------------------------------------------------------------------------*/
int_f
nh5dwrite_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, int_f *dims)
{
     int ret_value = -1;
     herr_t ret;
     hid_t c_dset_id;
     hid_t c_mem_type_id;
     hid_t c_mem_space_id;
     hid_t c_file_space_id;
     hid_t c_xfer_prp;

     /*
      * Define transfer property
      */
     c_xfer_prp = (hid_t)*xfer_prp;
     
     /*
      * Call H5Dwrite function.
      */
     c_dset_id = (hid_t)*dset_id;
     c_mem_type_id = (hid_t)*mem_type_id;
     c_mem_space_id = (hid_t)*mem_space_id;
     c_file_space_id = (hid_t)*file_space_id;
     ret = H5Dwrite(c_dset_id, c_mem_type_id, c_mem_space_id, c_file_space_id, c_xfer_prp, buf);

     if (ret < 0) return ret_value;
     ret_value = 0;
     return ret_value;
}      
/*----------------------------------------------------------------------------
 * Name:        h5dwritec_c_b
 * Purpose:     Call h5dwrite_c_b to write a dataset of characters 
 * Inputs:      dset_id - dataset identifier 
 *              mem_type_id - memory datatype identifier
 *              mem_space_id - memory dataspace identifier
 *              file_space_id - memory dataspace identifier
 *              xfer_pr  - identifier of transfer property list
 *              buf      - character data buffer
 * Returns:     0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Tuesday, May 14, 2002
 * Modifications: This function is added to accomodate oveloaded h5dwrite_f
 *                with the dims argument being of INTEGER(HSIZE_T) type
 *---------------------------------------------------------------------------*/
int_f
nh5dwritec_c_b (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, _fcd buf, hsize_t_f *dims)
{
     int ret_value = -1;
     
     /*
      * Call h5dwrite_c  function.
      */
     ret_value = nh5dwrite_c_b(dset_id, mem_type_id, mem_space_id, file_space_id, xfer_prp, _fcdtocp(buf), dims);

     return ret_value;
}

/*----------------------------------------------------------------------------
 * Name:        h5dwrite_c_b
 * Purpose:     Call H5Dwrite to write a dataset 
 * Inputs:      dset_id - dataset identifier 
 *              mem_type_id - memory datatype identifier
 *              mem_space_id - memory dataspace identifier
 *              file_space_id - memory dataspace identifier
 *              xfer_pr  - identifier of transfer property list
 *              buf      - data buffer
 * Returns:     0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Tuesday, May 14, 2002
 * Modifications: This function is added to accomodate oveloaded h5dwrite_f
 *                with the dims argument being of INTEGER(HSIZE_T) type
 *---------------------------------------------------------------------------*/
int_f
nh5dwrite_c_b (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims)
{
     int ret_value = -1;
     herr_t ret;
     hid_t c_dset_id;
     hid_t c_mem_type_id;
     hid_t c_mem_space_id;
     hid_t c_file_space_id;
     hid_t c_xfer_prp;

     /*
      * Define transfer property
      */
     c_xfer_prp = (hid_t)*xfer_prp;
     
     /*
      * Call H5Dwrite function.
      */
     c_dset_id = (hid_t)*dset_id;
     c_mem_type_id = (hid_t)*mem_type_id;
     c_mem_space_id = (hid_t)*mem_space_id;
     c_file_space_id = (hid_t)*file_space_id;
     ret = H5Dwrite(c_dset_id, c_mem_type_id, c_mem_space_id, c_file_space_id, c_xfer_prp, buf);

     if (ret < 0) return ret_value;
     ret_value = 0;
     return ret_value;
}      


/*----------------------------------------------------------------------------
 * Name:        h5dwrite_ref_obj_c
 * Purpose:     Call H5Dwrite to write a dataset  of object references
 * Inputs:      dset_id - dataset identifier 
 *              mem_type_id - memory datatype identifier
 *              mem_space_id - memory dataspace identifier
 *              file_space_id - memory dataspace identifier
 *              xfer_pr  - identifier of transfer property list
 *              buf      - data buffer with references to the objects.
 *              n - number of references to be stored.   
 * Returns:     0 on success,e-1 on failure
 * Programmer:  Elena Pourmal
 *              Monday, July 24, 2000 
 * Modifications:
 *---------------------------------------------------------------------------*/
int_f
nh5dwrite_ref_obj_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, int_f *buf, int_f *dims)
{
     int ret_value = -1;
     herr_t ret;
     hid_t c_dset_id;
     hid_t c_mem_type_id;
     hid_t c_mem_space_id;
     hid_t c_file_space_id;
     hid_t c_xfer_prp;
     hobj_ref_t *buf_c;
     int i, n;
     n = (int)*dims;

     /*
      * Define transfer property
      */
     c_xfer_prp = (hid_t)*xfer_prp;

     /*
      * Allocate temporary buffer and copy references from Fortran.
      */
      buf_c = (hobj_ref_t*)HDmalloc(sizeof(hobj_ref_t)*(n));
      if ( buf_c != NULL ) {
      for (i = 0; i < n; i++) {
           HDmemcpy(buf_c[i].oid, buf, H5R_OBJ_REF_BUF_SIZE);
           buf = buf + REF_OBJ_BUF_LEN_F;
      }
      }  
      else return ret_value;
     
     /*
      * Call H5Dwrite function.
      */
     c_dset_id = (hid_t)*dset_id;
     c_mem_type_id = (hid_t)*mem_type_id;
     c_mem_space_id = (hid_t)*mem_space_id;
     c_file_space_id = (hid_t)*file_space_id;
     ret = H5Dwrite(c_dset_id, c_mem_type_id, c_mem_space_id, c_file_space_id, c_xfer_prp, buf_c);
     HDfree(buf_c);
     if (ret < 0) return ret_value;
     ret_value = 0;
     return ret_value;
}      
/*----------------------------------------------------------------------------
 * Name:        h5dwrite_ref_obj_c_b
 * Purpose:     Call H5Dwrite to write a dataset  of object references
 * Inputs:      dset_id - dataset identifier 
 *              mem_type_id - memory datatype identifier
 *              mem_space_id - memory dataspace identifier
 *              file_space_id - memory dataspace identifier
 *              xfer_pr  - identifier of transfer property list
 *              buf      - data buffer with references to the objects.
 *              n - number of references to be stored.   
 * Returns:     0 on success,e-1 on failure
 * Programmer:  Elena Pourmal
 *              Tuesday, May 14, 2002
 * Modifications: This function was added to accomodate h5dwrite_f with the
 *                dims argumnet being of INTEGER(HSIZE_T) type.
 *---------------------------------------------------------------------------*/
int_f
nh5dwrite_ref_obj_c_b (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, int_f *buf, hsize_t_f *dims)
{
     int ret_value = -1;
     herr_t ret;
     hid_t c_dset_id;
     hid_t c_mem_type_id;
     hid_t c_mem_space_id;
     hid_t c_file_space_id;
     hid_t c_xfer_prp;
     hobj_ref_t *buf_c;
     int i, n;
     n = (int)*dims;

     /*
      * Define transfer property
      */
     c_xfer_prp = (hid_t)*xfer_prp;

     /*
      * Allocate temporary buffer and copy references from Fortran.
      */
      buf_c = (hobj_ref_t*)HDmalloc(sizeof(hobj_ref_t)*(n));
      if ( buf_c != NULL ) {
      for (i = 0; i < n; i++) {
           HDmemcpy(buf_c[i].oid, buf, H5R_OBJ_REF_BUF_SIZE);
           buf = buf + REF_OBJ_BUF_LEN_F;
      }
      }  
      else return ret_value;
     
     /*
      * Call H5Dwrite function.
      */
     c_dset_id = (hid_t)*dset_id;
     c_mem_type_id = (hid_t)*mem_type_id;
     c_mem_space_id = (hid_t)*mem_space_id;
     c_file_space_id = (hid_t)*file_space_id;
     ret = H5Dwrite(c_dset_id, c_mem_type_id, c_mem_space_id, c_file_space_id, c_xfer_prp, buf_c);
     HDfree(buf_c);
     if (ret < 0) return ret_value;
     ret_value = 0;
     return ret_value;

}
/*----------------------------------------------------------------------------
 * Name:        h5dwrite_ref_reg_c
 * Purpose:     Call H5Dwrite to write a dataset of dataset region references
 * Inputs:      dset_id - dataset identifier 
 *              mem_type_id - memory datatype identifier
 *              mem_space_id - memory dataspace identifier
 *              file_space_id - memory dataspace identifier
 *              xfer_pr  - identifier of transfer property list
 *              buf      - data buffer with references to the objects.
 *              n - number of references to be stored.   
 * Returns:     0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Monday, July 24, 2000 
 * Modifications:
 *---------------------------------------------------------------------------*/
int_f
nh5dwrite_ref_reg_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, int_f *buf, int_f *dims)
{
     int ret_value = -1;
     herr_t ret;
     hid_t c_dset_id;
     hid_t c_mem_type_id;
     hid_t c_mem_space_id;
     hid_t c_file_space_id;
     hid_t c_xfer_prp;
     hdset_reg_ref_t *buf_c;
     int i, n;

      n = (int)*dims;
     /*
      * Define transfer property
      */
     c_xfer_prp = (hid_t)*xfer_prp;
     
     /*
      * Allocate temporary buffer and copy references from Fortran.
      */
      buf_c = (hdset_reg_ref_t *)HDmalloc(sizeof(hdset_reg_ref_t)*(n));
      if ( buf_c != NULL ) {
      for (i = 0; i < n; i++) {
           HDmemcpy(buf_c[i].heapid, buf, H5R_DSET_REG_REF_BUF_SIZE);
           buf = buf + REF_REG_BUF_LEN_F;
      }
      }  
      else return ret_value;
        
     
     /*
      * Call H5Dwrite function.
      */
     c_dset_id = (hid_t)*dset_id;
     c_mem_type_id = (hid_t)*mem_type_id;
     c_mem_space_id = (hid_t)*mem_space_id;
     c_file_space_id = (hid_t)*file_space_id;
     ret = H5Dwrite(c_dset_id, c_mem_type_id, c_mem_space_id, c_file_space_id, c_xfer_prp, buf_c);
     HDfree(buf_c);
     if (ret < 0) return ret_value;
     ret_value = 0;
     return ret_value;
}      
/*----------------------------------------------------------------------------
 * Name:        h5dwrite_ref_reg_c_b
 * Purpose:     Call H5Dwrite to write a dataset of dataset region references
 * Inputs:      dset_id - dataset identifier 
 *              mem_type_id - memory datatype identifier
 *              mem_space_id - memory dataspace identifier
 *              file_space_id - memory dataspace identifier
 *              xfer_pr  - identifier of transfer property list
 *              buf      - data buffer with references to the objects.
 *              n - number of references to be stored.   
 * Returns:     0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Tuesday, May 14, 2002
 * Modifications: This function was added to accomodate h5dwrite_f with the
 *                dims argument being of INTEGER(HSIZE_T) type
 *---------------------------------------------------------------------------*/
int_f
nh5dwrite_ref_reg_c_b (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, int_f *buf, hsize_t_f *dims)
{
     int ret_value = -1;
     herr_t ret;
     hid_t c_dset_id;
     hid_t c_mem_type_id;
     hid_t c_mem_space_id;
     hid_t c_file_space_id;
     hid_t c_xfer_prp;
     hdset_reg_ref_t *buf_c;
     int i, n;

      n = (int)*dims;
     /*
      * Define transfer property
      */
     c_xfer_prp = (hid_t)*xfer_prp;
     
     /*
      * Allocate temporary buffer and copy references from Fortran.
      */
      buf_c = (hdset_reg_ref_t *)HDmalloc(sizeof(hdset_reg_ref_t)*(n));
      if ( buf_c != NULL ) {
      for (i = 0; i < n; i++) {
           HDmemcpy(buf_c[i].heapid, buf, H5R_DSET_REG_REF_BUF_SIZE);
           buf = buf + REF_REG_BUF_LEN_F;
      }
      }  
      else return ret_value;
        
     
     /*
      * Call H5Dwrite function.
      */
     c_dset_id = (hid_t)*dset_id;
     c_mem_type_id = (hid_t)*mem_type_id;
     c_mem_space_id = (hid_t)*mem_space_id;
     c_file_space_id = (hid_t)*file_space_id;
     ret = H5Dwrite(c_dset_id, c_mem_type_id, c_mem_space_id, c_file_space_id, c_xfer_prp, buf_c);
     HDfree(buf_c);
     if (ret < 0) return ret_value;
     ret_value = 0;
     return ret_value;
}      



/*----------------------------------------------------------------------------
 * Name:        h5dreadc_c
 * Purpose:     Call h5dread_c to read a dataset of characters 
 * Inputs:      dset_id - dataset identifier 
 *              mem_type_id - memory datatype identifier
 *              mem_space_id - memory dataspace identifier
 *              file_space_id - memory dataspace identifier
 *              xfer_pr  - identifier of transfer property list
 * Outputs:     buf      - character data buffer
 * Returns:     0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Monday, August 9, 1999
 * Modifications:
 *---------------------------------------------------------------------------*/
int_f
nh5dreadc_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, _fcd buf, int_f *dims)
{
     int ret_value = -1;
     
     /*
      * Call h5dread_c  function.
      */
     ret_value = nh5dread_c(dset_id, mem_type_id, mem_space_id, file_space_id, xfer_prp, _fcdtocp(buf), dims);

     return ret_value;
}

/*----------------------------------------------------------------------------
 * Name:        h5dread_c
 * Purpose:     Call H5Draed to read a dataset 
 * Inputs:      dset_id - dataset identifier 
 *              mem_type_id - memory datatype identifier
 *              mem_space_id - memory dataspace identifier
 *              file_space_id - memory dataspace identifier
 *              xfer_pr  - identifier of transfer property list
 * Outputs:     buf      - data buffer
 * Returns:     0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Monday, August 9, 1999
 * Modifications:
 *---------------------------------------------------------------------------*/
int_f
nh5dread_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, int_f *dims)
{
     int ret_value = -1;
     herr_t ret;
     hid_t c_dset_id;
     hid_t c_mem_type_id;
     hid_t c_mem_space_id;
     hid_t c_file_space_id;
     hid_t c_xfer_prp;

     /*
      * Define transfer property
      */
     c_xfer_prp = (hid_t)*xfer_prp;
     
     /*
      * Call H5Dread function.
      */
     c_dset_id = (hid_t)*dset_id;
     c_mem_type_id = (hid_t)*mem_type_id;
     c_mem_space_id = (hid_t)*mem_space_id;
     c_file_space_id = (hid_t)*file_space_id;
     ret = H5Dread(c_dset_id, c_mem_type_id, c_mem_space_id, c_file_space_id, c_xfer_prp, buf);

     if (ret < 0) return ret_value;
     ret_value = 0;
     return ret_value;
}
/*----------------------------------------------------------------------------
 * Name:        h5dreadc_c_b
 * Purpose:     Call h5dread_c_b to read a dataset of characters 
 * Inputs:      dset_id - dataset identifier 
 *              mem_type_id - memory datatype identifier
 *              mem_space_id - memory dataspace identifier
 *              file_space_id - memory dataspace identifier
 *              xfer_pr  - identifier of transfer property list
 * Outputs:     buf      - character data buffer
 * Returns:     0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Wednesday, May 15, 2002
 * Modifications: This function was added to accomodate h5dread_f subroutine
 *                with the dims parameter being of INTEGER(HSIZE_T_F) size.
 *---------------------------------------------------------------------------*/
int_f
nh5dreadc_c_b (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, _fcd buf, hsize_t_f *dims)
{
     int ret_value = -1;
     
     /*
      * Call h5dread_c  function.
      */
     ret_value = nh5dread_c_b(dset_id, mem_type_id, mem_space_id, file_space_id, xfer_prp, _fcdtocp(buf), dims);

     return ret_value;
}

/*----------------------------------------------------------------------------
 * Name:        h5dread_c
 * Purpose:     Call H5Draed to read a dataset 
 * Inputs:      dset_id - dataset identifier 
 *              mem_type_id - memory datatype identifier
 *              mem_space_id - memory dataspace identifier
 *              file_space_id - memory dataspace identifier
 *              xfer_pr  - identifier of transfer property list
 * Outputs:     buf      - data buffer
 * Returns:     0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Wednesday, May 15, 2002
 * Modifications: This function was added to accomodate h5dread_f subroutine
 *                with the dims parameter being of INTEGER(HSIZE_T_F) size.
 *---------------------------------------------------------------------------*/
int_f
nh5dread_c_b (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims)
{
     int ret_value = -1;
     herr_t ret;
     hid_t c_dset_id;
     hid_t c_mem_type_id;
     hid_t c_mem_space_id;
     hid_t c_file_space_id;
     hid_t c_xfer_prp;

     /*
      * Define transfer property
      */
     c_xfer_prp = (hid_t)*xfer_prp;
     
     /*
      * Call H5Dread function.
      */
     c_dset_id = (hid_t)*dset_id;
     c_mem_type_id = (hid_t)*mem_type_id;
     c_mem_space_id = (hid_t)*mem_space_id;
     c_file_space_id = (hid_t)*file_space_id;
     ret = H5Dread(c_dset_id, c_mem_type_id, c_mem_space_id, c_file_space_id, c_xfer_prp, buf);

     if (ret < 0) return ret_value;
     ret_value = 0;
     return ret_value;
}


/*----------------------------------------------------------------------------
 * Name:        h5dread_ref_obj_c
 * Purpose:     Call H5Dread to read a dataset  of object references
 * Inputs:      dset_id - dataset identifier 
 *              mem_type_id - memory datatype identifier
 *              mem_space_id - memory dataspace identifier
 *              file_space_id - memory dataspace identifier
 *              xfer_pr  - identifier of transfer property list
 *              buf      - data buffer to store references to the objects.
 *              n - number of references to be stored.   
 * Returns:     0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Monday, July 24, 2000 
 * Modifications:
 *---------------------------------------------------------------------------*/
int_f
nh5dread_ref_obj_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, int_f * buf, int_f *dims)
{
     int ret_value = -1;
     herr_t ret;
     hid_t c_dset_id;
     hid_t c_mem_type_id;
     hid_t c_mem_space_id;
     hid_t c_file_space_id;
     hid_t c_xfer_prp;
     hobj_ref_t *buf_c;
     int i, n;
     n = (int)*dims;
     /*
      * Define transfer property
      */
     c_xfer_prp = (hid_t)*xfer_prp;
     
     /*
      * Allocate temporary buffer.
      */
     buf_c = (hobj_ref_t*)HDmalloc(sizeof(hobj_ref_t)*(n));
     if ( buf_c != NULL ) {
     /*
      * Call H5Dread function.
      */
     c_dset_id = (hid_t)*dset_id;
     c_mem_type_id = (hid_t)*mem_type_id;
     c_mem_space_id = (hid_t)*mem_space_id;
     c_file_space_id = (hid_t)*file_space_id;
     ret = H5Dread(c_dset_id, c_mem_type_id, c_mem_space_id, c_file_space_id, c_xfer_prp, buf_c);
     if (ret >=0) {
        for (i = 0; i < n; i++) {
           HDmemcpy(buf, buf_c[i].oid, H5R_OBJ_REF_BUF_SIZE);
           buf = buf + REF_OBJ_BUF_LEN_F;
        }  
     }
     if ( buf_c != NULL ) HDfree(buf_c);
     } 
     if (ret < 0) return ret_value;
     ret_value = 0;
     return ret_value;
}      
/*----------------------------------------------------------------------------
 * Name:        h5dread_ref_obj_c_b
 * Purpose:     Call H5Dread to read a dataset  of object references
 * Inputs:      dset_id - dataset identifier 
 *              mem_type_id - memory datatype identifier
 *              mem_space_id - memory dataspace identifier
 *              file_space_id - memory dataspace identifier
 *              xfer_pr  - identifier of transfer property list
 *              buf      - data buffer to store references to the objects.
 *              n - number of references to be stored.   
 * Returns:     0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Wednesday, May 15, 2002
 * Modifications: This function was added to accomodate h5dread_f subroutine
 *                with the dims parameter being of INTEGER(HSIZE_T_F) size.
 *---------------------------------------------------------------------------*/
int_f
nh5dread_ref_obj_c_b (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, int_f * buf, hsize_t_f *dims)
{
     int ret_value = -1;
     herr_t ret;
     hid_t c_dset_id;
     hid_t c_mem_type_id;
     hid_t c_mem_space_id;
     hid_t c_file_space_id;
     hid_t c_xfer_prp;
     hobj_ref_t *buf_c;
     hsize_t i,n;
     n = (hsize_t)*dims;
     /*
      * Define transfer property
      */
     c_xfer_prp = (hid_t)*xfer_prp;
     
     /*
      * Allocate temporary buffer.
      */
     buf_c = (hobj_ref_t*)HDmalloc(sizeof(hobj_ref_t)*(n));
     if ( buf_c != NULL ) {
     /*
      * Call H5Dread function.
      */
     c_dset_id = (hid_t)*dset_id;
     c_mem_type_id = (hid_t)*mem_type_id;
     c_mem_space_id = (hid_t)*mem_space_id;
     c_file_space_id = (hid_t)*file_space_id;
     ret = H5Dread(c_dset_id, c_mem_type_id, c_mem_space_id, c_file_space_id, c_xfer_prp, buf_c);
     if (ret >=0) {
        for (i = 0; i < n; i++) {
           HDmemcpy(buf, buf_c[i].oid, H5R_OBJ_REF_BUF_SIZE);
           buf = buf + REF_OBJ_BUF_LEN_F;
        }  
     }
     if ( buf_c != NULL ) HDfree(buf_c);
     } 
     if (ret < 0) return ret_value;
     ret_value = 0;
     return ret_value;
}

/*----------------------------------------------------------------------------
 * Name:        h5dread_ref_reg_c
 * Purpose:     Call H5Dread to read a dataset of dataset region references
 * Inputs:      dset_id - dataset identifier 
 *              mem_type_id - memory datatype identifier
 *              mem_space_id - memory dataspace identifier
 *              file_space_id - memory dataspace identifier
 *              xfer_pr  - identifier of transfer property list
 *              buf      - data buffer to store references to the objects.
 *              n - number of references to be stored.   
 * Returns:     0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Monday, July 24, 2000 
 * Modifications:
 *---------------------------------------------------------------------------*/
int_f
nh5dread_ref_reg_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, int_f * buf, int_f *dims)
{
     int ret_value = -1;
     herr_t ret;
     hid_t c_dset_id;
     hid_t c_mem_type_id;
     hid_t c_mem_space_id;
     hid_t c_file_space_id;
     hid_t c_xfer_prp;
     hdset_reg_ref_t *buf_c;
     int i, n;
     n = (int)*dims;
     /*
      * Define transfer property
      */
     c_xfer_prp = (hid_t)*xfer_prp;
     
     /*
      * Allocate temporary buffer.
      */
     buf_c = (hdset_reg_ref_t *)HDmalloc(sizeof(hdset_reg_ref_t)*(n));
     if ( buf_c != NULL ) {
     /*
      * Call H5Dread function.
      */
     c_dset_id = (hid_t)*dset_id;
     c_mem_type_id = (hid_t)*mem_type_id;
     c_mem_space_id = (hid_t)*mem_space_id;
     c_file_space_id = (hid_t)*file_space_id;
     ret = H5Dread(c_dset_id, c_mem_type_id, c_mem_space_id, c_file_space_id, c_xfer_prp, buf_c);
     if (ret >=0) {
        for (i = 0; i < n; i++) {
           HDmemcpy(buf, buf_c[i].heapid, H5R_DSET_REG_REF_BUF_SIZE);
           buf = buf + REF_REG_BUF_LEN_F;
        }  
     }
     if ( buf_c != NULL ) HDfree(buf_c);
     } 
     if (ret < 0) return ret_value;
     ret_value = 0;
     return ret_value;
}      
/*----------------------------------------------------------------------------
 * Name:        h5dread_ref_reg_c_b
 * Purpose:     Call H5Dread to read a dataset of dataset region references
 * Inputs:      dset_id - dataset identifier 
 *              mem_type_id - memory datatype identifier
 *              mem_space_id - memory dataspace identifier
 *              file_space_id - memory dataspace identifier
 *              xfer_pr  - identifier of transfer property list
 *              buf      - data buffer to store references to the objects.
 *              n - number of references to be stored.   
 * Returns:     0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Wednesday, May 15, 2002
 * Modifications: This function was added to accomodate h5dread_f subroutine
 *                with the dims parameter being of INTEGER(HSIZE_T_F) size.
 *---------------------------------------------------------------------------*/
int_f
nh5dread_ref_reg_c_b (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, int_f * buf, hsize_t_f *dims)
{
     int ret_value = -1;
     herr_t ret;
     hid_t c_dset_id;
     hid_t c_mem_type_id;
     hid_t c_mem_space_id;
     hid_t c_file_space_id;
     hid_t c_xfer_prp;
     hdset_reg_ref_t *buf_c;
     hsize_t i, n;
     n = (hsize_t)*dims;
     /*
      * Define transfer property
      */
     c_xfer_prp = (hid_t)*xfer_prp;
     
     /*
      * Allocate temporary buffer.
      */
     buf_c = (hdset_reg_ref_t *)HDmalloc(sizeof(hdset_reg_ref_t)*(n));
     if ( buf_c != NULL ) {
     /*
      * Call H5Dread function.
      */
     c_dset_id = (hid_t)*dset_id;
     c_mem_type_id = (hid_t)*mem_type_id;
     c_mem_space_id = (hid_t)*mem_space_id;
     c_file_space_id = (hid_t)*file_space_id;
     ret = H5Dread(c_dset_id, c_mem_type_id, c_mem_space_id, c_file_space_id, c_xfer_prp, buf_c);
     if (ret >=0) {
        for (i = 0; i < n; i++) {
           HDmemcpy(buf, buf_c[i].heapid, H5R_DSET_REG_REF_BUF_SIZE);
           buf = buf + REF_REG_BUF_LEN_F;
        }  
     }
     if ( buf_c != NULL ) HDfree(buf_c);
     } 
     if (ret < 0) return ret_value;
     ret_value = 0;
     return ret_value;
}      



/*----------------------------------------------------------------------------
 * Name:        h5dclose_c
 * Purpose:     Call H5Dclose to close a dataset 
 * Inputs:      dset_id - identifier of the dataset to be closed
 * Returns:     0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Wednesday, August 4, 1999
 * Modifications:
 *---------------------------------------------------------------------------*/

int_f
nh5dclose_c ( hid_t_f *dset_id )
{
  int ret_value = 0;
  hid_t c_dset_id;
  c_dset_id = (hid_t)*dset_id;
  if ( H5Dclose(c_dset_id) < 0  ) ret_value = -1;
  return ret_value;
}

/*----------------------------------------------------------------------------
 * Name:        h5dget_space_c
 * Purpose:     Call H5Dget_space to obtain dataspace of a dataset 
 * Inputs:      dset_id - identifier of the dataset
 * Outputs:     space_id - identifier of the dataset's dataspace
 * Returns:     0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Thursday, August 19, 1999
 * Modifications:
 *---------------------------------------------------------------------------*/

int_f 
nh5dget_space_c ( hid_t_f *dset_id , hid_t_f *space_id)
{
  int ret_value = -1;
  hid_t c_dset_id;
  hid_t c_space_id;
  
  c_dset_id = (hid_t)*dset_id; 
  c_space_id = H5Dget_space(c_dset_id);
  if(c_space_id < 0 ) return ret_value;
  ret_value = 0;
  *space_id = (hid_t_f)c_space_id;
  return ret_value;
}

/*----------------------------------------------------------------------------
 * Name:        h5dget_type_c
 * Purpose:     Call H5Dget_type to obtain datatype of a dataset 
 * Inputs:      dset_id - identifier of the dataset
 * Outputs:     type_id - identifier of the dataset's datatype
 * Returns:     0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Thursday, August 19, 1999
 * Modifications:
 *---------------------------------------------------------------------------*/

int_f 
nh5dget_type_c ( hid_t_f *dset_id , hid_t_f *type_id)
{
  int ret_value = -1;
  hid_t c_dset_id;
  hid_t c_type_id;
  
  c_dset_id = (hid_t)*dset_id;
  c_type_id = H5Dget_type(c_dset_id);

  if(c_type_id < 0 ) return ret_value;

  *type_id = (hid_t_f)c_type_id;
  ret_value = 0;
  return ret_value;
}

/*----------------------------------------------------------------------------
 * Name:        h5dget_create_plist_c
 * Purpose:     Call H5Dget_create_plist to obtain creation property list
 *              of a dataset 
 * Inputs:      dset_id - identifier of the dataset
 * Outputs:     plist_id - identifier of he dataset creation property list
 * Returns:     0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Thursday, August 19, 1999
 * Modifications:
 *---------------------------------------------------------------------------*/

int_f 
nh5dget_create_plist_c ( hid_t_f *dset_id , hid_t_f *plist_id)
{
  int ret_value = -1;
  hid_t c_dset_id;
  hid_t c_plist_id;
  
  c_dset_id = (hid_t)*dset_id;
  c_plist_id = H5Dget_create_plist(c_dset_id);

  if(c_plist_id < 0 ) return ret_value;

  ret_value = 0;
  *plist_id = (hid_t_f)c_plist_id;
  return ret_value;
}


/*----------------------------------------------------------------------------
 * Name:        h5dextend_c
 * Purpose:     Call H5Dextend to extend dataset with unlimited dimensions 
 * Inputs:      dset_id - identifier of the dataset 
 * Outputs:     dims - array with the dimension sizes 
 * Returns:     0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Thursday, August 19, 1999
 * Modifications:
 *---------------------------------------------------------------------------*/

int_f 
nh5dextend_c ( hid_t_f *dset_id , hsize_t_f *dims)
{
  int ret_value = -1;
  hsize_t *c_dims;
  int status;
  int rank;
  int i;
  hid_t c_dset_id;
  hid_t c_space_id;

  c_dset_id = (hid_t)*dset_id;
  c_space_id = H5Dget_space(c_dset_id);
  if (c_space_id < 0) return ret_value;

  rank = H5Sget_simple_extent_ndims(c_space_id);
  if (rank < 0) return ret_value;

  c_dims = malloc(sizeof(hsize_t)*rank);
  if (!c_dims) return ret_value;

  /*
   * Reverse dimensions due to C-FORTRAN storage order.
   */
  for (i=0; i < rank; i++) 
      c_dims[i] = dims[rank - i - 1];
  
  status = H5Dextend(c_dset_id, c_dims);

  if ( status >= 0  ) ret_value = 0;
  HDfree(c_dims);
  return ret_value;
}

/*----------------------------------------------------------------------------
 * Name:        nh5dget_storage_size_c
 * Purpose:     Call H5Dget_storage_size to return the amount of storage 
 *              required for a dataset
 * Inputs:      dset_id - identifier of the dataset 
 * Outputs:     size    - the amount of storage required for a dataset
 * Returns:     0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Tuesday, October 22, 2002
 * Modifications:
 *---------------------------------------------------------------------------*/

int_f 
nh5dget_storage_size_c ( hid_t_f *dset_id , hsize_t_f *size)
{
  int ret_value = -1;
  hsize_t c_size;
  hid_t c_dset_id;

  c_dset_id = (hid_t)*dset_id;
  c_size = H5Dget_storage_size(c_dset_id);
  if (c_size == 0) return ret_value;
  *size = (hsize_t_f)c_size;
  ret_value = 0;
  return ret_value;
}

/*----------------------------------------------------------------------------
 * Name:        nh5dvlen_get_max_len_c
 * Purpose:     Get the maximum size of the VL dataset element
 * Inputs:      dset_id - identifier of the dataset 
 *              type_id - datatype identifier
 *              space_id - dataspace identifier 
 * Outputs:     len      - maximum length of the VL dataset element
 * Returns:     0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Tuesday, October 22, 2002
 * Modifications:
 *---------------------------------------------------------------------------*/

int_f 
nh5dvlen_get_max_len_c ( hid_t_f *dset_id ,  hid_t_f *type_id, hid_t_f *space_id, size_t_f *len)
{
  int ret_value = -1;
  size_t c_len;
  hid_t c_dset_id;
  hid_t c_type_id;
  hid_t c_space_id;
  hvl_t *c_buf;
  int i,j;
  hssize_t num_elem;

  c_dset_id = (hid_t)*dset_id;
  c_type_id = (hid_t)*type_id;
  c_space_id = (hid_t)*space_id;

  num_elem = H5Sget_select_npoints(c_space_id);  
  if( num_elem < 0) return ret_value;

  c_buf = (hvl_t *)malloc(sizeof(hvl_t)*num_elem); 
  if (c_buf == NULL) return ret_value;
  if(H5Dread(c_dset_id, c_type_id, H5S_ALL, c_space_id, H5P_DEFAULT, c_buf)) goto DONE;
  
  c_len = 0;
  for (i=0; i < num_elem; i++) c_len = H5_MAX(c_len, c_buf[i].len);  
  *len = (size_t_f)c_len;
  ret_value = 0;
  
DONE:

  H5Dvlen_reclaim(c_type_id, c_space_id, H5P_DEFAULT, c_buf);
  free(c_buf);
  return ret_value;
}
/*----------------------------------------------------------------------------
 * Name:        nh5dwrite_vl_integer_c
 * Purpose:     Write variable length dataset 
 * Inputs:      dset_id - identifier of the dataset 
 *              mem_type_id - datatype identifier
 *              mem_space_id - dataspace identifier 
 *              file_space_id - file dataspace identifier 
 *              xfer          - file transfer property
 *              buf           - data buffer
 *              dims          - one-demnsional array of size 2
 *                              dims[0] = MAXLENGTH
 *                              dims[1] = number of elements of VL type
 *              len           - array element lenghts
 * Returns:     0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Wednesday, October 23, 2002
 * Modifications:
 *---------------------------------------------------------------------------*/

int_f 
nh5dwrite_vl_integer_c ( hid_t_f *dset_id ,  hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, int_f *buf, hsize_t_f *dims, size_t_f *len)
{
  int ret_value = -1;
  hid_t c_dset_id;
  hid_t c_mem_type_id;
  hid_t c_mem_space_id;
  hid_t c_file_space_id;
  hid_t c_xfer_prp;
  herr_t status;
  int *tmp;
  size_t max_len;

  hvl_t *c_buf;
  int i, j;
  hsize_t num_elem;
  
  max_len = (size_t)dims[0];
  num_elem = dims[1];

  c_dset_id       = (hid_t)*dset_id;
  c_mem_type_id   = (hid_t)*mem_type_id;
  c_mem_space_id  = (hid_t)*mem_space_id;
  c_file_space_id = (hid_t)*file_space_id;
  c_xfer_prp      = (hid_t)*xfer_prp;

  c_buf = (hvl_t *)malloc(num_elem * sizeof(hvl_t)); 
  if (c_buf == NULL) return ret_value;
  tmp = buf; 
  for (i=0; i < num_elem; i++) {
       c_buf[i].len = (size_t)len[i];  
       c_buf[i].p   = tmp;
       tmp = tmp + max_len;
 } 
  /*
   * Call H5Dwrite function.
   */
   status = H5Dwrite(c_dset_id, c_mem_type_id, c_mem_space_id, c_file_space_id, c_xfer_prp, c_buf);

  if( status < 0) goto DONE;
  ret_value = 0;
DONE:
  free(c_buf);
  return ret_value;
}

/*----------------------------------------------------------------------------
 * Name:        nh5dread_vl_integer_c
 * Purpose:     Read variable length dataset 
 * Inputs:      dset_id - identifier of the dataset 
 *              mem_type_id - datatype identifier
 *              mem_space_id - dataspace identifier 
 *              file_space_id - file dataspace identifier 
 *              xfer          - file transfer property
 *              dims          - one-demnsional array of size 2
 *                              dims[0] = MAXLENGTH
 *                              dims[1] = number of elements of VL type
 * Outputs:     buf           - data buffer
 *              len           - array element lenghts
 * Returns:     0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Wednesday, October 24, 2002
 * Modifications:
 *---------------------------------------------------------------------------*/

int_f 
nh5dread_vl_integer_c ( hid_t_f *dset_id ,  hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, int_f *buf, hsize_t_f *dims, size_t_f *len)
{
  int ret_value = -1;
  hid_t c_dset_id;
  hid_t c_mem_type_id;
  hid_t c_mem_space_id;
  hid_t c_file_space_id;
  hid_t c_xfer_prp;
  herr_t status;
  size_t max_len;

  hvl_t *c_buf;
  size_t i;
  hssize_t num_elem;
  
  c_dset_id       = (hid_t)*dset_id;
  c_mem_type_id   = (hid_t)*mem_type_id;
  c_mem_space_id  = (hid_t)*mem_space_id;
  c_file_space_id = (hid_t)*file_space_id;
  c_xfer_prp      = (hid_t)*xfer_prp;

  max_len = (size_t)dims[0];
  num_elem = H5Sget_select_npoints(c_mem_space_id);  
  if(num_elem != dims[1]) return ret_value;

  c_buf = (hvl_t *)malloc(num_elem * sizeof(hvl_t)); 
  if (c_buf == NULL) return ret_value;
  /*
   * Call H5Dread function.
   */
   status = H5Dread(c_dset_id, c_mem_type_id, c_mem_space_id, c_file_space_id, c_xfer_prp, c_buf);
 if ( status >=0 ) {
  for (i=0; i < num_elem; i++) {
       len[i] = (size_t_f)c_buf[i].len;  
       memcpy(&buf[i*max_len], c_buf[i].p, c_buf[i].len*sizeof(int));
  }
 } 
  ret_value = num_elem;
DONE:
  H5Dvlen_reclaim(c_mem_type_id, c_mem_space_id, H5P_DEFAULT, c_buf);
  free(c_buf);
  return ret_value;
}

/*----------------------------------------------------------------------------
 * Name:        nh5dwrite_vl_string_c
 * Purpose:     Write variable length strings from Fortran program
 * Inputs:      dset_id - identifier of the dataset 
 *              mem_type_id - datatype identifier
 *              mem_space_id - dataspace identifier 
 *              file_space_id - file dataspace identifier 
 *              xfer          - file transfer property
 *              buf           - data buffer
 *              dims          - one-demnsional array of size 2 
 *                              dims[0] = number of strings of size max_len
 *              len           - array of strings lengths
 * Returns:     0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Monday, October 28, 2002
 * Modifications:
 *---------------------------------------------------------------------------*/

int_f 
nh5dwrite_vl_string_c( hid_t_f *dset_id ,  hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, _fcd buf, hsize_t_f *dims, size_t_f *len)
{
  int ret_value = -1;
  hid_t c_dset_id;
  hid_t c_mem_type_id;
  hid_t c_mem_space_id;
  hid_t c_file_space_id;
  hid_t c_xfer_prp;
  herr_t status;
  char *tmp, *tmp_p;
  size_t max_len;
  size_t buf_len;

  char **c_buf;
  int i, j;
  hsize_t num_elem;
  
  max_len = (size_t)dims[0];
  num_elem = dims[1];

  c_dset_id       = (hid_t)*dset_id;
  c_mem_type_id   = (hid_t)*mem_type_id;
  c_mem_space_id  = (hid_t)*mem_space_id;
  c_file_space_id = (hid_t)*file_space_id;
  c_xfer_prp      = (hid_t)*xfer_prp;

  /*
   * Allocate arra of character pointers
   */
  c_buf = (char **)malloc(num_elem * sizeof(char *)); 
  if (c_buf == NULL) return ret_value;

  /* Copy data to long C string */
  tmp = (char *)HD5f2cstring(buf, max_len*num_elem);
  if (tmp == NULL) { free(c_buf);
                     return ret_value;
                   }
  /*
   * Move data from temorary buffer
   */
   tmp_p = tmp;
   for (i=0; i < num_elem; i++) {
        c_buf[i] = (char *) malloc(len[i]+1);
        memcpy(c_buf[i], tmp_p, len[i]);
        c_buf[i][len[i]] = '\0'; 
        tmp_p = tmp_p + max_len;
   }

  /*
   * Call H5Dwrite function.
   */
   status = H5Dwrite(c_dset_id, c_mem_type_id, c_mem_space_id, c_file_space_id, c_xfer_prp, c_buf);

  if( status < 0) goto DONE;
  ret_value = 0;
DONE:
  H5Dvlen_reclaim(c_mem_type_id, c_mem_space_id, H5P_DEFAULT, c_buf);
  free(c_buf);
  free(tmp);
  return ret_value;
}
/*----------------------------------------------------------------------------
 * Name:        nh5dread_vl_string_c
 * Purpose:     Read variable length strings from Fortran program
 * Inputs:      dset_id - identifier of the dataset 
 *              mem_type_id - datatype identifier
 *              mem_space_id - dataspace identifier 
 *              file_space_id - file dataspace identifier 
 *              xfer          - file transfer property
 *              dims          - one-demnsional array of size 2 
 *                              dims[0] = number of strings of size max_len
 * Output:      buf           - data buffer
 *              len           - array of strings lengths
 * Returns:     0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Friday, November 1, 2002
 * Modifications:
 *---------------------------------------------------------------------------*/

int_f 
nh5dread_vl_string_c( hid_t_f *dset_id ,  hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, _fcd buf, hsize_t_f *dims, size_t_f *len)
{
  int ret_value = -1;
  hid_t c_dset_id;
  hid_t c_mem_type_id;
  hid_t c_mem_space_id;
  hid_t c_file_space_id;
  hid_t c_xfer_prp;
  herr_t status;
  char *tmp, *tmp_p;
  size_t max_len;
  size_t buf_len;

  char **c_buf;
  int i, j;
  hsize_t num_elem;
  
  max_len = (size_t)dims[0];
  num_elem = dims[1];

  c_dset_id       = (hid_t)*dset_id;
  c_mem_type_id   = (hid_t)*mem_type_id;
  c_mem_space_id  = (hid_t)*mem_space_id;
  c_file_space_id = (hid_t)*file_space_id;
  c_xfer_prp      = (hid_t)*xfer_prp;

  /*
   * Allocate array of character pointers
   */
  c_buf = (char **)malloc(num_elem * sizeof(char *)); 
  if (c_buf == NULL) return ret_value;

  /*
   * Call H5Dread function.
   */
   status = H5Dread(c_dset_id, c_mem_type_id, c_mem_space_id, c_file_space_id, c_xfer_prp, c_buf);
   if (status < 0) { free(c_buf);
                     return ret_value;
                   }
  /* Copy data to long C string */
  tmp = (char *)malloc(max_len*num_elem +1);
  tmp_p = tmp;
  for (i=0; i<max_len*num_elem; i++) tmp[i] = ' ';
  tmp[max_len*num_elem] = '\0';
  for (i=0; i < num_elem; i++) {
        memcpy(tmp_p, c_buf[i], strlen(c_buf[i]));
        len[i] = (size_t_f)strlen(c_buf[i]);
        tmp_p = tmp_p + max_len;
  }
  HD5packFstring(tmp, _fcdtocp(buf), max_len*num_elem);  
  ret_value = 0;
DONE:
  H5Dvlen_reclaim(c_mem_type_id, c_mem_space_id, H5P_DEFAULT, c_buf);
  free(c_buf);
  free(tmp);
  return ret_value;
}

/*----------------------------------------------------------------------------
 * Name:        nh5dwrite_vl_real_c
 * Purpose:     Write variable length dataset 
 * Inputs:      dset_id - identifier of the dataset 
 *              mem_type_id - datatype identifier
 *              mem_space_id - dataspace identifier 
 *              file_space_id - file dataspace identifier 
 *              xfer          - file transfer property
 *              buf           - data buffer
 *              dims          - one-demnsional array of size 2
 *                              dims[0] = MAXLENGTH
 *                              dims[1] = number of elements of VL type
 *              len           - array element lenghts
 * Returns:     0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Monday, November 11, 2002
 * Modifications:
 *---------------------------------------------------------------------------*/

int_f 
nh5dwrite_vl_real_c ( hid_t_f *dset_id ,  hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, real_f *buf, hsize_t_f *dims, size_t_f *len)
{
  int ret_value = -1;
  hid_t c_dset_id;
  hid_t c_mem_type_id;
  hid_t c_mem_space_id;
  hid_t c_file_space_id;
  hid_t c_xfer_prp;
  herr_t status;
  float *tmp;
  size_t max_len;

  hvl_t *c_buf;
  int i, j;
  hsize_t num_elem;
  
  max_len = (size_t)dims[0];
  num_elem = dims[1];

  c_dset_id       = (hid_t)*dset_id;
  c_mem_type_id   = (hid_t)*mem_type_id;
  c_mem_space_id  = (hid_t)*mem_space_id;
  c_file_space_id = (hid_t)*file_space_id;
  c_xfer_prp      = (hid_t)*xfer_prp;

  c_buf = (hvl_t *)malloc(num_elem * sizeof(hvl_t)); 
  if (c_buf == NULL) return ret_value;
  tmp = buf; 
  for (i=0; i < num_elem; i++) {
       c_buf[i].len = (size_t)len[i];  
       c_buf[i].p   = tmp;
       tmp = tmp + max_len;
 } 
  /*
   * Call H5Dwrite function.
   */
   status = H5Dwrite(c_dset_id, c_mem_type_id, c_mem_space_id, c_file_space_id, c_xfer_prp, c_buf);

  if( status < 0) goto DONE;
  ret_value = 0;
DONE:
  free(c_buf);
  return ret_value;
}

/*----------------------------------------------------------------------------
 * Name:        nh5dread_vl_real_c
 * Purpose:     Read variable length dataset 
 * Inputs:      dset_id - identifier of the dataset 
 *              mem_type_id - datatype identifier
 *              mem_space_id - dataspace identifier 
 *              file_space_id - file dataspace identifier 
 *              xfer          - file transfer property
 *              dims          - one-demnsional array of size 2
 *                              dims[0] = MAXLENGTH
 *                              dims[1] = number of elements of VL type
 * Outputs:     buf           - data buffer
 *              len           - array element lenghts
 * Returns:     0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Monday, November 11, 2002
 * Modifications:
 *---------------------------------------------------------------------------*/

int_f 
nh5dread_vl_real_c ( hid_t_f *dset_id ,  hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, real_f *buf, hsize_t_f *dims, size_t_f *len)
{
  int ret_value = -1;
  hid_t c_dset_id;
  hid_t c_mem_type_id;
  hid_t c_mem_space_id;
  hid_t c_file_space_id;
  hid_t c_xfer_prp;
  herr_t status;
  size_t max_len;

  hvl_t *c_buf;
  size_t i;
  hssize_t num_elem;
  
  c_dset_id       = (hid_t)*dset_id;
  c_mem_type_id   = (hid_t)*mem_type_id;
  c_mem_space_id  = (hid_t)*mem_space_id;
  c_file_space_id = (hid_t)*file_space_id;
  c_xfer_prp      = (hid_t)*xfer_prp;

  max_len = (size_t)dims[0];
  num_elem = H5Sget_select_npoints(c_mem_space_id);  
  if(num_elem != dims[1]) return ret_value;

  c_buf = (hvl_t *)malloc(num_elem * sizeof(hvl_t)); 
  if (c_buf == NULL) return ret_value;
  /*
   * Call H5Dread function.
   */
   status = H5Dread(c_dset_id, c_mem_type_id, c_mem_space_id, c_file_space_id, c_xfer_prp, c_buf);
 if ( status >=0 ) {
  for (i=0; i < num_elem; i++) {
       len[i] = (size_t_f)c_buf[i].len;  
       memcpy(&buf[i*max_len], c_buf[i].p, c_buf[i].len*sizeof(float));
  }
 } 
  ret_value = num_elem;
DONE:
  H5Dvlen_reclaim(c_mem_type_id, c_mem_space_id, H5P_DEFAULT, c_buf);
  free(c_buf);
  return ret_value;
}

/*----------------------------------------------------------------------------
 * Name:        h5dfillc_c
 * Purpose:     Call h5fill_c to fill memory buffer with a fill value
 * Inputs:      fill_value - fill value
 *              fill_type_id - fill value datatype identifier
 *              space_id - memory space selection identifier
 *              buf      - memory buffer to fill
 *              mem_type_id - memory buffer dtatype identifier
 * Returns:     0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Wednesday, March 12, 2003
 * Modifications:
 *---------------------------------------------------------------------------*/
int_f
nh5dfillc_c (_fcd fill_value, hid_t_f *fill_type_id, hid_t_f *space_id, _fcd buf, hid_t_f *mem_type_id)
{
     int ret_value = -1;
     
     /*
      * Call h5dfill_c  function.
      */
     ret_value = nh5dfill_c(_fcdtocp(fill_value), fill_type_id, space_id, _fcdtocp(buf), mem_type_id);

     return ret_value;
}
/*----------------------------------------------------------------------------
 * Name:        h5dfill_c
 * Purpose:     Call H5Dfill to fill memory buffer with a fill value
 * Inputs:      fill_value - fill value
 *              fill_type_id - fill value datatype identifier
 *              space_id - memory space selection identifier
 *              buf      - memory buffer to fill
 *              mem_type_id - memory buffer dtatype identifier
 * Returns:     0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Wednesday, March 12, 2003
 * Modifications:
 *---------------------------------------------------------------------------*/
int_f
nh5dfill_c (void * fill_value, hid_t_f *fill_type_id, hid_t_f *space_id, void * buf, hid_t_f *mem_type_id)

{
     int ret_value = -1;
     herr_t ret;
     hid_t c_fill_type_id;
     hid_t c_mem_type_id;
     hid_t c_space_id;

     c_fill_type_id = (hid_t)*fill_type_id;
     c_mem_type_id = (hid_t)*mem_type_id;
     c_space_id = (hid_t)*space_id;
     
     /*
      * Call H5Dfill function.
      */
     ret = H5Dfill(fill_value, c_fill_type_id, buf, c_mem_type_id, c_space_id);

     if (ret < 0) return ret_value;
     ret_value = 0;
     return ret_value;
}      

/*----------------------------------------------------------------------------
 * Name:        h5dget_space_status_c
 * Purpose:     Call H5Dget_space_status to request dataspace allocation status
 * Inputs:      dset_id - dataset identifier
 * Outputs:     flag - status flag
 * Returns:     0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Wednesday, March 12, 2003
 * Modifications:
 *---------------------------------------------------------------------------*/
int_f
nh5dget_space_status_c ( hid_t_f *dset_id, int_f *flag)

{
     int ret_value = -1;
     herr_t ret;
     hid_t c_dset_id;
     H5D_space_status_t c_flag;

     c_dset_id = (hid_t)*dset_id;
     
     /*
      * Call H5Dget_space_status
      */
     ret = H5Dget_space_status(c_dset_id, &c_flag);

     if (ret < 0) return ret_value;
     *flag = (int_f)c_flag;
     ret_value = 0;
     return ret_value;
}      

