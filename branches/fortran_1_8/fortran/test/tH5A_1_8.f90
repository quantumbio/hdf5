! * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
!   Copyright by The HDF Group.                                               *
!   Copyright by the Board of Trustees of the University of Illinois.         *
!   All rights reserved.                                                      *
!                                                                             *
!   This file is part of HDF5.  The full HDF5 copyright notice, including     *
!   terms governing use, modification, and redistribution, is contained in    *
!   the files COPYING and Copyright.html.  COPYING can be found at the root   *
!   of the source code distribution tree; Copyright.html can be found at the  *
!   root level of an installed copy of the electronic HDF5 document set and   *
!   is linked from the top-level documents page.  It can also be found at     *
!   http://hdfgroup.org/HDF5/doc/Copyright.html.  If you do not have          *
!   access to either file, you may request a copy from help@hdfgroup.org.     *
! * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
!
    SUBROUTINE attribute_test_1_8(cleanup, total_error) 

!   This subroutine tests following functionalities: 
!   h5acreate_f,  h5awrite_f, h5aclose_f,h5aread_f, h5aopen_name_f,
!   h5aget_name_f,h5aget_space_f, h5aget_type_f,
! 

     USE HDF5 ! This module contains all necessary modules 
        
     IMPLICIT NONE
     LOGICAL, INTENT(IN)  :: cleanup
     INTEGER, INTENT(OUT) :: total_error 

     CHARACTER(LEN=5), PARAMETER :: filename = "atest"    !File name
     CHARACTER(LEN=80) :: fix_filename 
     CHARACTER(LEN=9), PARAMETER :: dsetname = "atestdset"        !Dataset name
     CHARACTER(LEN=11), PARAMETER :: aname = "attr_string"   !String Attribute name
     CHARACTER(LEN=14), PARAMETER :: aname2 = "attr_character"!Character Attribute name
     CHARACTER(LEN=11), PARAMETER :: aname3 = "attr_double"   !DOuble Attribute name
     CHARACTER(LEN=9), PARAMETER :: aname4 = "attr_real"      !Real Attribute name
     CHARACTER(LEN=12), PARAMETER :: aname5 = "attr_integer"  !Integer Attribute name
     CHARACTER(LEN=9), PARAMETER :: aname6 = "attr_null"     !Null Attribute name
    
     !
     !data space rank and dimensions
     !
     INTEGER, PARAMETER :: RANK = 2
     INTEGER, PARAMETER :: NX = 4
     INTEGER, PARAMETER :: NY = 5



     INTEGER(HID_T) :: file_id       ! File identifier 
     INTEGER(HID_T) :: dset_id       ! Dataset identifier 
     INTEGER(HID_T) :: dataspace     ! Dataspace identifier for dataset 

     INTEGER(HID_T) :: attr_id        !String Attribute identifier 
     INTEGER(HID_T) :: attr2_id       !Character Attribute identifier 
     INTEGER(HID_T) :: attr3_id       !Double Attribute identifier 
     INTEGER(HID_T) :: attr4_id       !Real Attribute identifier 
     INTEGER(HID_T) :: attr5_id       !Integer Attribute identifier 
     INTEGER(HID_T) :: attr6_id       !Null Attribute identifier 
     INTEGER(HID_T) :: aspace_id      !String Attribute Dataspace identifier 
     INTEGER(HID_T) :: aspace2_id     !Character Attribute Dataspace identifier 
     INTEGER(HID_T) :: aspace3_id     !Double Attribute Dataspace identifier 
     INTEGER(HID_T) :: aspace4_id     !Real Attribute Dataspace identifier 
     INTEGER(HID_T) :: aspace5_id     !Integer Attribute Dataspace identifier 
     INTEGER(HID_T) :: aspace6_id     !Null Attribute Dataspace identifier 
     INTEGER(HID_T) :: atype_id       !String Attribute Datatype identifier 
     INTEGER(HID_T) :: atype2_id      !Character Attribute Datatype identifier 
     INTEGER(HID_T) :: atype3_id      !Double Attribute Datatype identifier 
     INTEGER(HID_T) :: atype4_id      !Real Attribute Datatype identifier 
     INTEGER(HID_T) :: atype5_id      !Integer Attribute Datatype identifier 
     INTEGER(HSIZE_T), DIMENSION(1) :: adims = (/2/) ! Attribute dimension
     INTEGER(HSIZE_T), DIMENSION(1) :: adims2 = (/1/) ! Attribute dimension
     INTEGER     ::   arank = 1                      ! Attribure rank
     INTEGER(SIZE_T) :: attrlen    ! Length of the attribute string

     INTEGER(HID_T) :: attr_space     !Returned String Attribute Space identifier 
     INTEGER(HID_T) :: attr2_space    !Returned other Attribute Space identifier 
     INTEGER(HID_T) :: attr_type      !Returned Attribute Datatype identifier
     INTEGER(HID_T) :: attr2_type      !Returned CHARACTER Attribute Datatype identifier
     INTEGER(HID_T) :: attr3_type      !Returned DOUBLE Attribute Datatype identifier
     INTEGER(HID_T) :: attr4_type      !Returned REAL Attribute Datatype identifier
     INTEGER(HID_T) :: attr5_type      !Returned INTEGER Attribute Datatype identifier
     INTEGER(HID_T) :: attr6_type      !Returned NULL Attribute Datatype identifier
     INTEGER        :: num_attrs      !number of attributes  
     INTEGER(HSIZE_T) :: attr_storage   ! attributes storage requirements .MSB.
     CHARACTER(LEN=256) :: attr_name    !buffer to put attr_name
     INTEGER(SIZE_T)    ::  name_size = 80 !attribute name length

     CHARACTER(LEN=35), DIMENSION(2) ::  attr_data  ! String attribute data
     CHARACTER(LEN=35), DIMENSION(2) ::  aread_data ! Buffer to put read back 
                                               ! string attr data
     CHARACTER ::  attr_character_data = 'A'
     DOUBLE PRECISION,  DIMENSION(1) ::  attr_double_data = 3.459
     REAL,         DIMENSION(1) ::  attr_real_data = 4.0
     INTEGER,      DIMENSION(1) ::  attr_integer_data = 5
     INTEGER(HSIZE_T), DIMENSION(7) :: data_dims

     
     CHARACTER :: aread_character_data ! variable to put read back Character attr data
     INTEGER, DIMENSION(1)  :: aread_integer_data ! variable to put read back integer attr data
     INTEGER, DIMENSION(1)  :: aread_null_data = 7 ! variable to put read back null attr data
     DOUBLE PRECISION, DIMENSION(1)   :: aread_double_data ! variable to put read back double attr data
     REAL, DIMENSION(1)  :: aread_real_data ! variable to put read back real attr data

     !
     !general purpose integer 
     !         
     INTEGER     ::   i, j
     INTEGER     ::   error ! Error flag
     
     ! 
     !The dimensions for the dataset.
     !
     INTEGER(HSIZE_T), DIMENSION(2) :: dims = (/NX,NY/)

     !
     !data buffers 
     !         
     INTEGER, DIMENSION(NX,NY) :: data_in, data_out

     ! NEW STARTS HERE
     INTEGER(HID_T) :: fapl = -1, fapl2 = -1
     INTEGER(HID_T) :: fcpl = -1, fcpl2 = -1
     INTEGER(HID_T) :: my_fapl, my_fcpl
     LOGICAL, DIMENSION(1:2) :: new_format = (/.TRUE.,.FALSE./)
     LOGICAL, DIMENSION(1:2) :: use_shared = (/.TRUE.,.FALSE./)
     INTEGER :: ret 


! ********************
! test_attr equivelent
! ********************

     WRITE(*,*) "Testing Attributes"

     CALL H5Pcreate_f(H5P_FILE_ACCESS_F,fapl,error)
     CALL check("h5Pcreate_f",error,total_error)
     CALL h5pcopy_f(fapl, fapl2, error)
     CALL check("h5pcopy_f",error,total_error)
!!$
!!$     ret = H5Pset_format_bounds(fapl2, H5F_FORMAT_LATEST, H5F_FORMAT_LATEST)
!!$     CALL CHECK(ret, FAIL, "H5Pset_format_bounds")


     CALL H5Pcreate_f(H5P_FILE_CREATE_F,fcpl,error)
     CALL check("h5Pcreate_f",error,total_error)

     CALL h5pcopy_f(fcpl, fcpl2, error)
     CALL check("h5pcopy_f",error,total_error)

!!$     ret = H5Pset_shared_mesg_nindexes(fcpl2, (unsigned)1)
!!$     CALL CHECK_I(ret, "H5Pset_shared_mesg_nindexes")
!!$     ret = H5Pset_shared_mesg_index(fcpl2, (unsigned)0, H5O_SHMESG_ATTR_FLAG, (unsigned)1)
!!$     CALL CHECK_I(ret, "H5Pset_shared_mesg_index")
     DO i = 1, 1 ! 2
        IF (new_format(i)) THEN
           WRITE(*,*) "testing with new file format"
           my_fapl = fapl2
        ELSE
           WRITE(*,*) "testing with old file format"
           my_fapl = fapl
        END IF
        CALL test_attr_basic_write(my_fapl, total_error)
!!$        CALL test_attr_basic_read(my_fapl)
!!$        CALL test_attr_flush(my_fapl)
!!$        CALL test_attr_plist(my_fapl)
!!$        CALL test_attr_compound_write(my_fapl)
!!$        CALL test_attr_compound_read(my_fapl)
!!$        CALL test_attr_scalar_write(my_fapl)
!!$        CALL test_attr_scalar_read(my_fapl)
!!$        CALL test_attr_mult_write(my_fapl)
!!$        CALL test_attr_mult_read(my_fapl)
!!$        CALL test_attr_iterate(my_fapl)
!!$        CALL test_attr_delete(my_fapl)
!!$        CALL test_attr_dtype_shared(my_fapl)
        IF(new_format(i)) THEN
           DO j = 1, 1 ! 2
              IF (use_shared(j)) THEN
                 WRITE(*,*) "testing with shared attributes"
                 my_fcpl = fcpl2
              ELSE
                 WRITE(*,*) "testing without shared attributes"
                 my_fcpl = fcpl
              END IF
!!$              CALL test_attr_dense_create(my_fcpl, my_fapl)
              CALL test_attr_dense_open(my_fcpl, my_fapl, total_error)
!!$              CALL test_attr_dense_delete(my_fcpl, my_fapl)
!!$              CALL test_attr_dense_rename(my_fcpl, my_fapl)
!!$              CALL test_attr_dense_unlink(my_fcpl, my_fapl)
!!$              CALL test_attr_dense_limits(my_fcpl, my_fapl)
!!$              CALL test_attr_big(my_fcpl, my_fapl)
              CALL test_attr_null_space(my_fcpl, my_fapl, total_error)
!!$              CALL test_attr_deprec(fcpl, my_fapl)
              CALL test_attr_many(new_format(i), my_fcpl, my_fapl, total_error)
              CALL test_attr_corder_create_basic(my_fcpl, my_fapl, total_error)
              CALL test_attr_corder_create_compact(my_fcpl, my_fapl, total_error)
!!$              CALL test_attr_corder_create_dense(my_fcpl, my_fapl)
!!$              CALL test_attr_corder_create_reopen(my_fcpl, my_fapl)
!!$              CALL test_attr_corder_transition(my_fcpl, my_fapl)
!!$              CALL test_attr_corder_delete(my_fcpl, my_fapl)
              CALL test_attr_info_by_idx(new_format, my_fcpl, my_fapl, total_error)
              CALL test_attr_delete_by_idx(new_format, my_fcpl, my_fapl, total_error)
!!$              CALL test_attr_iterate2(new_format, my_fcpl, my_fapl)
!!$              CALL test_attr_open_by_idx(new_format, my_fcpl, my_fapl)
!!$              CALL test_attr_open_by_name(new_format, my_fcpl, my_fapl)
              CALL test_attr_create_by_name(new_format(i), my_fcpl, my_fapl, total_error)
             ! /* More complex tests with both "new format" and "shared" attributes */
              IF( use_shared(j) ) THEN
!!$                 CALL test_attr_shared_write(my_fcpl, my_fapl)
                 CALL test_attr_shared_rename(my_fcpl, my_fapl, total_error)
                 CALL test_attr_shared_delete(my_fcpl, my_fapl, total_error)
!!$                 CALL test_attr_shared_unlink(my_fcpl, my_fapl)
              END IF
!!$              CALL test_attr_bug1(my_fcpl, my_fapl)
           END DO
!!$        ELSE
!!$           CALL test_attr_big(fcpl, my_fapl)
!!$           CALL test_attr_null_space(fcpl, my_fapl)
!!$           CALL test_attr_deprec(fcpl, my_fapl)
!!$           CALL test_attr_many(new_format, fcpl, my_fapl)
!!$           CALL test_attr_info_by_idx(new_format, fcpl, my_fapl)
!!$           CALL test_attr_delete_by_idx(new_format, fcpl, my_fapl)
!!$           CALL test_attr_iterate2(new_format, fcpl, my_fapl)
!!$           CALL test_attr_open_by_idx(new_format, fcpl, my_fapl)
!!$           CALL test_attr_open_by_name(new_format, fcpl, my_fapl)
!!$           CALL test_attr_create_by_name(new_format, fcpl, my_fapl)
!!$           CALL test_attr_bug1(fcpl, my_fapl)
        END IF
     END DO
!!$     ret = H5Pclose(fcpl)
!!$     CALL CHECK(ret, FAIL, "H5Pclose")
!!$     ret = H5Pclose(fcpl2)
!!$     CALL CHECK(ret, FAIL, "H5Pclose")
!!$     ret = H5Pclose(fapl)
!!$     CALL CHECK(ret, FAIL, "H5Pclose")
!!$     ret = H5Pclose(fapl2)
!!$     CALL CHECK(ret, FAIL, "H5Pclose") 

     RETURN
   END SUBROUTINE attribute_test_1_8

SUBROUTINE test_attr_corder_create_compact(fcpl,fapl, total_error)

! Needed for get_info_by_name

  USE HDF5 ! This module contains all necessary modules

  IMPLICIT NONE
! - - - arg types - - -

  INTEGER(HID_T), INTENT(IN) :: fcpl
  INTEGER(HID_T), INTENT(IN) :: fapl

  CHARACTER(LEN=8) :: FileName = "tattr.h5"
  INTEGER(HID_T) :: fid
  INTEGER(HID_T) :: dcpl
  INTEGER(HID_T) :: sid

  INTEGER :: error
  INTEGER, INTENT(INOUT) :: total_error

  CHARACTER(LEN=8) :: DSET1_NAME = "Dataset1"
  CHARACTER(LEN=8) :: DSET2_NAME = "Dataset2"
  CHARACTER(LEN=8) :: DSET3_NAME = "Dataset3"
  INTEGER, PARAMETER :: NUM_DSETS = 3

  INTEGER :: curr_dset
!!$
!!$! - - - local declarations - - -
!!$
!!$  INTEGER :: max_compact,min_dense,curr_dset,u
!!$  CHARACTER (LEN=NAME_BUF_SIZE) :: attrname
!!$
  INTEGER(HID_T) :: dset1, dset2, dset3
  INTEGER(HID_T) :: my_dataset

  INTEGER :: u
  
  INTEGER :: max_compact ! Maximum # of links to store in group compactly
  INTEGER :: min_dense   ! Minimum # of links to store in group "densely"

  CHARACTER(LEN=7) :: attrname
  CHARACTER(LEN=2) :: chr2
  INTEGER(HID_T) :: attr        !String Attribute identifier 
  INTEGER(HSIZE_T), DIMENSION(7) :: data_dims

  LOGICAL :: f_corder_valid ! Indicates whether the the creation order data is valid for this attribute 
  INTEGER :: corder ! Is a positive integer containing the creation order of the attribute
  INTEGER :: cset ! Indicates the character set used for the attribute’s name
  INTEGER(HSIZE_T) :: data_size   ! indicates the size, in the number of characters

  data_dims = 0

!!$  INTEGER :: sid
!!$  INTEGER :: attr
!!$  INTEGER :: dcpl
!!$  INTEGER ::is_empty
!!$  INTEGER ::is_dense
!!$
  WRITE(*,*) "Testing Compact Storage of Attributes with Creation Order Info"
  ! /* Create file */
  CALL h5fcreate_f(FileName, H5F_ACC_TRUNC_F, fid, error, fcpl, fapl)
  CALL check("h5fcreate_f",error,total_error)
  ! /* Create dataset creation property list */
  CALL H5Pcreate_f(H5P_DATASET_CREATE_F,dcpl,error)
  CALL check("h5Pcreate_f",error,total_error)

  CALL H5Pset_attr_creation_order_f(dcpl, IOR(H5P_CRT_ORDER_TRACKED_F, H5P_CRT_ORDER_INDEXED_F), error)
  CALL check("H5Pset_attr_creation_order",error,total_error)
! ret = H5Pset_attr_creation_order(dcpl, (H5P_CRT_ORDER_TRACKED | H5P_CRT_ORDER_INDEXED));

  ! /* Query the attribute creation properties */
  CALL H5Pget_attr_phase_change_f(dcpl, max_compact, min_dense, error)
  CALL check("H5Pget_attr_phase_change_f",error,total_error)
!!$  ret = H5Pget_attr_phase_change(dcpl, max_compact, min_dense)
!!$  CALL CHECK(ret, FAIL, "H5Pget_attr_phase_change")

  CALL h5screate_f(H5S_SCALAR_F, sid, error)
  CALL check("h5screate_f",error,total_error)

! FIX: need to check optional parameters i.e. h5dcreate1/2_f

  CALL h5dcreate_f(fid, DSET1_NAME, H5T_NATIVE_CHARACTER, sid, dset1, error, dcpl )
  CALL check("h5dcreate_f",error,total_error)

  CALL h5dcreate_f(fid, DSET2_NAME, H5T_NATIVE_CHARACTER, sid, dset2, error, dcpl )
  CALL check("h5dcreate_f",error,total_error)

  CALL h5dcreate_f(fid, DSET3_NAME, H5T_NATIVE_CHARACTER, sid, dset3, error, dcpl )
  CALL check("h5dcreate_f",error,total_error)

!!$  dset1 = H5Dcreate2(fid, DSET1_NAME, H5T_NATIVE_UCHAR, sid, H5P_DEFAULT, dcpl, H5P_DEFAULT)
!!$  dset2 = H5Dcreate2(fid, DSET2_NAME, H5T_NATIVE_UCHAR, sid, H5P_DEFAULT, dcpl, H5P_DEFAULT)
!!$  dset3 = H5Dcreate2(fid, DSET3_NAME, H5T_NATIVE_UCHAR, sid, H5P_DEFAULT, dcpl, H5P_DEFAULT)

  DO curr_dset = 0,NUM_DSETS-1
     SELECT CASE (curr_dset)
     CASE (0)
        my_dataset = dset1
     CASE (1)
        my_dataset = dset2
     CASE (2)
        my_dataset = dset3
!     CASE DEFAULT
!        CALL HDassert(0.AND."Toomanydatasets!")
     END SELECT
!!$    is_empty = H5O_is_attr_empty_test(my_dataset)
!!$    CALL VERIFY(is_empty, TRUE, "H5O_is_attr_empty_test")
!!$    is_dense = H5O_is_attr_dense_test(my_dataset)
!!$    CALL VERIFY(is_dense, FALSE, "H5O_is_attr_dense_test")
    DO u = 0, max_compact - 1
       ! /* Create attribute */
       WRITE(chr2,'(I2.2)') u
       attrname = 'attr '//chr2
       
           ! attr = H5Acreate2(my_dataset, attrname, H5T_NATIVE_UINT, sid, H5P_DEFAULT, H5P_DEFAULT);
           ! check with the optional information create2 specs.
       CALL h5acreate_f(my_dataset, attrname, H5T_NATIVE_INTEGER, sid, attr, error, H5P_DEFAULT_F, H5P_DEFAULT_F)
       CALL check("h5acreate_f",error,total_error)

       data_dims(1) = 1 
       CALL h5awrite_f(attr, H5T_NATIVE_INTEGER, u, data_dims, error)
       CALL check("h5awrite_f",error,total_error)

       CALL h5aclose_f(attr, error)
       CALL check("h5aclose_f",error,total_error)

!!$      ret = H5O_num_attrs_test(my_dataset, nattrs)
!!$      CALL CHECK(ret, FAIL, "H5O_num_attrs_test")
!!$      CALL VERIFY(nattrs, (u + 1))
!!$      is_empty = H5O_is_attr_empty_test(my_dataset)
!!$      CALL VERIFY(is_empty, FALSE, "H5O_is_attr_empty_test")
!!$      is_dense = H5O_is_attr_dense_test(my_dataset)
!!$      CALL VERIFY(is_dense, FALSE, "H5O_is_attr_dense_test")
    END DO
  END DO

  !  /* Close Datasets */
  CALL h5dclose_f(dset1, error)
  CALL check("h5dclose_f",error,total_error)
  CALL h5dclose_f(dset2, error)
  CALL check("h5dclose_f",error,total_error)
  CALL h5dclose_f(dset3, error)
  CALL check("h5dclose_f",error,total_error)
  
  !   /* Close file */
  CALL h5fclose_f(fid, error)
  CALL check("h5fclose_f",error,total_error)

  ! /* Close dataspace */
  CALL h5sclose_f(sid, error)
  CALL check("h5sclose_f",error,total_error)

  ! /* Close property list */
  CALL h5pclose_f(dcpl, error)
  CALL check("h5pclose_f",error,total_error)

  CALL h5fopen_f(FileName, H5F_ACC_RDWR_F, fid, error, fapl)
  CALL check("h5open_f",error,total_error)

!!$  fid = H5Fopen(FILENAME, H5F_ACC_RDWR, fapl)
!!$  CALL CHECK(fid, FAIL, "H5Fopen")

  CALL h5dopen_f(fid, DSET1_NAME, dset1, error)
  CALL check("h5dopen_f",error,total_error)
  CALL h5dopen_f(fid, DSET2_NAME, dset2, error)
  CALL check("h5dopen_f",error,total_error)
  CALL h5dopen_f(fid, DSET3_NAME, dset3, error)
  CALL check("h5dopen_f",error,total_error)
  DO curr_dset = 0,NUM_DSETS-1
     SELECT CASE (curr_dset)
     CASE (0)
        my_dataset = dset1
     CASE (1)
        my_dataset = dset2
     CASE (2)
        my_dataset = dset3
     CASE DEFAULT
        WRITE(*,*) " WARNING: To many data sets! "
     END SELECT
!!$    ret = H5O_num_attrs_test(my_dataset, nattrs)
!!$    CALL CHECK(ret, FAIL, "H5O_num_attrs_test")
!!$    CALL VERIFY(nattrs, max_compact, "H5O_num_attrs_test")
!!$    is_empty = H5O_is_attr_empty_test(my_dataset)
!!$    CALL VERIFY(is_empty, FALSE, "H5O_is_attr_empty_test")
!!$    is_dense = H5O_is_attr_dense_test(my_dataset)
!!$    CALL VERIFY(is_dense, FALSE, "H5O_is_attr_dense_test")

     DO u = 0,max_compact-1
        WRITE(chr2,'(I2.2)') u
        attrname = 'attr '//chr2
        ! /* Retrieve information for attribute */

        CALL H5Aget_info_by_name_f(my_dataset, ".", attrname, H5P_DEFAULT_F, &
             f_corder_valid, corder, cset, data_size, error )

        CALL check("H5Aget_info_by_name_f", error, total_error)
        
        ! /* Verify creation order of attribute */

        CALL verifyLogical("H5Aget_info_by_name_f", f_corder_valid, .TRUE., total_error)
        CALL verify("H5Aget_info_by_name_f", corder, u, total_error)

     END DO
  END DO
  !  /* Close Datasets */
  CALL h5dclose_f(dset1, error)
  CALL check("h5dclose_f",error,total_error)
  CALL h5dclose_f(dset2, error)
  CALL check("h5dclose_f",error,total_error)
  CALL h5dclose_f(dset3, error)
  CALL check("h5dclose_f",error,total_error)
  
  !   /* Close file */
  CALL h5fclose_f(fid, error)
  CALL check("h5fclose_f",error,total_error)

END SUBROUTINE test_attr_corder_create_compact

SUBROUTINE test_attr_null_space(fcpl, fapl, total_error)
! --------------------------------------------------
  USE HDF5
  IMPLICIT NONE

  INTEGER(HID_T), INTENT(IN) :: fcpl
  INTEGER(HID_T), INTENT(IN) :: fapl
  INTEGER, INTENT(INOUT) :: total_error

  CHARACTER(LEN=8) :: FileName = "tattr.h5"
  INTEGER(HID_T) :: fid
  INTEGER(HID_T) :: dcpl
  INTEGER(HID_T) :: sid, null_sid
  INTEGER(HID_T) :: dataset

  CHARACTER(LEN=8) :: DSET1_NAME = "Dataset1"
  CHARACTER(LEN=8) :: DSET2_NAME = "Dataset2"
  CHARACTER(LEN=8) :: DSET3_NAME = "Dataset3"
  INTEGER, PARAMETER :: NUM_DSETS = 3

  INTEGER :: curr_dset

  INTEGER(HID_T) :: dset1, dset2, dset3
  INTEGER(HID_T) :: my_dataset
  INTEGER :: error

  INTEGER :: value
  INTEGER(HID_T) :: attr        !String Attribute identifier 
  INTEGER(HID_T) :: attr_sid
  INTEGER(HSIZE_T), DIMENSION(7) :: data_dims
  INTEGER(HSIZE_T) :: storage_size   ! attributes storage requirements .MSB.

  LOGICAL :: f_corder_valid ! Indicates whether the the creation order data is valid for this attribute 
  INTEGER :: corder ! Is a positive integer containing the creation order of the attribute
  INTEGER :: cset ! Indicates the character set used for the attribute’s name
  INTEGER(HSIZE_T) :: data_size   ! indicates the size, in the number of characters
  data_dims = 0

!  CHARACTER (LEN=NAME_BUF_SIZE) :: attrname

  WRITE(*,*) "Testing Storing Attributes with 'null' dataspace"
  ! /* Create file */
  CALL h5fcreate_f(FileName, H5F_ACC_TRUNC_F, fid, error, fcpl, fapl)
  CALL check("h5fcreate_f",error,total_error)
! /* Close file */
  CALL h5fclose_f(fid, error)
  CALL check("h5fclose_f",error,total_error)

!!$  empty_filesize = h5_get_file_size(FILENAME)
!!$  IF (empty_filesize < 0) CALL TestErrPrintf("Line %d: file size wrong!\n"C, __LINE__)

  CALL h5fopen_f(FileName, H5F_ACC_RDWR_F, fid, error)
  CALL check("h5open_f",error,total_error)

  CALL h5screate_f(H5S_SCALAR_F, sid, error)
  CALL check("h5screate_f",error,total_error)

  CALL h5screate_f(H5S_NULL_F, null_sid, error)
  CALL check("h5screate_f",error,total_error)

  CALL h5dcreate_f(fid, DSET1_NAME, H5T_NATIVE_CHARACTER, sid, &
       dataset, error)
  CALL check("h5dcreate_f",error,total_error)
!!$  dataset = H5Dcreate2(fid, DSET1_NAME, H5T_NATIVE_UCHAR, sid, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)
!!$  CALL CHECK(dataset, FAIL, "H5Dcreate2")

  CALL h5acreate_f(dataset, "null attr", H5T_NATIVE_INTEGER, null_sid, attr, error)
  CALL check("h5acreate_f",error,total_error)

!!$  CALL HDstrcpy(attrname, "null attr")
!!$  attr = H5Acreate2(dataset, attrname, H5T_NATIVE_UINT, null_sid, H5P_DEFAULT, H5P_DEFAULT)

  value = 103
  data_dims(1) = 1
  CALL h5aread_f(attr, H5T_NATIVE_INTEGER, value, data_dims, error)
  CALL check("h5aread_f",error,total_error)
  CALL verify("h5aread_f",value,103,total_error)

  CALL h5aget_space_f(attr, attr_sid, error)
  CALL check("h5aget_space_f",error,total_error)

!!$  cmp = H5Sextent_equal(attr_sid, null_sid)
!!$  CALL CHECK(cmp, FAIL, "H5Sextent_equal")
!!$  CALL VERIFY(cmp, TRUE, "H5Sextent_equal")
!!$  ret = H5Sclose(attr_sid)
!!$  CALL CHECK(ret, FAIL, "H5Sclose")

  CALL h5aget_storage_size_f(attr, storage_size, error)
  CALL check("h5aget_storage_size_f",error,total_error)
  CALL verify("h5aget_storage_size_f",storage_size,0,total_error)

  CALL h5aget_info_f(attr, f_corder_valid, corder, cset, data_size,  error)
  CALL verify("h5aget_info_f",data_size,storage_size,total_error)


  CALL h5aclose_f(attr,error)
  CALL check("h5aclose_f",error,total_error)
  

!!$  CALL HDstrcpy(attrname, "null attr #2")
!!$  attr = H5Acreate2(dataset, attrname, H5T_NATIVE_UINT, null_sid, H5P_DEFAULT, H5P_DEFAULT)
!!$  CALL CHECK(attr, FAIL, "H5Acreate2")
!!$  value = 23
!!$  ret = H5Awrite(attr, H5T_NATIVE_UINT, value)
!!$  CALL CHECK(ret, FAIL, "H5Awrite")
!!$  CALL VERIFY(value, 23, "H5Awrite")
!!$  ret = H5Aclose(attr)
!!$  CALL CHECK(ret, FAIL, "H5Aclose")
!!$  ret = H5Dclose(dataset)
!!$  CALL CHECK(ret, FAIL, "H5Dclose")
!!$  ret = H5Fclose(fid)
!!$  CALL CHECK(ret, FAIL, "H5Fclose")
!!$  fid = H5Fopen(FILENAME, H5F_ACC_RDWR, fapl)
!!$  CALL CHECK(fid, FAIL, "H5Fopen")
!!$  dataset = H5Dopen2(fid, DSET1_NAME, H5P_DEFAULT)
!!$  CALL CHECK(dataset, FAIL, "H5Dopen2")
!!$  CALL HDstrcpy(attrname, "null attr #2")
!!$  attr = H5Aopen(dataset, attrname, H5P_DEFAULT)
!!$  CALL CHECK(attr, FAIL, "H5Aopen")
!!$  value = 23
!!$  ret = H5Aread(attr, H5T_NATIVE_UINT, value)
!!$  CALL CHECK(ret, FAIL, "H5Aread")
!!$  CALL VERIFY(value, 23, "H5Aread")
!!$  attr_sid = H5Aget_space(attr)
!!$  CALL CHECK(attr_sid, FAIL, "H5Aget_space")
!!$  cmp = H5Sextent_equal(attr_sid, null_sid)
!!$  CALL CHECK(cmp, FAIL, "H5Sextent_equal")
!!$  CALL VERIFY(cmp, TRUE, "H5Sextent_equal")

  
  CALL H5Sclose_f(attr_sid, error)
  CALL check("H5Sclose_f",error,total_error)
  

!!$  ret = H5Sclose(attr_sid)
!!$  CALL CHECK(ret, FAIL, "H5Sclose")
!!$  storage_size = H5Aget_storage_size(attr)
!!$  CALL VERIFY(storage_size, 0, "H5Aget_storage_size")
!!$  ret = H5Aget_info(attr, ainfo)
!!$  CALL CHECK(ret, FAIL, "H5Aget_info")
!!$  CALL VERIFY(ainfo%data_size, storage_size, "H5Aget_info")
!!$  ret = H5Aclose(attr)
!!$  CALL CHECK(ret, FAIL, "H5Aclose")
!!$  CALL HDstrcpy(attrname, "null attr")
!!$  attr = H5Aopen(dataset, attrname, H5P_DEFAULT)
!!$  CALL CHECK(attr, FAIL, "H5Aopen")
!!$  value = 23
!!$  ret = H5Awrite(attr, H5T_NATIVE_UINT, value)
!!$  CALL CHECK(ret, FAIL, "H5Awrite")
!!$  CALL VERIFY(value, 23, "H5Awrite")


!!$  CALL H5Aclose_f(attr, error)
!!$  CALL check("H5Aclose_f", error,total_error)
!!$  CALL H5Ddelete_f(fid, DSET1_NAME, H5P_DEFAULT_F, error)
!!$  CALL check("H5Aclose_f", error,total_error)

  CALL H5Dclose_f(dataset, error)
  CALL check("H5Dclose_f", error,total_error)

!!$  ret = H5delete(fid, DSET1_NAME, H5P_DEFAULT)
!!$  CALL CHECK(ret, FAIL, "H5Ldelete")

! TESTING1

  CALL H5Fclose_f(fid, error)
  CALL check("H5Fclose_f", error,total_error)

  CALL H5Sclose_f(sid, error)
  CALL check("H5Sclose_f", error,total_error)

  CALL H5Sclose_f(null_sid, error)
  CALL check("H5Sclose_f", error,total_error)

!!$  filesize = h5_get_file_size(FILENAME)
!!$  CALL VERIFY(filesize, empty_filesize, "h5_get_file_size")

END SUBROUTINE test_attr_null_space


SUBROUTINE test_attr_create_by_name(new_format,fcpl,fapl, total_error)

  USE HDF5

  IMPLICIT NONE

  INTEGER(HSIZE_T), PARAMETER :: NAME_BUF_SIZE = 7
  LOGICAL :: new_format 
  INTEGER(HID_T), INTENT(IN) :: fcpl
  INTEGER(HID_T), INTENT(IN) :: fapl
  INTEGER, INTENT(INOUT) :: total_error

  INTEGER :: max_compact,min_dense,u
  CHARACTER (LEN=NAME_BUF_SIZE) :: attrname
  CHARACTER(LEN=8) :: dsetname

  CHARACTER(LEN=8) :: FileName = "tattr.h5"
  INTEGER(HID_T) :: fid
  INTEGER(HID_T) :: dcpl
  INTEGER(HID_T) :: sid, null_sid
  INTEGER(HID_T) :: dataset

  CHARACTER(LEN=8) :: DSET1_NAME = "Dataset1"
  CHARACTER(LEN=8) :: DSET2_NAME = "Dataset2"
  CHARACTER(LEN=8) :: DSET3_NAME = "Dataset3"
  INTEGER, PARAMETER :: NUM_DSETS = 3

  INTEGER :: curr_dset

  INTEGER(HID_T) :: dset1, dset2, dset3
  INTEGER(HID_T) :: my_dataset
  INTEGER :: error

  INTEGER :: value
  INTEGER(HID_T) :: attr        !String Attribute identifier 
  INTEGER(HID_T) :: attr_sid
  INTEGER(HSIZE_T), DIMENSION(7) :: data_dims
  INTEGER(HSIZE_T) :: storage_size   ! attributes storage requirements .MSB.

  LOGICAL :: f_corder_valid ! Indicates whether the the creation order data is valid for this attribute 
  INTEGER :: corder ! Is a positive integer containing the creation order of the attribute
  INTEGER :: cset ! Indicates the character set used for the attribute’s name
  INTEGER(HSIZE_T) :: data_size   ! indicates the size, in the number of characters

  CHARACTER(LEN=2) :: chr2
  LOGICAL, DIMENSION(1:2) :: use_index = (/.FALSE.,.TRUE./) 
  INTEGER :: Input1
  INTEGER :: i

  data_dims = 0


  ! /* Create dataspace for dataset & attributes */
  CALL h5screate_f(H5S_SCALAR_F, sid, error)
  CALL check("h5screate_f",error,total_error)

  ! /* Create dataset creation property list */
  CALL H5Pcreate_f(H5P_DATASET_CREATE_F,dcpl,error)
  CALL check("h5Pcreate_f",error,total_error)

  ! /* Query the attribute creation properties */

  CALL H5Pget_attr_phase_change_f(dcpl, max_compact, min_dense, error)
  CALL check("H5Pget_attr_phase_change_f",error,total_error)

  ! /* Loop over using index for creation order value */
  DO i = 1, 2
     ! /* Print appropriate test message */
     IF(use_index(i))THEN
        WRITE(*,*) "Testing Creating Attributes By Name w/Creation Order Index"
     ELSE
        WRITE(*,*) "Testing Creating Attributes By Name w/o Creation Order Index"
     ENDIF
     ! /* Create file */
     CALL h5fcreate_f(FileName, H5F_ACC_TRUNC_F, fid, error, fcpl, fapl)
     CALL check("h5fcreate_f",error,total_error)

     ! /* Set attribute creation order tracking & indexing for object */
     IF(new_format)THEN

        IF(use_index(i))THEN
           Input1 = H5P_CRT_ORDER_INDEXED_F
        ELSE
           Input1 = 0
        ENDIF

        CALL H5Pset_attr_creation_order_f(dcpl, IOR(H5P_CRT_ORDER_TRACKED_F, Input1), error)
        CALL check("H5Pset_attr_creation_order",error,total_error)

     ENDIF

     ! /* Create datasets */

     CALL h5dcreate_f(fid, DSET1_NAME, H5T_NATIVE_CHARACTER, sid, dset1, error, dcpl )
     CALL check("h5dcreate_f2",error,total_error)
     
     CALL h5dcreate_f(fid, DSET2_NAME, H5T_NATIVE_CHARACTER, sid, dset2, error, dcpl )
     CALL check("h5dcreate_f3",error,total_error)
     
     CALL h5dcreate_f(fid, DSET3_NAME, H5T_NATIVE_CHARACTER, sid, dset3, error, dcpl )
     CALL check("h5dcreate_f4",error,total_error)


     ! /* Work on all the datasets */

     DO curr_dset = 0,NUM_DSETS-1
        SELECT CASE (curr_dset)
        CASE (0)
           my_dataset = dset1
           dsetname = DSET1_NAME
        CASE (1)
           my_dataset = dset2
           dsetname = DSET2_NAME
        CASE (2)
           my_dataset = dset3
           dsetname = DSET3_NAME
           !     CASE DEFAULT
           !        CALL HDassert(0.AND."Toomanydatasets!")
        END SELECT

        ! /* Check on dataset's attribute storage status */
!!$            is_empty = H5O_is_attr_empty_test(my_dataset);
!!$            VERIFY(is_empty, TRUE, "H5O_is_attr_empty_test");
!!$            is_dense = H5O_is_attr_dense_test(my_dataset);
!!$            VERIFY(is_dense, FALSE, "H5O_is_attr_dense_test");

        !/* Create attributes, up to limit of compact form */

        DO u = 0, max_compact - 1
           ! /* Create attribute */
           WRITE(chr2,'(I2.2)') u
           attrname = 'attr '//chr2
           CALL H5Acreate_by_name_f(fid, dsetname, attrname, H5T_NATIVE_INTEGER, sid, H5P_DEFAULT_F, H5P_DEFAULT_F, H5P_DEFAULT_F, &
                attr, error)
           CALL check("H5Acreate_by_name_f",error,total_error)
           
           ! /* Write data into the attribute */

           data_dims(1) = 1 
           CALL h5awrite_f(attr, H5T_NATIVE_INTEGER, u, data_dims, error)
           CALL check("h5awrite_f",error,total_error)

           ! /* Close attribute */
           CALL h5aclose_f(attr, error)
           CALL check("h5aclose_f",error,total_error)

           ! /* Verify information for NEW attribute */
           CALL attr_info_by_idx_check(my_dataset, attrname, INT(u,HSIZE_T), use_index, total_error)
         !   CALL check("FAILED IN attr_info_by_idx_check",total_error)
        ENDDO

        ! /* Verify state of object */
!!$            ret = H5O_num_attrs_test(my_dataset, &nattrs);
!!$            CHECK(ret, FAIL, "H5O_num_attrs_test");
!!$            VERIFY(nattrs, max_compact, "H5O_num_attrs_test");
!!$            is_empty = H5O_is_attr_empty_test(my_dataset);
!!$            VERIFY(is_empty, FALSE, "H5O_is_attr_empty_test");
!!$            is_dense = H5O_is_attr_dense_test(my_dataset);
!!$            VERIFY(is_dense, FALSE, "H5O_is_attr_dense_test");

        ! /* Test opening attributes stored compactly */

        CALL attr_open_check(fid, dsetname, my_dataset, u, total_error)

        !    CHECK(ret, FAIL, "attr_open_check");
     ENDDO


     ! /* Work on all the datasets */
     DO curr_dset = 0,NUM_DSETS-1
        SELECT CASE (curr_dset)
        CASE (0)
           my_dataset = dset1
           dsetname = DSET1_NAME
        CASE (1)
           my_dataset = dset2
           dsetname = DSET2_NAME
        CASE (2)
           my_dataset = dset3
           dsetname = DSET3_NAME
!     CASE DEFAULT
!        CALL HDassert(0.AND."Toomanydatasets!")
        END SELECT
        
        ! /* Create more attributes, to push into dense form */
        DO u = max_compact, max_compact* 2 - 1

           WRITE(chr2,'(I2.2)') u
           attrname = 'attr '//chr2

           CALL H5Acreate_by_name_f(fid, dsetname, attrname, H5T_NATIVE_INTEGER, sid, H5P_DEFAULT_F, H5P_DEFAULT_F, H5P_DEFAULT_F, &
                attr, error)
           CALL check("H5Acreate_by_name",error,total_error)

           ! /* Write data into the attribute */
           data_dims(1) = 1 
           CALL h5awrite_f(attr, H5T_NATIVE_INTEGER, u, data_dims, error)
           CALL check("h5awrite_f",error,total_error)

           ! /* Close attribute */
           CALL h5aclose_f(attr, error)
           CALL check("h5aclose_f",error,total_error)

           ! /* Verify state of object */
!!$                if(u >= max_compact) {
!!$                    is_dense = H5O_is_attr_dense_test(my_dataset);
!!$                    VERIFY(is_dense, (new_format ? TRUE : FALSE), "H5O_is_attr_dense_test");
!!$                } /* end if */
!!$
!!$                /* Verify information for new attribute */
!!$                ret = attr_info_by_idx_check(my_dataset, attrname, (hsize_t)u, use_index);
!!$                CHECK(ret, FAIL, "attr_info_by_idx_check");
        ENDDO

        ! /* Verify state of object */
!!$            ret = H5O_num_attrs_test(my_dataset, &nattrs);
!!$            CHECK(ret, FAIL, "H5O_num_attrs_test");
!!$            VERIFY(nattrs, (max_compact * 2), "H5O_num_attrs_test");
!!$            is_empty = H5O_is_attr_empty_test(my_dataset);
!!$            VERIFY(is_empty, FALSE, "H5O_is_attr_empty_test");
!!$            is_dense = H5O_is_attr_dense_test(my_dataset);
!!$            VERIFY(is_dense, (new_format ? TRUE : FALSE), "H5O_is_attr_dense_test");

!!$            if(new_format) {
!!$                /* Retrieve & verify # of records in the name & creation order indices */
!!$                ret = H5O_attr_dense_info_test(my_dataset, &name_count, &corder_count);
!!$                CHECK(ret, FAIL, "H5O_attr_dense_info_test");
!!$                if(use_index)
!!$                    VERIFY(name_count, corder_count, "H5O_attr_dense_info_test");
!!$                VERIFY(name_count, (max_compact * 2), "H5O_attr_dense_info_test");
!!$            } /* end if */

!!$            /* Test opening attributes stored compactly */
!!$            ret = attr_open_check(fid, dsetname, my_dataset, u);
!!$            CHECK(ret, FAIL, "attr_open_check");

     ENDDO

     ! /* Close Datasets */
     CALL h5dclose_f(dset1, error)
     CALL check("h5dclose_f",error,total_error)
     CALL h5dclose_f(dset2, error)
     CALL check("h5dclose_f",error,total_error)
     CALL h5dclose_f(dset3, error)
     CALL check("h5dclose_f",error,total_error)


     ! /* Close file */
     CALL h5fclose_f(fid, error)
     CALL check("h5fclose_f",error,total_error)
  ENDDO

  ! /* Close property list */
  CALL h5pclose_f(dcpl, error)
  CALL check("h5pclose_f",error,total_error)

  ! /* Close dataspace */
  CALL h5sclose_f(sid, error)
  CALL check("h5sclose_f",error,total_error)

END SUBROUTINE test_attr_create_by_name


SUBROUTINE test_attr_info_by_idx(new_format, fcpl, fapl, total_error)

  USE HDF5

  IMPLICIT NONE

  LOGICAL :: new_format
  INTEGER(HID_T), INTENT(IN) :: fcpl
  INTEGER(HID_T), INTENT(IN) :: fapl
  INTEGER, INTENT(INOUT) :: total_error
  CHARACTER(LEN=8) :: FileName = "tattr.h5"
  INTEGER(HID_T) :: fid
  INTEGER(HID_T) :: dcpl
  INTEGER(HID_T) :: sid, null_sid
  INTEGER(HID_T) :: dataset

  CHARACTER(LEN=8) :: DSET1_NAME = "Dataset1"
  CHARACTER(LEN=8) :: DSET2_NAME = "Dataset2"
  CHARACTER(LEN=8) :: DSET3_NAME = "Dataset3"
  INTEGER, PARAMETER :: NUM_DSETS = 3

  INTEGER :: curr_dset

  INTEGER(HID_T) :: dset1, dset2, dset3
  INTEGER(HID_T) :: my_dataset
  INTEGER :: error

  INTEGER :: value
  INTEGER(HID_T) :: attr        !String Attribute identifier 
  INTEGER(HID_T) :: attr_sid
  INTEGER(HSIZE_T), DIMENSION(7) :: data_dims
  INTEGER(HSIZE_T) :: storage_size   ! attributes storage requirements .MSB.

  LOGICAL :: f_corder_valid ! Indicates whether the the creation order data is valid for this attribute 
  INTEGER :: corder ! Is a positive integer containing the creation order of the attribute
  INTEGER :: cset ! Indicates the character set used for the attribute’s name
  INTEGER(HSIZE_T) :: data_size   ! indicates the size, in the number of characters
  INTEGER(HSIZE_T) :: n
  LOGICAL, DIMENSION(1:2) :: use_index = (/.FALSE.,.TRUE./)

  INTEGER :: max_compact ! Maximum # of links to store in group compactly
  INTEGER :: min_dense   ! Minimum # of links to store in group "densely"

  CHARACTER(LEN=2) :: chr2

  INTEGER :: i, j
  INTEGER(HSIZE_T) :: nn

  INTEGER, DIMENSION(1) ::  attr_integer_data
  CHARACTER(LEN=7) :: attrname

  INTEGER(SIZE_T) :: size
  CHARACTER(LEN=80) :: tmpname

  INTEGER :: Input1

  data_dims = 0
!!$    htri_t	is_empty;	/* Are there any attributes? */
!!$    htri_t	is_dense;	/* Are attributes stored densely? */
!!$    hsize_t     nattrs;         /* Number of attributes on object */
!!$    hsize_t     name_count;     /* # of records in name index */
!!$    hsize_t     corder_count;   /* # of records in creation order index */
!!$    hbool_t     use_index;      /* Use index on creation order values */
!!$    char	attrname[NAME_BUF_SIZE];    /* Name of attribute */
!!$    char        tmpname[NAME_BUF_SIZE];     /* Temporary attribute name */
!!$    unsigned    curr_dset;      /* Current dataset to work on */
!!$    unsigned    u;              /* Local index variable */
!!$    herr_t	ret;		/* Generic return value		*/

  ! /* Create dataspace for dataset & attributes */
  
  CALL h5screate_f(H5S_SCALAR_F, sid, error)
  CALL check("h5screate_f",error,total_error)


  ! /* Create dataset creation property list */

  CALL H5Pcreate_f(H5P_DATASET_CREATE_F,dcpl,error)
  CALL check("h5Pcreate_f",error,total_error)


  ! /* Query the attribute creation properties */
  CALL H5Pget_attr_phase_change_f(dcpl, max_compact, min_dense, error)
  CALL check("H5Pget_attr_phase_change_f",error,total_error)

  !  /* Loop over using index for creation order value */
    
  DO i = 1, 2

     ! /* Output message about test being performed */
     IF(use_index(i))THEN
        WRITE(*,'(A63)') "Testing Querying Attribute Info By Index w/Creation Order Index"
     ELSE
        WRITE(*,'(A65)') "Testing Querying Attribute Info By Index w/o Creation Order Index"
     ENDIF

     ! /* Create file */
     CALL h5fcreate_f(FileName, H5F_ACC_TRUNC_F, fid, error, fcpl, fapl)
     CALL check("h5fcreate_f",error,total_error)

        ! /* Set attribute creation order tracking & indexing for object */
     IF(new_format)THEN
        IF(use_index(i))THEN
           Input1 = H5P_CRT_ORDER_INDEXED_F
        ELSE
           Input1 = 0
        ENDIF
        CALL H5Pset_attr_creation_order_f(dcpl, IOR(H5P_CRT_ORDER_TRACKED_F, Input1), error)
        CALL check("H5Pset_attr_creation_order",error,total_error)
     ENDIF

     ! /* Create datasets */
     
     CALL h5dcreate_f(fid, DSET1_NAME, H5T_NATIVE_CHARACTER, sid, dset1, error )
     CALL check("h5dcreate_f",error,total_error)
     
     CALL h5dcreate_f(fid, DSET2_NAME, H5T_NATIVE_CHARACTER, sid, dset2, error )
     CALL check("h5dcreate_f",error,total_error)
     
     CALL h5dcreate_f(fid, DSET3_NAME, H5T_NATIVE_CHARACTER, sid, dset3, error )
     CALL check("h5dcreate_f",error,total_error)

     ! /* Work on all the datasets */
     
     DO curr_dset = 0,NUM_DSETS-1

        SELECT CASE (curr_dset)
        CASE (0)
           my_dataset = dset1
        CASE (1)
           my_dataset = dset2
        CASE (2)
           my_dataset = dset3
           !     CASE DEFAULT
           !        CALL HDassert(0.AND."Toomanydatasets!")
        END SELECT

        !/* Check on dataset's attribute storage status */
!!$            is_empty = H5O_is_attr_empty_test(my_dataset);
!!$            VERIFY(is_empty, TRUE, "H5O_is_attr_empty_test");
!!$            is_dense = H5O_is_attr_dense_test(my_dataset);
!!$            VERIFY(is_dense, FALSE, "H5O_is_attr_dense_test");

        ! /* Check for query on non-existant attribute */

        n = 0
        CALL h5aget_info_by_idx_f(my_dataset, ".", H5_INDEX_CRT_ORDER_F, H5_ITER_INC_F, 0_HSIZE_T, H5P_DEFAULT_F, &
             f_corder_valid, corder, cset, data_size, error)
        CALL VERIFY("h5aget_info_by_idx_f",error,-1,total_error)
        
        size = 0
        CALL h5aget_name_by_idx_f(my_dataset, ".", H5_INDEX_CRT_ORDER_F, H5_ITER_INC_F, &
             0_HSIZE_T, tmpname, size, H5P_DEFAULT_F, error)
        CALL VERIFY("h5aget_name_by_idx_f",error,-1,total_error)


        ! /* Create attributes, up to limit of compact form */

        DO j = 0, max_compact-1
           ! /* Create attribute */
           WRITE(chr2,'(I2.2)') j
           attrname = 'attr '//chr2

           ! attr = H5Acreate2(my_dataset, attrname, H5T_NATIVE_UINT, sid, H5P_DEFAULT, H5P_DEFAULT);
           ! check with the optional information create2 specs.
           CALL h5acreate_f(my_dataset, attrname, H5T_NATIVE_INTEGER, sid, attr, error, H5P_DEFAULT_F, H5P_DEFAULT_F)
           CALL check("h5acreate_f",error,total_error)
           
           ! /* Write data into the attribute */

           attr_integer_data(1) = j
           data_dims(1) = 1
           CALL h5awrite_f(attr, H5T_NATIVE_INTEGER, attr_integer_data, data_dims, error)
           CALL check("h5awrite_f",error,total_error)

           ! /* Close attribute */

           CALL h5aclose_f(attr, error)
           CALL check("h5aclose_f",error,total_error)

           ! /* Verify information for new attribute */
              
           CALL attr_info_by_idx_check(my_dataset, attrname, INT(j,HSIZE_T), use_index(i), total_error )
          
           CALL check("attr_info_by_idx_check",error,total_error)

           !CHECK(ret, FAIL, "attr_info_by_idx_check");
        ENDDO

            ! /* Verify state of object */
!!$            ret = H5O_num_attrs_test(my_dataset, &nattrs);
!!$            CHECK(ret, FAIL, "H5O_num_attrs_test");
!!$            VERIFY(nattrs, max_compact, "H5O_num_attrs_test");
!!$            is_empty = H5O_is_attr_empty_test(my_dataset);
!!$            VERIFY(is_empty, FALSE, "H5O_is_attr_empty_test");
!!$            is_dense = H5O_is_attr_dense_test(my_dataset);
!!$            VERIFY(is_dense, FALSE, "H5O_is_attr_dense_test");

            ! /* Check for out of bound offset queries */
!!$            ret = H5Aget_info_by_idx(my_dataset, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, (hsize_t)u, &ainfo, H5P_DEFAULT);
!!$            VERIFY(ret, FAIL, "H5Aget_info_by_idx");
!!$            ret = H5Aget_info_by_idx(my_dataset, ".", H5_INDEX_CRT_ORDER, H5_ITER_DEC, (hsize_t)u, &ainfo, H5P_DEFAULT);
!!$            VERIFY(ret, FAIL, "H5Aget_info_by_idx");
!!$            ret = H5Aget_name_by_idx(my_dataset, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, (hsize_t)u, tmpname, (size_t)NAME_BUF_SIZE, H5P_DEFAULT);
!!$            VERIFY(ret, FAIL, "H5Aget_name_by_idx");
!!$
!!$            /* Create more attributes, to push into dense form */
!!$            for(; u < (max_compact * 2); u++) {
!!$                /* Create attribute */
!!$                sprintf(attrname, "attr %02u", u);
!!$                attr = H5Acreate2(my_dataset, attrname, H5T_NATIVE_UINT, sid, H5P_DEFAULT, H5P_DEFAULT);
!!$                CHECK(attr, FAIL, "H5Acreate2");
!!$
!!$                /* Write data into the attribute */
!!$                ret = H5Awrite(attr, H5T_NATIVE_UINT, &u);
!!$                CHECK(ret, FAIL, "H5Awrite");
!!$
!!$                /* Close attribute */
!!$                ret = H5Aclose(attr);
!!$                CHECK(ret, FAIL, "H5Aclose");
!!$
!!$                /* Verify state of object */
!!$                is_dense = H5O_is_attr_dense_test(my_dataset);
!!$                VERIFY(is_dense, (new_format ? TRUE : FALSE), "H5O_is_attr_dense_test");
!!$
!!$                /* Verify information for new attribute */
!!$                ret = attr_info_by_idx_check(my_dataset, attrname, (hsize_t)u, use_index);
!!$                CHECK(ret, FAIL, "attr_info_by_idx_check");
!!$            } /* end for */
!!$
!!$            /* Verify state of object */
!!$            ret = H5O_num_attrs_test(my_dataset, &nattrs);
!!$            CHECK(ret, FAIL, "H5O_num_attrs_test");
!!$            VERIFY(nattrs, (max_compact * 2), "H5O_num_attrs_test");
!!$            is_empty = H5O_is_attr_empty_test(my_dataset);
!!$            VERIFY(is_empty, FALSE, "H5O_is_attr_empty_test");
!!$            is_dense = H5O_is_attr_dense_test(my_dataset);
!!$            VERIFY(is_dense, (new_format ? TRUE : FALSE), "H5O_is_attr_dense_test");
!!$
!!$            if(new_format) {
!!$                /* Retrieve & verify # of records in the name & creation order indices */
!!$                ret = H5O_attr_dense_info_test(my_dataset, &name_count, &corder_count);
!!$                CHECK(ret, FAIL, "H5O_attr_dense_info_test");
!!$                if(use_index)
!!$                    VERIFY(name_count, corder_count, "H5O_attr_dense_info_test");
!!$                VERIFY(name_count, (max_compact * 2), "H5O_attr_dense_info_test");
!!$            } /* end if */
!!$
!!$            /* Check for out of bound offset queries */
!!$            ret = H5Aget_info_by_idx(my_dataset, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, (hsize_t)u, &ainfo, H5P_DEFAULT);
!!$            VERIFY(ret, FAIL, "H5Aget_info_by_idx");
!!$            ret = H5Aget_info_by_idx(my_dataset, ".", H5_INDEX_CRT_ORDER, H5_ITER_DEC, (hsize_t)u, &ainfo, H5P_DEFAULT);
!!$            VERIFY(ret, FAIL, "H5Aget_info_by_idx");
!!$            ret = H5Aget_name_by_idx(my_dataset, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, (hsize_t)u, tmpname, (size_t)NAME_BUF_SIZE, H5P_DEFAULT);
!!$            VERIFY(ret, FAIL, "H5Aget_name_by_idx");
!!$        } /* end for */
!!$

!!$    } /* end for */
!!$

     ENDDO


     !  /* Close Datasets */
     CALL h5dclose_f(dset1, error)
     CALL check("h5dclose_f",error,total_error)
     CALL h5dclose_f(dset2, error)
     CALL check("h5dclose_f",error,total_error)
     CALL h5dclose_f(dset3, error)
     CALL check("h5dclose_f",error,total_error)

     !   /* Close file */
     CALL h5fclose_f(fid, error)
     CALL check("h5fclose_f",error,total_error)
    
  END DO

  ! /* Close property list */
  CALL h5pclose_f(dcpl,error)
  CALL check("h5pclose_f", error, total_error)

  ! /* Close dataspace */
  CALL h5sclose_f(sid, error)
  CALL check("h5sclose_f",error,total_error)

END SUBROUTINE test_attr_info_by_idx


SUBROUTINE attr_info_by_idx_check(obj_id, attrname, n, use_index, total_error )

  USE HDF5

  IMPLICIT NONE

  INTEGER :: error, total_error

  INTEGER :: obj_id
  CHARACTER(LEN=*) :: attrname
  INTEGER(HSIZE_T) :: n
  LOGICAL :: use_index
  LOGICAL :: f_corder_valid ! Indicates whether the the creation order data is valid for this attribute 
  INTEGER :: corder ! Is a positive integer containing the creation order of the attribute
  INTEGER :: cset ! Indicates the character set used for the attribute’s name
  INTEGER(HSIZE_T) :: data_size   ! indicates the size, in the number of characters

  INTEGER(HSIZE_T) :: NAME_BUF_SIZE = 7
  CHARACTER(LEN=7) :: tmpname

!!$
!!$  INTEGER :: const
!!$  INTEGER :: har
!!$  INTEGER :: attrname
!!$  INTEGER :: hsize_t
!!$  INTEGER :: hbool_t
!!$  INTEGER :: se_index
!!$  INTEGER :: old_nerrs
!!$  CHARACTER (LEN=NAME_BUF_SIZE) :: tmpname
!!$    ainfo
!!$    ret
!!$    old_nerrs = GetTestNumErrs()

  ! /* Verify the information for first attribute, in increasing creation order */
  CALL h5aget_info_by_idx_f(obj_id, ".", H5_INDEX_CRT_ORDER_F, H5_ITER_INC_F, INT(0,HSIZE_T), H5P_DEFAULT_F, &
       f_corder_valid, corder, cset, data_size, error)
  
  CALL check("h5aget_info_by_idx_f1",error,total_error)
  CALL verify("h5aget_info_by_idx_f2",corder,0,total_error)

  ! /* Verify the information for new attribute, in increasing creation order */

  CALL h5aget_info_by_idx_f(obj_id, ".", H5_INDEX_CRT_ORDER_F, H5_ITER_INC_F, n, H5P_DEFAULT_F, &
       f_corder_valid, corder, cset, data_size, error)

  CALL check("h5aget_info_by_idx_f3",error,total_error)
  CALL verify("h5aget_info_by_idx_f4",corder,n,total_error)

  ! /* Verify the name for new link, in increasing creation order */

!!$    CALL HDmemset(tmpname, 0, (size_t))

  CALL h5aget_name_by_idx_f(obj_id, ".", H5_INDEX_CRT_ORDER_F, H5_ITER_INC_F, &
       n, tmpname, NAME_BUF_SIZE, H5P_DEFAULT_F, error)
  CALL check("h5aget_name_by_idx_f",error,total_error)

  IF(TRIM(attrname).NE.TRIM(tmpname))THEN
     error = -1
  ENDIF
  CALL VERIFY("h5aget_name_by_idx_f",error,0,total_error)
  
  !  /* Don't test "native" order if there is no creation order index, since
  !   *  there's not a good way to easily predict the attribute's order in the name
  !   *  index.
  !   */
  IF (use_index) THEN
     !  CALL HDmemset(ainfo, 0, SIZEOF(ainfo)
     ! /* Verify the information for first attribute, in native creation order */
     CALL h5aget_info_by_idx_f(obj_id, ".", H5_INDEX_CRT_ORDER_F, H5_ITER_NATIVE_F, INT(0,HSIZE_T), H5P_DEFAULT_F, &
          f_corder_valid, corder, cset, data_size, error)
     CALL check("h5aget_info_by_idx_f5",error,total_error)
     CALL VERIFY("h5aget_info_by_idx_f6",corder,0,total_error)

     ! CALL HDmemset(ainfo, 0, SIZEOF(ainfo)

     ! /* Verify the information for new attribute, in native creation order */
     CALL h5aget_info_by_idx_f(obj_id, ".", H5_INDEX_CRT_ORDER_F, H5_ITER_NATIVE_F, n, H5P_DEFAULT_F, &
          f_corder_valid, corder, cset, data_size, error)
     CALL check("h5aget_info_by_idx_f7",error,total_error)
     CALL VERIFY("h5aget_info_by_idx_f8",corder,n,total_error)

   ! /* Verify the name for new link, in increasing native order */
   ! CALL HDmemset(tmpname, 0, (size_t))
     CALL h5aget_name_by_idx_f(obj_id, ".", H5_INDEX_CRT_ORDER_F, H5_ITER_NATIVE_F, &
          n, tmpname, NAME_BUF_SIZE, H5P_DEFAULT_F, error)
     CALL check("h5aget_name_by_idx_f",error,total_error)
     IF(TRIM(attrname).NE.TRIM(tmpname))THEN
        WRITE(*,*) "ERROR: attribute name size wrong!"
        error = -1
     ENDIF
     CALL VERIFY("h5aget_name_by_idx_f",error,0,total_error)
  END IF


  ! CALL HDmemset(ainfo, 0, SIZEOF(ainfo)
  CALL h5aget_info_by_idx_f(obj_id, ".", H5_INDEX_CRT_ORDER_F, H5_ITER_DEC_F, n, H5P_DEFAULT_F, &
       f_corder_valid, corder, cset, data_size, error)
  CALL check("h5aget_info_by_idx_f9",error,total_error)
  CALL VERIFY("h5aget_info_by_idx_f10",corder,0,total_error)
  
  ! CALL HDmemset(ainfo, 0, SIZEOF(ainfo)
  CALL h5aget_info_by_idx_f(obj_id, ".", H5_INDEX_CRT_ORDER_F, H5_ITER_DEC_F, 0_HSIZE_T, H5P_DEFAULT_F, &
       f_corder_valid, corder, cset, data_size, error)
  CALL check("h5aget_info_by_idx_f11",error,total_error)
  CALL VERIFY("h5aget_info_by_idx_f12",corder,n,total_error)
!!$    CALL HDmemset(tmpname, 0, (size_t))
!!$    ret = H5Aget_name_by_idx(obj_id, ".", H5_INDEX_CRT_ORDER, H5_ITER_DEC, (hsize_t)0, tmpname, (size_t)NAME_BUF_SIZE, H5P_DEFAULT)
!!$    CALL CHECK(ret, FAIL, "H5Aget_name_by_idx")
!!$    IF (HDstrcmp(attrname, tmpname)) CALL TestErrPrintf("Line %d: attribute name size wrong!\n"C, __LINE__)
!!$    CALL HDmemset(ainfo, 0, SIZEOF(ainfo)
  CALL h5aget_info_by_idx_f(obj_id, ".", H5_INDEX_NAME_F, H5_ITER_INC_F, 0_HSIZE_T, H5P_DEFAULT_F, &
       f_corder_valid, corder, cset, data_size, error)
  CALL check("h5aget_info_by_idx_f13",error,total_error)
  CALL VERIFY("h5aget_info_by_idx_f14",corder,0,total_error)
!!$    CALL HDmemset(ainfo, 0, SIZEOF(ainfo)
  CALL h5aget_info_by_idx_f(obj_id, ".", H5_INDEX_NAME_F, H5_ITER_INC_F, n, H5P_DEFAULT_F, &
       f_corder_valid, corder, cset, data_size, error)
  CALL check("h5aget_info_by_idx_f15",error,total_error)
  CALL VERIFY("h5aget_info_by_idx_f16",corder,n,total_error)
!!$    CALL HDmemset(tmpname, 0, (size_t))
!!$    ret = H5Aget_name_by_idx(obj_id, ".", H5_INDEX_NAME, H5_ITER_INC, n, tmpname, (size_t)NAME_BUF_SIZE, H5P_DEFAULT)
!!$    CALL CHECK(ret, FAIL, "H5Aget_name_by_idx")
!!$    IF (HDstrcmp(attrname, tmpname)) CALL TestErrPrintf("Line %d: attribute name size wrong!\n"C, __LINE__)
!!$    CALL HDmemset(ainfo, 0, SIZEOF(ainfo)
  CALL h5aget_info_by_idx_f(obj_id, ".", H5_INDEX_NAME_F, H5_ITER_DEC_F, n, H5P_DEFAULT_F, &
       f_corder_valid, corder, cset, data_size, error)
  CALL check("h5aget_info_by_idx_f17",error,total_error)
  CALL VERIFY("h5aget_info_by_idx_f18",corder,0,total_error)
!!$    CALL HDmemset(ainfo, 0, SIZEOF(ainfo)
  CALL h5aget_info_by_idx_f(obj_id, ".", H5_INDEX_NAME_F, H5_ITER_DEC_F, 0_HSIZE_T, H5P_DEFAULT_F, &
       f_corder_valid, corder, cset, data_size, error)
  CALL check("h5aget_info_by_idx_f19",error,total_error)
  CALL VERIFY("h5aget_info_by_idx_f20",corder,n,total_error)
!!$    CALL HDmemset(tmpname, 0, (size_t))
!!$    ret = H5Aget_name_by_idx(obj_id, ".", H5_INDEX_NAME, H5_ITER_DEC, (hsize_t)0, tmpname, (size_t)NAME_BUF_SIZE, H5P_DEFAULT)
!!$    CALL CHECK(ret, FAIL, "H5Aget_name_by_idx")
!!$    IF (HDstrcmp(attrname, tmpname)) CALL TestErrPrintf("Line %d: attribute name size wrong!\n"C, __LINE__)

END SUBROUTINE attr_info_by_idx_check


SUBROUTINE test_attr_shared_rename( fcpl, fapl, total_error)

!/****************************************************************
!**
!**  test_attr_shared_rename(): Test basic H5A (attribute) code.
!**      Tests renaming shared attributes in "compact" & "dense" storage
!**
!****************************************************************/

  USE HDF5

  IMPLICIT NONE

  INTEGER(HID_T), INTENT(IN) :: fcpl
  INTEGER(HID_T), INTENT(IN) :: fapl
  INTEGER, INTENT(INOUT) :: total_error

    CHARACTER(LEN=8) :: FileName = "tattr.h5"
  INTEGER(HID_T) :: fid
  INTEGER(HID_T) :: dcpl
  INTEGER(HID_T) :: sid, big_sid

  CHARACTER(LEN=8) :: DSET1_NAME = "Dataset1"
  CHARACTER(LEN=8) :: DSET2_NAME = "Dataset2"
  CHARACTER(LEN=8) :: DSET3_NAME = "Dataset3"
  INTEGER, PARAMETER :: NUM_DSETS = 3

  INTEGER :: curr_dset

  INTEGER(HID_T) :: dset1, dset2, dset3
  INTEGER(HID_T) :: dataset, dataset2
  INTEGER(HID_T) :: my_dataset

  INTEGER :: error

  INTEGER :: value
  INTEGER(HID_T) :: attr        !String Attribute identifier 
  INTEGER(HID_T) :: attr_sid
  INTEGER(HID_T) :: attr_tid
  INTEGER(HSIZE_T), DIMENSION(7) :: data_dims
  INTEGER(HSIZE_T) :: storage_size   ! attributes storage requirements .MSB.

  LOGICAL :: f_corder_valid ! Indicates whether the the creation order data is valid for this attribute 
  INTEGER :: corder ! Is a positive integer containing the creation order of the attribute
  INTEGER :: cset ! Indicates the character set used for the attribute’s name
  INTEGER(HSIZE_T) :: data_size   ! indicates the size, in the number of characters
  INTEGER(HSIZE_T) :: n
  LOGICAL, DIMENSION(1:2) :: use_index = (/.FALSE.,.TRUE./)

  INTEGER :: max_compact ! Maximum # of links to store in group compactly
  INTEGER :: min_dense   ! Minimum # of links to store in group "densely"

  CHARACTER(LEN=2) :: chr2

  INTEGER :: i, j

  INTEGER, DIMENSION(1) ::  attr_integer_data
  CHARACTER(LEN=7) :: attrname
  CHARACTER(LEN=11) :: attrname2

  INTEGER(SIZE_T) :: size
  CHARACTER(LEN=7) :: tmpname
  CHARACTER(LEN=1), PARAMETER :: chr1 = '.'
  
  INTEGER :: idx_type
  INTEGER :: order
  INTEGER :: u 
  INTEGER :: Input1
  INTEGER, PARAMETER :: SPACE1_RANK = 3
  INTEGER, PARAMETER :: NX = 20
  INTEGER, PARAMETER :: NY = 5
  INTEGER, PARAMETER :: NZ = 10
  INTEGER(HSIZE_T), DIMENSION(3) :: big_dims = (/NX,NY,NZ/)
  INTEGER(HID_T) :: my_fcpl

  CHARACTER(LEN=5), PARAMETER :: TYPE1_NAME = "/Type"

  INTEGER :: test_shared

  ! /* Output message about test being performed */
  WRITE(*,*) "Testing Renaming Shared & Unshared Attributes in Compact & Dense Storage"
!!$ /* Initialize "big" attribute data */
!!$  CALL HDmemset(big_value, 1, SIZEOF(big_value)

  ! /* Create dataspace for dataset */
  CALL h5screate_f(H5S_SCALAR_F, sid, error)
  CALL check("h5screate_f",error,total_error)

  ! /* Create "big" dataspace for "large" attributes */

  CALL h5screate_simple_f(SPACE1_RANK, big_dims, big_sid, error)
  CALL check("h5screate_simple_f",error,total_error)

  ! /* Loop over type of shared components */
  DO test_shared = 0, 2
     ! /* Make copy of file creation property list */
     CALL H5Pcopy_f(fcpl, my_fcpl, error)
     CALL check("H5Pcopy",error,total_error)

     ! /* Set up datatype for attributes */

     CALL H5Tcopy_f(H5T_NATIVE_INTEGER, attr_tid, error)
     CALL check("H5Tcopy",error,total_error)

     ! /* Special setup for each type of shared components */

     IF( test_shared .EQ. 0) THEN
        ! /* Make attributes > 500 bytes shared */
        CALL H5Pset_shared_mesg_nindexes_f(my_fcpl,1,error)
        CALL check("H5Pset_shared_mesg_nindexes_f",error, total_error)
!!$     CHECK_I(ret, "H5Pset_shared_mesg_nindexes");
        CALL H5Pset_shared_mesg_index_f(my_fcpl, 0, H5O_SHMESG_ATTR_FLAG_F, 500,error)
        CALL check(" H5Pset_shared_mesg_index_f",error, total_error)

!!$     CHECK_I(ret, "H5Pset_shared_mesg_index");
     ELSE
        ! /* Set up copy of file creation property list */
        CALL H5Pset_shared_mesg_nindexes_f(my_fcpl,3,error)
!!$
!!$     CHECK_I(ret, "H5Pset_shared_mesg_nindexes");
!!$ 
        ! /* Make attributes > 500 bytes shared */
        CALL H5Pset_shared_mesg_index_f(my_fcpl, 0, H5O_SHMESG_ATTR_FLAG_F, 500,error)
!!$     CHECK_I(ret, "H5Pset_shared_mesg_index");
!!$ 
        ! /* Make datatypes & dataspaces > 1 byte shared (i.e. all of them :-) */
        CALL H5Pset_shared_mesg_index_f(my_fcpl, 1, H5O_SHMESG_DTYPE_FLAG_F, 1,error)
!!$            CHECK_I(ret, "H5Pset_shared_mesg_index");
        CALL H5Pset_shared_mesg_index_f(my_fcpl, 2,  H5O_SHMESG_SDSPACE_FLAG_F, 1,error)
!!$            CHECK_I(ret, "H5Pset_shared_mesg_index");
     ENDIF

     ! /* Create file */
     CALL h5fcreate_f(FileName, H5F_ACC_TRUNC_F, fid, error, my_fcpl, fapl)
     CALL check("h5fcreate_f",error,total_error)

     ! /* Close FCPL copy */
     CALL h5pclose_f(my_fcpl, error)
     CALL check("h5pclose_f", error, total_error)
     ! /* Close file */
     CALL h5fclose_f(fid, error)
     CALL check("h5fclose_f",error,total_error)

!!$
!!$        /* Get size of file */
!!$        empty_filesize = h5_get_file_size(FILENAME);
!!$        if(empty_filesize < 0)
!!$            TestErrPrintf("Line %d: file size wrong!\n", __LINE__);

     ! /* Re-open file */
     CALL h5fopen_f(FileName, H5F_ACC_RDWR_F, fid, error,fapl)
     CALL check("h5open_f",error,total_error)
     
     ! /* Commit datatype to file */
     IF(test_shared.EQ.2) THEN
        CALL H5Tcommit_f(fid, TYPE1_NAME, attr_tid, H5P_DEFAULT_F, H5P_DEFAULT_F, H5P_DEFAULT_F,error)
        CALL check("H5Tcommit",error,total_error)
     ENDIF

     ! /* Set up to query the object creation properties */
     CALL H5Pcreate_f(H5P_DATASET_CREATE_F,dcpl,error)
     CALL check("h5Pcreate_f",error,total_error)

     ! /* Create datasets */
     CALL h5dcreate_f(fid, DSET1_NAME, H5T_NATIVE_CHARACTER, sid, dataset, error, dcpl )
     CALL check("h5dcreate_f",error,total_error)
     CALL h5dcreate_f(fid, DSET2_NAME, H5T_NATIVE_CHARACTER, sid, dataset2, error, dcpl )
     CALL check("h5dcreate_f",error,total_error)

     ! /* Check on dataset's message storage status */
!!$        if(test_shared != 0) {
!!$            /* Datasets' datatypes can be shared */
!!$            ret = H5F_get_sohm_mesg_count_test(fid, H5O_DTYPE_ID, &mesg_count);
!!$            CHECK(ret, FAIL, "H5F_get_sohm_mesg_count_test");
!!$            VERIFY(mesg_count, 1, "H5F_get_sohm_mesg_count_test");
!!$
!!$            /* Datasets' dataspace can be shared */
!!$            ret = H5F_get_sohm_mesg_count_test(fid, H5O_SDSPACE_ID, &mesg_count);
!!$            CHECK(ret, FAIL, "H5F_get_sohm_mesg_count_test");
!!$            VERIFY(mesg_count, 1, "H5F_get_sohm_mesg_count_test");
!!$        } /* end if */

     ! /* Retrieve limits for compact/dense attribute storage */
     CALL H5Pget_attr_phase_change_f(dcpl, max_compact, min_dense, error)
     CALL check("H5Pget_attr_phase_change_f",error,total_error)

     ! /* Close property list */
     CALL h5pclose_f(dcpl,error)
     CALL check("h5pclose_f", error, total_error)
!!$
!!$
!!$        /* Check on datasets' attribute storage status */
!!$        is_dense = H5O_is_attr_dense_test(dataset);
!!$        VERIFY(is_dense, FALSE, "H5O_is_attr_dense_test");
!!$        is_dense = H5O_is_attr_dense_test(dataset2);
!!$        VERIFY(is_dense, FALSE, "H5O_is_attr_dense_test");
     ! /* Add attributes to each dataset, until after converting to dense storage */
     
     DO u = 0, (max_compact * 2) - 1

        ! /* Create attribute name */
        WRITE(chr2,'(I2.2)') u
        attrname = 'attr '//chr2
        
        ! /* Alternate between creating "small" & "big" attributes */

        IF(MOD(u+1,2).EQ.0)THEN
           ! /* Create "small" attribute on first dataset */

           CALL h5acreate_f(dataset, attrname, attr_tid, sid, attr, error, H5P_DEFAULT_F, H5P_DEFAULT_F)
           CALL check("h5acreate_f",error,total_error)

!!$                /* Check that attribute is not shared */
!!$                is_shared = H5A_is_shared_test(attr);
!!$                VERIFY(is_shared, FALSE, "H5A_is_shared_test");

           ! /* Write data into the attribute */
           attr_integer_data(1) = u + 1
           data_dims(1) = 1
           CALL h5awrite_f(attr, attr_tid, attr_integer_data, data_dims, error)
           CALL check("h5awrite_f",error,total_error)
        ELSE
           !  Create "big" attribute on first dataset */

           CALL h5acreate_f(dataset, attrname, attr_tid, big_sid, attr, error, H5P_DEFAULT_F, H5P_DEFAULT_F)
           CALL check("h5acreate_f",error,total_error)
!!$
           !  Check that attribute is shared */
!!$                is_shared = H5A_is_shared_test(attr);
!!$                VERIFY(is_shared, TRUE, "H5A_is_shared_test");

           !  Check refcount for attribute */
!!$                ret = H5A_get_shared_rc_test(attr, &shared_refcount);
!!$                CHECK(ret, FAIL, "H5A_get_shared_rc_test");
!!$                VERIFY(shared_refcount, 1, "H5A_get_shared_rc_test");
!!$
           !  Write data into the attribute */

           attr_integer_data(1) = u + 1
           data_dims(1) = 1
           CALL h5awrite_f(attr,  attr_tid, attr_integer_data, data_dims, error)
           CALL check("h5awrite_f",error,total_error)

           !  Check refcount for attribute */
!!$                ret = H5A_get_shared_rc_test(attr, &shared_refcount);
!!$                CHECK(ret, FAIL, "H5A_get_shared_rc_test");
!!$                VERIFY(shared_refcount, 1, "H5A_get_shared_rc_test");
        ENDIF

        ! /* Close attribute */
        CALL h5aclose_f(attr, error)
        CALL check("h5aclose_f",error,total_error)

        ! /* Check on dataset's attribute storage status */
!!$            is_dense = H5O_is_attr_dense_test(dataset);
!!$            if(u < max_compact)
!!$                VERIFY(is_dense, FALSE, "H5O_is_attr_dense_test");
!!$            else
!!$                VERIFY(is_dense, TRUE, "H5O_is_attr_dense_test");
!!$
!!$
        ! /* Alternate between creating "small" & "big" attributes */
        IF(MOD(u+1,2).EQ.0)THEN

           !  /* Create "small" attribute on second dataset */

           CALL h5acreate_f(dataset2, attrname, attr_tid, sid, attr, error, H5P_DEFAULT_F, H5P_DEFAULT_F)
           CALL check("h5acreate_f",error,total_error)

           !  /* Check that attribute is not shared */
!!$                is_shared = H5A_is_shared_test(attr);
!!$                VERIFY(is_shared, FALSE, "H5A_is_shared_test");
!!$
           ! /* Write data into the attribute */
           attr_integer_data(1) = u + 1
           data_dims(1) = 1
           CALL h5awrite_f(attr, attr_tid, attr_integer_data, data_dims, error)
           CALL check("h5awrite_f",error,total_error)
        ELSE

           ! /* Create "big" attribute on second dataset */
           
           CALL h5acreate_f(dataset2, attrname, attr_tid, big_sid, attr, error, H5P_DEFAULT_F, H5P_DEFAULT_F)
           CALL check("h5acreate_f",error,total_error)

! /* Check that attribute is shared */
!!$                is_shared = H5A_is_shared_test(attr);
!!$                VERIFY(is_shared, TRUE, "H5A_is_shared_test");
!!$
! /* Check refcount for attribute */
!!$                ret = H5A_get_shared_rc_test(attr, &shared_refcount);
!!$                CHECK(ret, FAIL, "H5A_get_shared_rc_test");
!!$                VERIFY(shared_refcount, 1, "H5A_get_shared_rc_test");
!!$
! /* Write data into the attribute */

           
           attr_integer_data(1) = u + 1
           data_dims(1) = 1
           CALL h5awrite_f(attr,  attr_tid, attr_integer_data, data_dims, error)
           CALL check("h5awrite_f",error,total_error)


! /* Check refcount for attribute */
!!$                ret = H5A_get_shared_rc_test(attr, &shared_refcount);
!!$                CHECK(ret, FAIL, "H5A_get_shared_rc_test");
!!$                VERIFY(shared_refcount, 2, "H5A_get_shared_rc_test");

        ENDIF
        ! /* Close attribute */
        CALL h5aclose_f(attr, error)
        CALL check("h5aclose_f",error,total_error)

        !    /* Check on dataset's attribute storage status */
!!$            is_dense = H5O_is_attr_dense_test(dataset2);
!!$            if(u < max_compact)
!!$                VERIFY(is_dense, FALSE, "H5O_is_attr_dense_test");
!!$            else
!!$                VERIFY(is_dense, TRUE, "H5O_is_attr_dense_test");


        ! /* Create new attribute name */
        
        WRITE(chr2,'(I2.2)') u
        attrname2 = 'new attr '//chr2


        ! /* Change second dataset's attribute's name */

        CALL H5Arename_by_name_f(fid, DSET2_NAME, attrname, attrname2, H5P_DEFAULT_F, error)
        CALL check("H5Arename_by_name_f",error,total_error)

        ! /* Check refcount on attributes now */

        ! /* Check refcount on renamed attribute */

        CALL H5Aopen_f(dataset2, attrname2, H5P_DEFAULT_F, attr, error)
        CALL check("H5Aopen_f",error,total_error)

!!$
!!$        IF(MOD(u+1,2).EQ.0)THEN
!!$           ! /* Check that attribute is not shared */
!!$           is_shared = H5A_is_shared_test(attr);
!!$           CALL VERIFY("H5A_is_shared_test", error, -1)
!!$        ELSE
!!$                ! /* Check that attribute is shared */
!!$                is_shared = H5A_is_shared_test(attr);
!!$                VERIFY(is_shared, TRUE, "H5A_is_shared_test");
!!$
!!$                /* Check refcount for attribute */
!!$                ret = H5A_get_shared_rc_test(attr, &shared_refcount);
!!$                CHECK(ret, FAIL, "H5A_get_shared_rc_test");
!!$                VERIFY(shared_refcount, 1, "H5A_get_shared_rc_test")
!!$             ENDIF

        ! /* Close attribute */
        CALL h5aclose_f(attr, error)
        CALL check("h5aclose_f",error,total_error)

        ! /* Check refcount on original attribute */
        CALL H5Aopen_f(dataset, attrname, H5P_DEFAULT_F, attr, error)
        CALL check("H5Aopen",error,total_error)

!!$            if(u % 2) {
!!$                /* Check that attribute is not shared */
!!$                is_shared = H5A_is_shared_test(attr);
!!$                VERIFY(is_shared, FALSE, "H5A_is_shared_test");
!!$            } /* end if */
!!$            else {
!!$                /* Check that attribute is shared */
!!$                is_shared = H5A_is_shared_test(attr);
!!$                VERIFY(is_shared, TRUE, "H5A_is_shared_test");
!!$
!!$                /* Check refcount for attribute */
!!$                ret = H5A_get_shared_rc_test(attr, &shared_refcount);
!!$                CHECK(ret, FAIL, "H5A_get_shared_rc_test");
!!$                VERIFY(shared_refcount, 1, "H5A_get_shared_rc_test");
!!$            } /* end else */

        ! /* Close attribute */
        CALL h5aclose_f(attr, error)
        CALL check("h5aclose_f",error,total_error)


        ! /* Change second dataset's attribute's name back to original */
        
        CALL H5Arename_by_name_f(fid, DSET2_NAME, attrname2, attrname, H5P_DEFAULT_F, error)
        CALL check("H5Arename_by_name_f",error,total_error)

        ! /* Check refcount on attributes now */

        ! /* Check refcount on renamed attribute */
        CALL H5Aopen_f(dataset2, attrname, H5P_DEFAULT_F, attr, error)
        CALL check("H5Aopen",error,total_error)
!!$
!!$            if(u % 2) {
!!$                /* Check that attribute is not shared */
!!$                is_shared = H5A_is_shared_test(attr);
!!$                VERIFY(is_shared, FALSE, "H5A_is_shared_test");
!!$            } /* end if */
!!$            else {
!!$                /* Check that attribute is shared */
!!$                is_shared = H5A_is_shared_test(attr);
!!$                VERIFY(is_shared, TRUE, "H5A_is_shared_test");
!!$
!!$                /* Check refcount for attribute */
!!$                ret = H5A_get_shared_rc_test(attr, &shared_refcount);
!!$                CHECK(ret, FAIL, "H5A_get_shared_rc_test");
!!$                VERIFY(shared_refcount, 2, "H5A_get_shared_rc_test");
!!$            } /* end else */

        ! /* Close attribute */
        CALL h5aclose_f(attr, error)
        CALL check("h5aclose_f",error,total_error)

        ! /* Check refcount on original attribute */

        ! /* Check refcount on renamed attribute */
        CALL H5Aopen_f(dataset, attrname, H5P_DEFAULT_F, attr, error)
        CALL check("H5Aopen",error,total_error)

!!$            if(u % 2) {
!!$                /* Check that attribute is not shared */
!!$                is_shared = H5A_is_shared_test(attr);
!!$                VERIFY(is_shared, FALSE, "H5A_is_shared_test");
!!$            } /* end if */
!!$            else {
!!$                /* Check that attribute is shared */
!!$                is_shared = H5A_is_shared_test(attr);
!!$                VERIFY(is_shared, TRUE, "H5A_is_shared_test");
!!$
!!$                /* Check refcount for attribute */
!!$                ret = H5A_get_shared_rc_test(attr, &shared_refcount);
!!$                CHECK(ret, FAIL, "H5A_get_shared_rc_test");
!!$                VERIFY(shared_refcount, 2, "H5A_get_shared_rc_test");
!!$            } /* end else */

        ! /* Close attribute */
        CALL h5aclose_f(attr, error)
        CALL check("h5aclose_f",error,total_error)
        
     ENDDO

     ! /* Close attribute's datatype */
     CALL h5tclose_f(attr_tid, error)
     CALL check("h5tclose_f",error,total_error)

     ! /* Close attribute's datatype */
     CALL h5dclose_f(dataset, error)
     CALL check("h5dclose_f",error,total_error)
     CALL h5dclose_f(dataset2, error)
     CALL check("h5dclose_f",error,total_error)

!!$        /* Check on shared message status now */
!!$        if(test_shared != 0) {
!!$            if(test_shared == 1) {
!!$                /* Check on datatype storage status */
!!$                ret = H5F_get_sohm_mesg_count_test(fid, H5O_DTYPE_ID, &mesg_count);
!!$                CHECK(ret, FAIL, "H5F_get_sohm_mesg_count_test");
!!$                VERIFY(mesg_count, 2, "H5F_get_sohm_mesg_count_test");
!!$            } /* end if */
!!$
!!$            /* Check on dataspace storage status */
!!$            ret = H5F_get_sohm_mesg_count_test(fid, H5O_SDSPACE_ID, &mesg_count);
!!$            CHECK(ret, FAIL, "H5F_get_sohm_mesg_count_test");
!!$            VERIFY(mesg_count, 2, "H5F_get_sohm_mesg_count_test");
!!$        } /* end if */

     ! /* Unlink datasets with attributes */
     CALL H5Ldelete_f(fid, DSET1_NAME, H5P_DEFAULT_F, error)
     CALL check("HLdelete",error,total_error)
     CALL H5Ldelete_f(fid, DSET2_NAME, H5P_DEFAULT_F, error)
     CALL check("HLdelete",error,total_error)

     !/* Unlink committed datatype */
     IF(test_shared == 2)THEN
        CALL H5Ldelete_f(fid, TYPE1_NAME, H5P_DEFAULT_F, error)
        CALL check("HLdelete_f",error,total_error)
     ENDIF

     ! /* Check on attribute storage status */
!!$        ret = H5F_get_sohm_mesg_count_test(fid, H5O_ATTR_ID, &mesg_count);
!!$        CHECK(ret, FAIL, "H5F_get_sohm_mesg_count_test");
!!$        VERIFY(mesg_count, 0, "H5F_get_sohm_mesg_count_test");
!!$
!!$        if(test_shared != 0) {
!!$            /* Check on datatype storage status */
!!$            ret = H5F_get_sohm_mesg_count_test(fid, H5O_DTYPE_ID, &mesg_count);
!!$            CHECK(ret, FAIL, "H5F_get_sohm_mesg_count_test");
!!$            VERIFY(mesg_count, 0, "H5F_get_sohm_mesg_count_test");
!!$
!!$            /* Check on dataspace storage status */
!!$            ret = H5F_get_sohm_mesg_count_test(fid, H5O_SDSPACE_ID, &mesg_count);
!!$            CHECK(ret, FAIL, "H5F_get_sohm_mesg_count_test");
!!$            VERIFY(mesg_count, 0, "H5F_get_sohm_mesg_count_test");
!!$        } /* end if */

     ! /* Close file */
     CALL h5fclose_f(fid, error)
     CALL check("h5fclose_f",error,total_error)

     ! /* Check size of file */
     !filesize = h5_get_file_size(FILENAME);
     !VERIFY(filesize, empty_filesize, "h5_get_file_size");
  ENDDO

  ! /* Close dataspaces */
  CALL h5sclose_f(sid, error)
  CALL check("h5sclose_f",error,total_error)
  CALL h5sclose_f(big_sid, error)
  CALL check("h5sclose_f",error,total_error)

END SUBROUTINE test_attr_shared_rename


SUBROUTINE test_attr_delete_by_idx(new_format, fcpl, fapl, total_error)

  USE HDF5
  
  IMPLICIT NONE

  LOGICAL, INTENT(IN) :: new_format
  INTEGER(HID_T), INTENT(IN) :: fcpl
  INTEGER(HID_T), INTENT(IN) :: fapl
  INTEGER, INTENT(INOUT) :: total_error
  CHARACTER(LEN=8) :: FileName = "tattr.h5"
  INTEGER(HID_T) :: fid
  INTEGER(HID_T) :: dcpl
  INTEGER(HID_T) :: sid
  INTEGER(HID_T) :: dataset

  CHARACTER(LEN=8) :: DSET1_NAME = "Dataset1"
  CHARACTER(LEN=8) :: DSET2_NAME = "Dataset2"
  CHARACTER(LEN=8) :: DSET3_NAME = "Dataset3"
  INTEGER, PARAMETER :: NUM_DSETS = 3

  INTEGER :: curr_dset

  INTEGER(HID_T) :: dset1, dset2, dset3
  INTEGER(HID_T) :: my_dataset

  INTEGER :: error

  INTEGER :: value
  INTEGER(HID_T) :: attr        !String Attribute identifier 
  INTEGER(HID_T) :: attr_sid
  INTEGER(HSIZE_T), DIMENSION(7) :: data_dims
  INTEGER(HSIZE_T) :: storage_size   ! attributes storage requirements .MSB.

  LOGICAL :: f_corder_valid ! Indicates whether the the creation order data is valid for this attribute 
  INTEGER :: corder ! Is a positive integer containing the creation order of the attribute
  INTEGER :: cset ! Indicates the character set used for the attribute’s name
  INTEGER(HSIZE_T) :: data_size   ! indicates the size, in the number of characters
  INTEGER(HSIZE_T) :: n
  LOGICAL, DIMENSION(1:2) :: use_index = (/.FALSE.,.TRUE./)

  INTEGER :: max_compact ! Maximum # of links to store in group compactly
  INTEGER :: min_dense   ! Minimum # of links to store in group "densely"

  CHARACTER(LEN=2) :: chr2

  INTEGER :: i, j

  INTEGER, DIMENSION(1) ::  attr_integer_data
  CHARACTER(LEN=7) :: attrname

  INTEGER(SIZE_T) :: size
  CHARACTER(LEN=7) :: tmpname
  CHARACTER(LEN=1), PARAMETER :: chr1 = '.'
  
  INTEGER :: idx_type
  INTEGER :: order
  INTEGER :: u 
  INTEGER :: Input1
  
  data_dims = 0

!!$test_attr_delete_by_idx(hbool_t new_format, hid_t fcpl, hid_t fapl)
!!${
!!$    hid_t	fid;		/* HDF5 File ID			*/
!!$    hid_t	dset1, dset2, dset3;	/* Dataset IDs			*/
!!$    hid_t	my_dataset;	/* Current dataset ID		*/
!!$    hid_t	sid;	        /* Dataspace ID			*/
!!$    hid_t	attr;	        /* Attribute ID			*/
!!$    hid_t	dcpl;	        /* Dataset creation property list ID */
!!$    H5A_info_t  ainfo;          /* Attribute information */
!!$    unsigned    max_compact;    /* Maximum # of links to store in group compactly */
!!$    unsigned    min_dense;      /* Minimum # of links to store in group "densely" */
!!$    htri_t	is_empty;	/* Are there any attributes? */
!!$    htri_t	is_dense;	/* Are attributes stored densely? */
!!$    hsize_t     nattrs;         /* Number of attributes on object */
!!$    hsize_t     name_count;     /* # of records in name index */
!!$    hsize_t     corder_count;   /* # of records in creation order index */
!!$    H5_index_t idx_type;        /* Type of index to operate on */
!!$    H5_iter_order_t order;      /* Order within in the index */
!!$    hbool_t     use_index;      /* Use index on creation order values */
!!$    char	attrname[NAME_BUF_SIZE];    /* Name of attribute */
!!$    char        tmpname[NAME_BUF_SIZE];     /* Temporary attribute name */
!!$    unsigned    curr_dset;      /* Current dataset to work on */
!!$    unsigned    u;              /* Local index variable */
!!$    herr_t	ret;		/* Generic return value		*/
!!$

  ! /* Create dataspace for dataset & attributes */
  CALL h5screate_f(H5S_SCALAR_F, sid, error)
  CALL check("h5screate_f",error,total_error)

  !    /* Create dataset creation property list */
  CALL H5Pcreate_f(H5P_DATASET_CREATE_F,dcpl,error)
  CALL check("h5Pcreate_f",error,total_error)

  ! /* Query the attribute creation properties */
  CALL H5Pget_attr_phase_change_f(dcpl, max_compact, min_dense, error)
  CALL check("H5Pget_attr_phase_change_f",error,total_error)

  !/* Loop over operating on different indices on link fields */
  DO idx_type = H5_INDEX_NAME_F, H5_INDEX_CRT_ORDER_F

     ! /* Loop over operating in different orders */
     DO order = H5_ITER_INC_F, H5_ITER_DEC_F
        
        ! /* Loop over using index for creation order value */
        DO i = 1, 2
           
           ! /* Print appropriate test message */
           IF(idx_type .EQ. H5_INDEX_CRT_ORDER_F)THEN
              IF(order .EQ. H5_ITER_INC_F) THEN
                 IF(use_index(i))THEN
                    WRITE(*,'(A93)')"Testing Deleting Attribute By Creation Order Index in Increasing Order w/Creation Order Index"
                 ELSE
                    WRITE(*,'(A95)')"Testing Deleting Attribute By Creation Order Index in Increasing Order w/o Creation Order Index"
                 ENDIF
              ELSE
                 IF(use_index(i))THEN
                    WRITE(*,'(A93)')"Testing Deleting Attribute By Creation Order Index in Decreasing Order w/Creation Order Index"
                 ELSE
                    WRITE(*,'(A95)')"Testing Deleting Attribute By Creation Order Index in Decreasing Order w/o Creation Order Index"
                 ENDIF
              ENDIF
           ELSE
              IF(order .EQ. H5_ITER_INC_F)THEN
                 IF(use_index(i))THEN
                    WRITE(*,'(A83)')"Testing Deleting Attribute By Name Index in Increasing Order w/Creation Order Index"
                 ELSE
                    WRITE(*,'(A85)')"Testing Deleting Attribute By Name Index in Increasing Order w/o Creation Order Index"
                 ENDIF
              ELSE
                 IF(use_index(i))THEN
                    WRITE(*,'(A84)') "Testing Deleting Attribute By Name Index in Decreasing Order w/Creation Order Index"
                 ELSE
                    WRITE(*,'(A86)') "Testing Deleting Attribute By Name Index in Decreasing Order w/o Creation Order Index"
                 ENDIF
              ENDIF
           ENDIF

           ! /* Create file */
           CALL h5fcreate_f(FileName, H5F_ACC_TRUNC_F, fid, error, fcpl, fapl)
           CALL check("h5fcreate_f",error,total_error)

           !  /* Set attribute creation order tracking & indexing for object */
           IF(new_format)THEN

              IF(use_index(i))THEN
                 Input1 = H5P_CRT_ORDER_INDEXED_F
              ELSE
                 Input1 = 0
              ENDIF

              CALL H5Pset_attr_creation_order_f(dcpl, IOR(H5P_CRT_ORDER_TRACKED_F, Input1), error)
              CALL check("H5Pset_attr_creation_order",error,total_error)

           ENDIF

           ! /* Create datasets */
           
           CALL h5dcreate_f(fid, DSET1_NAME, H5T_NATIVE_CHARACTER, sid, dset1, error, dcpl )
           CALL check("h5dcreate_f2",error,total_error)
           
           CALL h5dcreate_f(fid, DSET2_NAME, H5T_NATIVE_CHARACTER, sid, dset2, error, dcpl )
           CALL check("h5dcreate_f3",error,total_error)
           
           CALL h5dcreate_f(fid, DSET3_NAME, H5T_NATIVE_CHARACTER, sid, dset3, error, dcpl )
           CALL check("h5dcreate_f4",error,total_error)
           
           !   /* Work on all the datasets */
           
           DO curr_dset = 0,NUM_DSETS-1
              SELECT CASE (curr_dset)
              CASE (0)
                 my_dataset = dset1
              CASE (1)
                 my_dataset = dset2
              CASE (2)
                 my_dataset = dset3
                 !     CASE DEFAULT
                 !        CALL HDassert(0.AND."Toomanydatasets!")
              END SELECT
              
              ! /* Check on dataset's attribute storage status */
!!$                is_empty = H5O_is_attr_empty_test(my_dataset);
!!$                VERIFY(is_empty, TRUE, "H5O_is_attr_empty_test");
!!$                is_dense = H5O_is_attr_dense_test(my_dataset);
!!$                VERIFY(is_dense, FALSE, "H5O_is_attr_dense_test");
              
              ! /* Check for deleting non-existant attribute */
              CALL H5Adelete_by_idx_f(my_dataset, '.', idx_type, order, 0_HSIZE_T, H5P_DEFAULT_F,error)
              CALL VERIFY("H5Adelete_by_idx_f",error,-1,total_error)
              
              !    /* Create attributes, up to limit of compact form */
              DO u = 0, max_compact - 1
                 ! /* Create attribute */
                 WRITE(chr2,'(I2.2)') u
                 attrname = 'attr '//chr2
                 
                 CALL h5acreate_f(my_dataset, attrname, H5T_NATIVE_INTEGER, sid, attr, error, H5P_DEFAULT_F, H5P_DEFAULT_F)
                 CALL check("h5acreate_f",error,total_error)
                 
                 ! /* Write data into the attribute */
                 attr_integer_data(1) = u
                 data_dims(1) = 1
                 CALL h5awrite_f(attr, H5T_NATIVE_INTEGER, attr_integer_data, data_dims, error)
                 CALL check("h5awrite_f",error,total_error)
                 
                 ! /* Close attribute */
                 CALL h5aclose_f(attr, error)
                 CALL check("h5aclose_f",error,total_error)

                 ! /* Verify information for new attribute */
                 CALL attr_info_by_idx_check(my_dataset, attrname, INT(u,HSIZE_T), use_index(i), total_error )
                 
              ENDDO


              
              !  /* Verify state of object */

!!$                ret = H5O_num_attrs_test(my_dataset, &nattrs);
!!$                CHECK(ret, FAIL, "H5O_num_attrs_test");
!!$                VERIFY(nattrs, max_compact, "H5O_num_attrs_test");
!!$                is_empty = H5O_is_attr_empty_test(my_dataset);
!!$                VERIFY(is_empty, FALSE, "H5O_is_attr_empty_test");
!!$                is_dense = H5O_is_attr_dense_test(my_dataset);
!!$                VERIFY(is_dense, FALSE, "H5O_is_attr_dense_test");

              !/* Check for out of bound deletions */
              CALL H5Adelete_by_idx_f(my_dataset, ".", idx_type, order, INT(u,HSIZE_T), H5P_DEFAULT_F, error)
              CALL VERIFY("H5Adelete_by_idx_f",error,-1,total_error)
               
           ENDDO


           DO curr_dset = 0, NUM_DSETS-1
              SELECT CASE (curr_dset)
              CASE (0)
                 my_dataset = dset1
              CASE (1)
                 my_dataset = dset2
              CASE (2)
                 my_dataset = dset3
                 !     CASE DEFAULT
                 !        CALL HDassert(0.AND."Toomanydatasets!")
              END SELECT
              
              ! /* Delete attributes from compact storage */
              
              DO u = 0, max_compact - 2
                 
                 ! /* Delete first attribute in appropriate order */
                 
                 
                 CALL H5Adelete_by_idx_f(my_dataset, ".", idx_type, order, 0_HSIZE_T, H5P_DEFAULT_F, error)
                 CALL check("H5Adelete_by_idx_f",error,total_error)
                 

                 ! /* Verify the attribute information for first attribute in appropriate order */
                 ! HDmemset(&ainfo, 0, sizeof(ainfo));

                 CALL h5aget_info_by_idx_f(my_dataset, ".", idx_type, order, 0_HSIZE_T, H5P_DEFAULT_F, &
                      f_corder_valid, corder, cset, data_size, error)
                 
                 IF(new_format)THEN
                    IF(order.EQ.H5_ITER_INC_F)THEN
                       CALL VERIFY("H5Aget_info_by_idx_f",corder,u + 1,total_error)
                    ENDIF
                 ELSE
                    CALL VERIFY("H5Aget_info_by_idx_f",corder, max_compact-(u + 2),total_error)
                 ENDIF
                 
                   ! /* Verify the name for first attribute in appropriate order */
                   ! HDmemset(tmpname, 0, (size_t)NAME_BUF_SIZE);

                 size = 80 ! CHECK IF TO SMALL
                 CALL h5aget_name_by_idx_f(my_dataset, ".", idx_type, order,INT(0,hsize_t), &
                      tmpname, size, H5P_DEFAULT_F, error)

                 IF(order .EQ. H5_ITER_INC_F)THEN
                    WRITE(chr2,'(I2.2)') u + 1
                    attrname = 'attr '//chr2
                 ELSE
                    WRITE(chr2,'(I2.2)') max_compact - (u + 2)
                    attrname = 'attr '//chr2
                 ENDIF
                 IF(TRIM(attrname).NE.TRIM(tmpname)) error = -1
                 CALL VERIFY("h5aget_name_by_idx_f",error,0,total_error)
              ENDDO

              ! /* Delete last attribute */

              CALL H5Adelete_by_idx_f(my_dataset, ".", idx_type, order, 0_HSIZE_T, H5P_DEFAULT_F, error)
              CALL check("H5Adelete_by_idx_f",error,total_error)

                
              ! /* Verify state of attribute storage (empty) */
!!$                is_empty = H5O_is_attr_empty_test(my_dataset);
!!$                VERIFY(is_empty, TRUE, "H5O_is_attr_empty_test");
           ENDDO

!   /* Work on all the datasets */
           
           DO curr_dset = 0,NUM_DSETS-1
              SELECT CASE (curr_dset)
              CASE (0)
                 my_dataset = dset1
              CASE (1)
                 my_dataset = dset2
              CASE (2)
                 my_dataset = dset3
                 !     CASE DEFAULT
                 !        CALL HDassert(0.AND."Toomanydatasets!")
              END SELECT

              ! /* Create more attributes, to push into dense form */

              DO u = 0, (max_compact * 2) - 1

                 ! /* Create attribute */
                 WRITE(chr2,'(I2.2)') u
                 attrname = 'attr '//chr2
                 
                 CALL h5acreate_f(my_dataset, attrname, H5T_NATIVE_INTEGER, sid, attr, error, H5P_DEFAULT_F, H5P_DEFAULT_F)
                 CALL check("h5acreate_f",error,total_error)


                 ! /* Write data into the attribute */
                 attr_integer_data(1) = u
                 data_dims(1) = 1
                 CALL h5awrite_f(attr, H5T_NATIVE_INTEGER, attr_integer_data, data_dims, error)
                 CALL check("h5awrite_f",error,total_error)

                 ! /* Close attribute */
                 CALL h5aclose_f(attr, error)
                 CALL check("h5aclose_f",error,total_error)

                 ! /* Verify state of object */
                 IF(u .GE. max_compact)THEN
!!$                  is_dense = H5O_is_attr_dense_test(my_dataset);
!!$                  VERIFY(is_dense, (new_format ? TRUE : FALSE), "H5O_is_attr_dense_test");
                 ENDIF

                 ! /* Verify information for new attribute */
!!$              CALL check("attr_info_by_idx_check",error,total_error)
              ENDDO

              ! /* Verify state of object */
!!$                    ret = H5O_num_attrs_test(my_dataset, &nattrs);
!!$                    CHECK(ret, FAIL, "H5O_num_attrs_test");
!!$                    VERIFY(nattrs, (max_compact * 2), "H5O_num_attrs_test");
!!$                    is_empty = H5O_is_attr_empty_test(my_dataset);
!!$                    VERIFY(is_empty, FALSE, "H5O_is_attr_empty_test");
!!$                    is_dense = H5O_is_attr_dense_test(my_dataset);
!!$                    VERIFY(is_dense, (new_format ? TRUE : FALSE), "H5O_is_attr_dense_test");
!!$
              IF(new_format)THEN
!!$                 ! /* Retrieve & verify # of records in the name & creation order indices */
!!$                 ret = H5O_attr_dense_info_test(my_dataset, &name_count, &corder_count);
!!$                 CHECK(ret, FAIL, "H5O_attr_dense_info_test");
!!$                 IF(use_index)
!!$                 VERIFY(name_count, corder_count, "H5O_attr_dense_info_test");
!!$                 VERIFY(name_count, (max_compact * 2), "H5O_attr_dense_info_test");
              ENDIF

              ! /* Check for out of bound deletion */
              CALL H5Adelete_by_idx_f(my_dataset, ".", idx_type, order, INT(u,HSIZE_T), H5P_DEFAULT_F, error)
              CALL VERIFY("H5Adelete_by_idx_f",error,-1,total_error)
           ENDDO

           ! /* Work on all the datasets */

           DO curr_dset = 0,NUM_DSETS-1
              SELECT CASE (curr_dset)
              CASE (0)
                 my_dataset = dset1
              CASE (1)
                 my_dataset = dset2
              CASE (2)
                 my_dataset = dset3
                 !     CASE DEFAULT
                 !        CALL HDassert(0.AND."Toomanydatasets!")
              END SELECT

              ! /* Delete attributes from dense storage */

              DO u = 0, (max_compact * 2) - 1 - 1

                 ! /* Delete first attribute in appropriate order */

                 CALL H5Adelete_by_idx_f(my_dataset, ".", idx_type, order, INT(0,HSIZE_T), H5P_DEFAULT_F, error)
                 CALL check("H5Adelete_by_idx_f",error,total_error)


                 ! /* Verify the attribute information for first attribute in appropriate order */
!!$                        HDmemset(&ainfo, 0, sizeof(ainfo));

                 
                 CALL h5aget_info_by_idx_f(my_dataset, ".", idx_type, order, INT(0,HSIZE_T), H5P_DEFAULT_F, &
                      f_corder_valid, corder, cset, data_size, error)

                                  
                 IF(new_format)THEN
                    IF(order.EQ.H5_ITER_INC_F)THEN
                       CALL VERIFY("H5Aget_info_by_idx_f",corder,u + 1,total_error)
                    ENDIF
                 ELSE
                    CALL VERIFY("H5Aget_info_by_idx_f",corder, ((max_compact * 2) - (u + 2)), total_error)
                 ENDIF


                 ! /* Verify the name for first attribute in appropriate order */
                 ! HDmemset(tmpname, 0, (size_t)NAME_BUF_SIZE);

                 size = 80 ! CHECK IF TO SMALL
                 CALL h5aget_name_by_idx_f(my_dataset, ".", idx_type, order,INT(0,hsize_t), &
                      tmpname, size, H5P_DEFAULT_F, error)

                 IF(order .EQ. H5_ITER_INC_F)THEN
                    WRITE(chr2,'(I2.2)') u + 1
                    attrname = 'attr '//chr2
                 ELSE
                    WRITE(chr2,'(I2.2)') max_compact * 2 - (u + 2) 
                    attrname = 'attr '//chr2
                 ENDIF
                 IF(TRIM(attrname).NE.TRIM(tmpname)) error = -1
                 CALL VERIFY("h5aget_name_by_idx_f",error,0,total_error)
                 

              ENDDO
              ! /* Delete last attribute */

              CALL H5Adelete_by_idx_f(my_dataset, ".", idx_type, order, INT(0,HSIZE_T), H5P_DEFAULT_F, error)
              CALL check("H5Adelete_by_idx_f",error,total_error)
              ! /* Verify state of attribute storage (empty) */
!!$                    is_empty = H5O_is_attr_empty_test(my_dataset);
!!$                    VERIFY(is_empty, TRUE, "H5O_is_attr_empty_test");

              !/* Check for deletion on empty attribute storage again */
              CALL H5Adelete_by_idx_f(my_dataset, ".", idx_type, order, INT(0,HSIZE_T), H5P_DEFAULT_F, error)
              CALL VERIFY("H5Adelete_by_idx_fa",error,-1,total_error)
           ENDDO

           
!!$
!!$
!!$                    /* Delete attributes in middle */
!!$
!!$
!!$                /* Work on all the datasets */
!!$                for(curr_dset = 0; curr_dset < NUM_DSETS; curr_dset++) {
!!$                    switch(curr_dset) {
!!$                        case 0:
!!$                            my_dataset = dset1;
!!$                            break;
!!$
!!$                        case 1:
!!$                            my_dataset = dset2;
!!$                            break;
!!$
!!$                        case 2:
!!$                            my_dataset = dset3;
!!$                            break;
!!$
!!$                        default:
!!$                            HDassert(0 && "Too many datasets!");
!!$                    } /* end switch */
!!$
!!$                    /* Create attributes, to push into dense form */
!!$                    for(u = 0; u < (max_compact * 2); u++) {
!!$                        /* Create attribute */
!!$                        sprintf(attrname, "attr %02u", u);
!!$                        attr = H5Acreate2(my_dataset, attrname, H5T_NATIVE_UINT, sid, H5P_DEFAULT, H5P_DEFAULT);
!!$                        CHECK(attr, FAIL, "H5Acreate2");
!!$
!!$                        /* Write data into the attribute */
!!$                        ret = H5Awrite(attr, H5T_NATIVE_UINT, &u);
!!$                        CHECK(ret, FAIL, "H5Awrite");
!!$
!!$                        /* Close attribute */
!!$                        ret = H5Aclose(attr);
!!$                        CHECK(ret, FAIL, "H5Aclose");
!!$
!!$                        /* Verify state of object */
!!$                        if(u >= max_compact) {
!!$                            is_dense = H5O_is_attr_dense_test(my_dataset);
!!$                            VERIFY(is_dense, (new_format ? TRUE : FALSE), "H5O_is_attr_dense_test");
!!$                        } /* end if */
!!$
!!$                        /* Verify information for new attribute */
!!$                        ret = attr_info_by_idx_check(my_dataset, attrname, (hsize_t)u, use_index);
!!$                        CHECK(ret, FAIL, "attr_info_by_idx_check");
!!$                    } /* end for */
!!$                } /* end for */
!!$
!!$                /* Work on all the datasets */
!!$                for(curr_dset = 0; curr_dset < NUM_DSETS; curr_dset++) {
!!$                    switch(curr_dset) {
!!$                        case 0:
!!$                            my_dataset = dset1;
!!$                            break;
!!$
!!$                        case 1:
!!$                            my_dataset = dset2;
!!$                            break;
!!$
!!$                        case 2:
!!$                            my_dataset = dset3;
!!$                            break;
!!$
!!$                        default:
!!$                            HDassert(0 && "Too many datasets!");
!!$                    } /* end switch */
!!$
!!$                    /* Delete every other attribute from dense storage, in appropriate order */
!!$                    for(u = 0; u < max_compact; u++) {
!!$                        /* Delete attribute */
!!$                        ret = H5Adelete_by_idx(my_dataset, ".", idx_type, order, (hsize_t)u, H5P_DEFAULT);
!!$                        CHECK(ret, FAIL, "H5Adelete_by_idx");
!!$
!!$                        /* Verify the attribute information for first attribute in appropriate order */
!!$                        HDmemset(&ainfo, 0, sizeof(ainfo));
!!$                        ret = H5Aget_info_by_idx(my_dataset, ".", idx_type, order, (hsize_t)u, &ainfo, H5P_DEFAULT);
!!$                        if(new_format) {
!!$                            if(order == H5_ITER_INC) {
!!$                                VERIFY(ainfo.corder, ((u * 2) + 1), "H5Aget_info_by_idx");
!!$                            } /* end if */
!!$                            else {
!!$                                VERIFY(ainfo.corder, ((max_compact * 2) - ((u * 2) + 2)), "H5Aget_info_by_idx");
!!$                            } /* end else */
!!$                        } /* end if */
!!$
!!$                        /* Verify the name for first attribute in appropriate order */
!!$                        HDmemset(tmpname, 0, (size_t)NAME_BUF_SIZE);
!!$                        ret = H5Aget_name_by_idx(my_dataset, ".", idx_type, order, (hsize_t)u, tmpname, (size_t)NAME_BUF_SIZE, H5P_DEFAULT);
!!$                        if(order == H5_ITER_INC)
!!$                            sprintf(attrname, "attr %02u", ((u * 2) + 1));
!!$                        else
!!$                            sprintf(attrname, "attr %02u", ((max_compact * 2) - ((u * 2) + 2)));
!!$                        ret = HDstrcmp(attrname, tmpname);
!!$                        VERIFY(ret, 0, "H5Aget_name_by_idx");
!!$                    } /* end for */
!!$                } /* end for */
!!$
!!$                /* Work on all the datasets */
!!$                for(curr_dset = 0; curr_dset < NUM_DSETS; curr_dset++) {
!!$                    switch(curr_dset) {
!!$                        case 0:
!!$                            my_dataset = dset1;
!!$                            break;
!!$
!!$                        case 1:
!!$                            my_dataset = dset2;
!!$                            break;
!!$
!!$                        case 2:
!!$                            my_dataset = dset3;
!!$                            break;
!!$
!!$                        default:
!!$                            HDassert(0 && "Too many datasets!");
!!$                    } /* end switch */
!!$
!!$                    /* Delete remaining attributes from dense storage, in appropriate order */
!!$                    for(u = 0; u < (max_compact - 1); u++) {
!!$                        /* Delete attribute */
!!$                        ret = H5Adelete_by_idx(my_dataset, ".", idx_type, order, (hsize_t)0, H5P_DEFAULT);
!!$                        CHECK(ret, FAIL, "H5Adelete_by_idx");
!!$
!!$                        /* Verify the attribute information for first attribute in appropriate order */
!!$                        HDmemset(&ainfo, 0, sizeof(ainfo));
!!$                        ret = H5Aget_info_by_idx(my_dataset, ".", idx_type, order, (hsize_t)0, &ainfo, H5P_DEFAULT);
!!$                        if(new_format) {
!!$                            if(order == H5_ITER_INC) {
!!$                                VERIFY(ainfo.corder, ((u * 2) + 3), "H5Aget_info_by_idx");
!!$                            } /* end if */
!!$                            else {
!!$                                VERIFY(ainfo.corder, ((max_compact * 2) - ((u * 2) + 4)), "H5Aget_info_by_idx");
!!$                            } /* end else */
!!$                        } /* end if */
!!$
!!$                        /* Verify the name for first attribute in appropriate order */
!!$                        HDmemset(tmpname, 0, (size_t)NAME_BUF_SIZE);
!!$                        ret = H5Aget_name_by_idx(my_dataset, ".", idx_type, order, (hsize_t)0, tmpname, (size_t)NAME_BUF_SIZE, H5P_DEFAULT);
!!$                        if(order == H5_ITER_INC)
!!$                            sprintf(attrname, "attr %02u", ((u * 2) + 3));
!!$                        else
!!$                            sprintf(attrname, "attr %02u", ((max_compact * 2) - ((u * 2) + 4)));
!!$                        ret = HDstrcmp(attrname, tmpname);
!!$                        VERIFY(ret, 0, "H5Aget_name_by_idx");
!!$                    } /* end for */
!!$
!!$                    /* Delete last attribute */
!!$                    ret = H5Adelete_by_idx(my_dataset, ".", idx_type, order, (hsize_t)0, H5P_DEFAULT);
!!$                    CHECK(ret, FAIL, "H5Adelete_by_idx");
!!$
!!$                    /* Verify state of attribute storage (empty) */
!!$                    is_empty = H5O_is_attr_empty_test(my_dataset);
!!$                    VERIFY(is_empty, TRUE, "H5O_is_attr_empty_test");
!!$
!!$                    /* Check for deletion on empty attribute storage again */
!!$                    ret = H5Adelete_by_idx(my_dataset, ".", idx_type, order, (hsize_t)0, H5P_DEFAULT);
!!$                    VERIFY(ret, FAIL, "H5Adelete_by_idx");
!!$                } /* end for */

           !  /* Close Datasets */
           CALL h5dclose_f(dset1, error)
           CALL check("h5dclose_f",error,total_error)
           CALL h5dclose_f(dset2, error)
           CALL check("h5dclose_f",error,total_error)
           CALL h5dclose_f(dset3, error)
           CALL check("h5dclose_f",error,total_error)
           
           !   /* Close file */
           CALL h5fclose_f(fid, error)
           CALL check("h5fclose_f",error,total_error)
        ENDDO
     ENDDO
  ENDDO

  ! /* Close property list */
  CALL h5pclose_f(dcpl,error)
  CALL check("h5pclose_f", error, total_error)

  ! /* Close dataspace */
  CALL h5sclose_f(sid, error)
  CALL check("h5sclose_f",error,total_error)

END SUBROUTINE test_attr_delete_by_idx

SUBROUTINE test_attr_shared_delete(fcpl, fapl, total_error)

!/****************************************************************
!**
!**  test_attr_shared_delete(): Test basic H5A (attribute) code.
!**      Tests deleting shared attributes in "compact" & "dense" storage
!**
!****************************************************************/

  USE HDF5
  
  IMPLICIT NONE

  INTEGER(HID_T), INTENT(IN) :: fcpl
  INTEGER(HID_T), INTENT(IN) :: fapl
  INTEGER, INTENT(INOUT) :: total_error
  CHARACTER(LEN=8) :: FileName = "tattr.h5"
  INTEGER(HID_T) :: fid
  INTEGER(HID_T) :: dcpl
  INTEGER(HID_T) :: sid, big_sid

  CHARACTER(LEN=8) :: DSET1_NAME = "Dataset1"
  CHARACTER(LEN=8) :: DSET2_NAME = "Dataset2"
  CHARACTER(LEN=8) :: DSET3_NAME = "Dataset3"
  INTEGER, PARAMETER :: NUM_DSETS = 3

  INTEGER :: curr_dset

  INTEGER(HID_T) :: dset1, dset2, dset3
  INTEGER(HID_T) :: dataset, dataset2
  INTEGER(HID_T) :: my_dataset

  INTEGER :: error

  INTEGER :: value
  INTEGER(HID_T) :: attr        !String Attribute identifier 
  INTEGER(HID_T) :: attr_sid
  INTEGER(HID_T) :: attr_tid
  INTEGER(HSIZE_T), DIMENSION(7) :: data_dims
  INTEGER(HSIZE_T) :: storage_size   ! attributes storage requirements .MSB.

  LOGICAL :: f_corder_valid ! Indicates whether the the creation order data is valid for this attribute 
  INTEGER :: corder ! Is a positive integer containing the creation order of the attribute
  INTEGER :: cset ! Indicates the character set used for the attribute’s name
  INTEGER(HSIZE_T) :: data_size   ! indicates the size, in the number of characters
  INTEGER(HSIZE_T) :: n
  LOGICAL, DIMENSION(1:2) :: use_index = (/.FALSE.,.TRUE./)

  INTEGER :: max_compact ! Maximum # of links to store in group compactly
  INTEGER :: min_dense   ! Minimum # of links to store in group "densely"

  CHARACTER(LEN=2) :: chr2

  INTEGER :: i, j

  INTEGER, DIMENSION(1) ::  attr_integer_data
  CHARACTER(LEN=7) :: attrname

  INTEGER(SIZE_T) :: size
  CHARACTER(LEN=7) :: tmpname
  CHARACTER(LEN=1), PARAMETER :: chr1 = '.'
  
  INTEGER :: idx_type
  INTEGER :: order
  INTEGER :: u 
  INTEGER :: Input1
  INTEGER, PARAMETER :: SPACE1_RANK = 3
  INTEGER, PARAMETER :: NX = 20
  INTEGER, PARAMETER :: NY = 5
  INTEGER, PARAMETER :: NZ = 10
  INTEGER(HSIZE_T), DIMENSION(3) :: big_dims = (/NX,NY,NZ/)
  INTEGER(HID_T) :: my_fcpl

  CHARACTER(LEN=5), PARAMETER :: TYPE1_NAME = "/Type"

  INTEGER :: test_shared

  ! /* Output message about test being performed */
  WRITE(*,*) "Testing Deleting Shared & Unshared Attributes in Compact & Dense Storage"

  ! /* Initialize "big" attribute DATA */
!!$    HDmemset(big_value, 1, sizeof(big_value));
!!$
  !    /* Create dataspace for dataset */
  CALL h5screate_f(H5S_SCALAR_F, sid, error)
  CALL check("h5screate_f",error,total_error)

  !/* Create "big" dataspace for "large" attributes */

  CALL h5screate_simple_f(SPACE1_RANK, big_dims, big_sid, error)
  CALL check("h5screate_simple_f",error,total_error)

  ! /* Loop over type of shared components */

  DO test_shared = 0, 2
     
     ! /* Make copy of file creation property list */

     CALL H5Pcopy_f(fcpl, my_fcpl, error)
     CALL check("H5Pcopy",error,total_error)

     ! /* Set up datatype for attributes */

     CALL H5Tcopy_f(H5T_NATIVE_INTEGER, attr_tid, error)
     CALL check("H5Tcopy",error,total_error)

     ! /* Special setup for each type of shared components */
     IF( test_shared .EQ. 0) THEN
        ! /* Make attributes > 500 bytes shared */
        CALL H5Pset_shared_mesg_nindexes_f(my_fcpl,1,error)
        CALL check("H5Pset_shared_mesg_nindexes_f",error, total_error)
!!$     CHECK_I(ret, "H5Pset_shared_mesg_nindexes");
        CALL H5Pset_shared_mesg_index_f(my_fcpl, 0, H5O_SHMESG_ATTR_FLAG_F, 500,error)
        CALL check(" H5Pset_shared_mesg_index_f",error, total_error)

!!$     CHECK_I(ret, "H5Pset_shared_mesg_index");
     ELSE
        ! /* Set up copy of file creation property list */
        CALL H5Pset_shared_mesg_nindexes_f(my_fcpl,3,error)
!!$
!!$     CHECK_I(ret, "H5Pset_shared_mesg_nindexes");
!!$ 
        ! /* Make attributes > 500 bytes shared */
        CALL H5Pset_shared_mesg_index_f(my_fcpl, 0, H5O_SHMESG_ATTR_FLAG_F, 500,error)
!!$     CHECK_I(ret, "H5Pset_shared_mesg_index");
!!$ 
        ! /* Make datatypes & dataspaces > 1 byte shared (i.e. all of them :-) */
        CALL H5Pset_shared_mesg_index_f(my_fcpl, 1, H5O_SHMESG_DTYPE_FLAG_F, 1,error)
!!$            CHECK_I(ret, "H5Pset_shared_mesg_index");
        CALL H5Pset_shared_mesg_index_f(my_fcpl, 2,  H5O_SHMESG_SDSPACE_FLAG_F, 1,error)
!!$            CHECK_I(ret, "H5Pset_shared_mesg_index");
     ENDIF

     ! /* Create file */
     CALL h5fcreate_f(FileName, H5F_ACC_TRUNC_F, fid, error, my_fcpl, fapl)
     CALL check("h5fcreate_f",error,total_error)

     ! /* Close FCPL copy */
     CALL h5pclose_f(my_fcpl, error)
     CALL check("h5pclose_f", error, total_error)
     ! /* Close file */
     CALL h5fclose_f(fid, error)
     CALL check("h5fclose_f",error,total_error)
!!$
!!$        /* Get size of file */
!!$        empty_filesize = h5_get_file_size(FILENAME);
!!$        if(empty_filesize < 0)
!!$            TestErrPrintf("Line %d: file size wrong!\n", __LINE__);

     ! /* Re-open file */
     CALL h5fopen_f(FileName, H5F_ACC_RDWR_F, fid, error,fapl)
     CALL check("h5open_f",error,total_error)

     ! /* Commit datatype to file */

     IF(test_shared.EQ.2) THEN
        CALL H5Tcommit_f(fid, TYPE1_NAME, attr_tid, H5P_DEFAULT_F, H5P_DEFAULT_F, H5P_DEFAULT_F,error)
        CALL check("H5Tcommit",error,total_error)
     ENDIF

     ! /* Set up to query the object creation properties */
     CALL H5Pcreate_f(H5P_DATASET_CREATE_F,dcpl,error)
     CALL check("h5Pcreate_f",error,total_error)

     ! /* Create datasets */

     CALL h5dcreate_f(fid, DSET1_NAME, H5T_NATIVE_CHARACTER, sid, dataset, error, dcpl )
     CALL check("h5dcreate_f",error,total_error)
           
     CALL h5dcreate_f(fid, DSET2_NAME, H5T_NATIVE_CHARACTER, sid, dataset2, error, dcpl )
     CALL check("h5dcreate_f",error,total_error)

     ! /* Check on dataset's message storage status */
!!$        if(test_shared != 0) {
!!$            /* Datasets' datatypes can be shared */
!!$            ret = H5F_get_sohm_mesg_count_test(fid, H5O_DTYPE_ID, &mesg_count);
!!$            CHECK(ret, FAIL, "H5F_get_sohm_mesg_count_test");
!!$            VERIFY(mesg_count, 1, "H5F_get_sohm_mesg_count_test");
!!$
!!$            /* Datasets' dataspace can be shared */
!!$            ret = H5F_get_sohm_mesg_count_test(fid, H5O_SDSPACE_ID, &mesg_count);
!!$            CHECK(ret, FAIL, "H5F_get_sohm_mesg_count_test");
!!$            VERIFY(mesg_count, 1, "H5F_get_sohm_mesg_count_test");
!!$        } /* end if */
!!$
     ! /* Retrieve limits for compact/dense attribute storage */
     CALL H5Pget_attr_phase_change_f(dcpl, max_compact, min_dense, error)
     CALL check("H5Pget_attr_phase_change_f",error,total_error)

     ! /* Close property list */
     CALL h5pclose_f(dcpl,error)
     CALL check("h5pclose_f", error, total_error)
!!$
!!$        /* Check on datasets' attribute storage status */
!!$        is_dense = H5O_is_attr_dense_test(dataset);
!!$        VERIFY(is_dense, FALSE, "H5O_is_attr_dense_test");
!!$        is_dense = H5O_is_attr_dense_test(dataset2);
!!$        VERIFY(is_dense, FALSE, "H5O_is_attr_dense_test");
!!$
     ! /* Add attributes to each dataset, until after converting to dense storage */
     
     DO u = 0, (max_compact * 2) - 1

        ! /* Create attribute name */
        WRITE(chr2,'(I2.2)') u
        attrname = 'attr '//chr2
        
        ! /* Alternate between creating "small" & "big" attributes */

        IF(MOD(u+1,2).EQ.0)THEN
           ! /* Create "small" attribute on first dataset */

           CALL h5acreate_f(dataset, attrname, attr_tid, sid, attr, error, H5P_DEFAULT_F, H5P_DEFAULT_F)
           CALL check("h5acreate_f",error,total_error)

!!$                /* Check that attribute is not shared */
!!$                is_shared = H5A_is_shared_test(attr);
!!$                VERIFY(is_shared, FALSE, "H5A_is_shared_test");

           ! /* Write data into the attribute */
           attr_integer_data(1) = u + 1
           data_dims(1) = 1
           CALL h5awrite_f(attr, attr_tid, attr_integer_data, data_dims, error)
           CALL check("h5awrite_f",error,total_error)
        ELSE
           !  Create "big" attribute on first dataset */

           CALL h5acreate_f(dataset, attrname, attr_tid, big_sid, attr, error, H5P_DEFAULT_F, H5P_DEFAULT_F)
           CALL check("h5acreate_f",error,total_error)
!!$
           !  Check that attribute is shared */
!!$                is_shared = H5A_is_shared_test(attr);
!!$                VERIFY(is_shared, TRUE, "H5A_is_shared_test");

           !  Check refcount for attribute */
!!$                ret = H5A_get_shared_rc_test(attr, &shared_refcount);
!!$                CHECK(ret, FAIL, "H5A_get_shared_rc_test");
!!$                VERIFY(shared_refcount, 1, "H5A_get_shared_rc_test");
!!$
           !  Write data into the attribute */

           attr_integer_data(1) = u + 1
           data_dims(1) = 1
           CALL h5awrite_f(attr,  attr_tid, attr_integer_data, data_dims, error)
           CALL check("h5awrite_f",error,total_error)

           !  Check refcount for attribute */
!!$                ret = H5A_get_shared_rc_test(attr, &shared_refcount);
!!$                CHECK(ret, FAIL, "H5A_get_shared_rc_test");
!!$                VERIFY(shared_refcount, 1, "H5A_get_shared_rc_test");
        ENDIF

        ! /* Close attribute */
        CALL h5aclose_f(attr, error)
        CALL check("h5aclose_f",error,total_error)

        ! /* Check on dataset's attribute storage status */
!!$            is_dense = H5O_is_attr_dense_test(dataset);
!!$            if(u < max_compact)
!!$                VERIFY(is_dense, FALSE, "H5O_is_attr_dense_test");
!!$            else
!!$                VERIFY(is_dense, TRUE, "H5O_is_attr_dense_test");
!!$
!!$
        ! /* Alternate between creating "small" & "big" attributes */
        IF(MOD(u+1,2).EQ.0)THEN

           !  /* Create "small" attribute on second dataset */

           CALL h5acreate_f(dataset2, attrname, attr_tid, sid, attr, error, H5P_DEFAULT_F, H5P_DEFAULT_F)
           CALL check("h5acreate_f",error,total_error)

           !  /* Check that attribute is not shared */
!!$                is_shared = H5A_is_shared_test(attr);
!!$                VERIFY(is_shared, FALSE, "H5A_is_shared_test");
!!$
           ! /* Write data into the attribute */
           attr_integer_data(1) = u + 1
           data_dims(1) = 1
           CALL h5awrite_f(attr, attr_tid, attr_integer_data, data_dims, error)
           CALL check("h5awrite_f",error,total_error)
        ELSE

           ! /* Create "big" attribute on second dataset */
           
           CALL h5acreate_f(dataset2, attrname, attr_tid, big_sid, attr, error, H5P_DEFAULT_F, H5P_DEFAULT_F)
           CALL check("h5acreate_f",error,total_error)

! /* Check that attribute is shared */
!!$                is_shared = H5A_is_shared_test(attr);
!!$                VERIFY(is_shared, TRUE, "H5A_is_shared_test");
!!$
! /* Check refcount for attribute */
!!$                ret = H5A_get_shared_rc_test(attr, &shared_refcount);
!!$                CHECK(ret, FAIL, "H5A_get_shared_rc_test");
!!$                VERIFY(shared_refcount, 1, "H5A_get_shared_rc_test");
!!$
! /* Write data into the attribute */

           
           attr_integer_data(1) = u + 1
           data_dims(1) = 1
           CALL h5awrite_f(attr,  attr_tid, attr_integer_data, data_dims, error)
           CALL check("h5awrite_f",error,total_error)


! /* Check refcount for attribute */
!!$                ret = H5A_get_shared_rc_test(attr, &shared_refcount);
!!$                CHECK(ret, FAIL, "H5A_get_shared_rc_test");
!!$                VERIFY(shared_refcount, 2, "H5A_get_shared_rc_test");

        ENDIF
        ! /* Close attribute */
        CALL h5aclose_f(attr, error)
        CALL check("h5aclose_f",error,total_error)

        ! /* Check on dataset's attribute storage status */
!!$            is_dense = H5O_is_attr_dense_test(dataset2);
!!$            if(u < max_compact)
!!$                VERIFY(is_dense, FALSE, "H5O_is_attr_dense_test");
!!$            else
!!$                VERIFY(is_dense, TRUE, "H5O_is_attr_dense_test");
     ENDDO

     ! /* Delete attributes from second dataset */

     DO u = 0, max_compact*2-1

        ! /* Create attribute name */
        WRITE(chr2,'(I2.2)') u
        attrname = 'attr '//chr2

        ! /* Delete second dataset's attribute */
        CALL H5Adelete_by_name_f(fid, DSET2_NAME, attrname, H5P_DEFAULT_F,error)
        CALL check("H5Adelete_by_name", error, total_error)

!!$            /* Check refcount on attributes now */
!!$
!!$            /* Check refcount on first dataset's attribute */

        CALL h5aopen_f(dataset, attrname, H5P_DEFAULT_F, attr, error)
        CALL check("h5aopen_f",error,total_error)

!!$
!!$            if(u % 2) {
! /* Check that attribute is not shared */
!!$                is_shared = H5A_is_shared_test(attr);
!!$                VERIFY(is_shared, FALSE, "H5A_is_shared_test");
!!$            } /* end if */
!!$            else {
!/*  Check that attribute is shared */
!!$                is_shared = H5A_is_shared_test(attr);
!!$                VERIFY(is_shared, TRUE, "H5A_is_shared_test");
!!$
!/*  Check refcount for attribute */
!!$                ret = H5A_get_shared_rc_test(attr, &shared_refcount);
!!$                CHECK(ret, FAIL, "H5A_get_shared_rc_test");
!!$                VERIFY(shared_refcount, 1, "H5A_get_shared_rc_test");
!!$            } /* end else */

        ! /* Close attribute */
        CALL h5aclose_f(attr, error)
        CALL check("h5aclose_f",error,total_error)
     ENDDO

     ! /* Close attribute's datatype */
     
     CALL h5tclose_f(attr_tid, error)
     CALL check("h5tclose_f",error,total_error)

     ! /* Close Datasets */

     CALL h5dclose_f(dataset, error)
     CALL check("h5dclose_f",error,total_error)
     CALL h5dclose_f(dataset2, error)
     CALL check("h5dclose_f",error,total_error)

     ! /* Check on shared message status now */
!!$        if(test_shared != 0) {
!!$            if(test_shared == 1) {
     ! /*  Check on datatype storage status */
!!$                ret = H5F_get_sohm_mesg_count_test(fid, H5O_DTYPE_ID, &mesg_count);
!!$                CHECK(ret, FAIL, "H5F_get_sohm_mesg_count_test");
!!$                VERIFY(mesg_count, 2, "H5F_get_sohm_mesg_count_test");
!!$            } /* end if */
!!$
!!$            /* Check on dataspace storage status */
!!$            ret = H5F_get_sohm_mesg_count_test(fid, H5O_SDSPACE_ID, &mesg_count);
!!$            CHECK(ret, FAIL, "H5F_get_sohm_mesg_count_test");
!!$            VERIFY(mesg_count, 2, "H5F_get_sohm_mesg_count_test");
!!$        } /* end if */
!!$
     ! /* Unlink datasets WITH attributes */

     CALL h5ldelete_f(fid, DSET1_NAME, H5P_DEFAULT_F, error)
     CALL check("H5Ldelete_f", error, total_error)
     CALL h5ldelete_f(fid, DSET2_NAME, H5P_DEFAULT_F, error)
     CALL check("H5Ldelete_f", error, total_error)

     ! /* Unlink committed datatype */

     IF( test_shared == 2) THEN
        CALL h5ldelete_f(fid, TYPE1_NAME, H5P_DEFAULT_F, error)
        CALL check("H5Ldelete_f", error, total_error)
     ENDIF

     ! /* Check on attribute storage status */
!!$        ret = H5F_get_sohm_mesg_count_test(fid, H5O_ATTR_ID, &mesg_count);
!!$        CHECK(ret, FAIL, "H5F_get_sohm_mesg_count_test");
!!$        VERIFY(mesg_count, 0, "H5F_get_sohm_mesg_count_test");
!!$
!!$        if(test_shared != 0) {
!!$            /* Check on datatype storage status */
!!$            ret = H5F_get_sohm_mesg_count_test(fid, H5O_DTYPE_ID, &mesg_count);
!!$            CHECK(ret, FAIL, "H5F_get_sohm_mesg_count_test");
!!$            VERIFY(mesg_count, 0, "H5F_get_sohm_mesg_count_test");
!!$
!!$            /* Check on dataspace storage status */
!!$            ret = H5F_get_sohm_mesg_count_test(fid, H5O_SDSPACE_ID, &mesg_count);
!!$            CHECK(ret, FAIL, "H5F_get_sohm_mesg_count_test");
!!$            VERIFY(mesg_count, 0, "H5F_get_sohm_mesg_count_test");
!!$        } /* end if */
!!$

     ! /* Close file */
     CALL h5fclose_f(fid, error)
     CALL check("h5fclose_f",error,total_error)
!!$
!!$        /* Check size of file */
!!$        filesize = h5_get_file_size(FILENAME);
!!$        VERIFY(filesize, empty_filesize, "h5_get_file_size");
  ENDDO

  ! /* Close dataspaces */
  CALL h5sclose_f(sid, error)
  CALL check("h5sclose_f",error,total_error)
  CALL h5sclose_f(big_sid, error)
  CALL check("h5sclose_f",error,total_error)

END SUBROUTINE test_attr_shared_delete



SUBROUTINE test_attr_dense_open( fcpl, fapl, total_error)

!/****************************************************************
!**
!**  test_attr_dense_open(): Test basic H5A (attribute) code.
!**      Tests opening attributes in "dense" storage
!**
!****************************************************************/

  USE HDF5
  
  IMPLICIT NONE

  INTEGER(HID_T), INTENT(IN) :: fcpl
  INTEGER(HID_T), INTENT(IN) :: fapl
  INTEGER, INTENT(IN) :: total_error
  CHARACTER(LEN=8) :: FileName = "tattr.h5"
  INTEGER(HID_T) :: fid
  INTEGER(HID_T) :: dcpl
  INTEGER(HID_T) :: sid

  CHARACTER(LEN=8) :: DSET1_NAME = "Dataset1"

  INTEGER :: curr_dset

  INTEGER(HID_T) :: dset1, dset2, dset3
  INTEGER(HID_T) :: dataset, dataset2
  INTEGER(HID_T) :: my_dataset

  INTEGER :: error

  INTEGER :: value
  INTEGER(HID_T) :: attr        !String Attribute identifier
  INTEGER(HSIZE_T), DIMENSION(7) :: data_dims
  INTEGER(HSIZE_T) :: storage_size   ! attributes storage requirements .MSB.

  LOGICAL :: f_corder_valid ! Indicates whether the the creation order data is valid for this attribute 
  INTEGER :: corder ! Is a positive integer containing the creation order of the attribute
  INTEGER :: cset ! Indicates the character set used for the attribute’s name
  INTEGER(HSIZE_T) :: data_size   ! indicates the size, in the number of characters
  INTEGER(HSIZE_T) :: n

  INTEGER :: max_compact ! Maximum # of links to store in group compactly
  INTEGER :: min_dense   ! Minimum # of links to store in group "densely"

  CHARACTER(LEN=2) :: chr2


  INTEGER, DIMENSION(1) ::  attr_integer_data
  CHARACTER(LEN=7) :: attrname

  INTEGER(SIZE_T) :: size
  
  INTEGER :: order
  INTEGER :: u

  data_dims = 0

  ! /* Output message about test being performed */
  WRITE(*,*) "Testing Opening Attributes in Dense Storage"

  ! /* Create file */

  CALL h5fcreate_f(FileName, H5F_ACC_TRUNC_F, fid, error, fcpl, fapl)
  CALL check("h5fcreate_f",error,total_error)

  ! /* Close file */
  CALL h5fclose_f(fid, error)
  CALL check("h5fclose_f",error,total_error)


  ! /* Get size of file */
!!$    empty_filesize = h5_get_file_size(FILENAME);
!!$    if(empty_filesize < 0)
!!$        TestErrPrintf("Line %d: file size wrong!\n", __LINE__);

  ! /* Re-open file */
  CALL h5fopen_f(FileName, H5F_ACC_RDWR_F, fid, error, fapl)
  CALL check("h5open_f",error,total_error)

  ! /* Create dataspace for dataset */
  CALL h5screate_f(H5S_SCALAR_F, sid, error)
  CALL check("h5screate_f",error,total_error)

  ! /* Query the group creation properties */
  CALL H5Pcreate_f(H5P_DATASET_CREATE_F,dcpl,error)
  CALL check("h5Pcreate_f",error,total_error)

  ! /* Enable creation order tracking on attributes, so creation order tests work */
  CALL H5Pset_attr_creation_order_f(dcpl, H5P_CRT_ORDER_TRACKED_F, error)
  CALL check("H5Pset_attr_creation_order",error,total_error)

  ! /* Create a dataset */
     
  CALL h5dcreate_f(fid, DSET1_NAME, H5T_NATIVE_CHARACTER, sid, dataset, error , H5P_DEFAULT_F, dcpl, H5P_DEFAULT_F)
  CALL check("h5dcreate_f",error,total_error)

  ! /* Retrieve limits for compact/dense attribute storage */
  CALL H5Pget_attr_phase_change_f(dcpl, max_compact, min_dense, error)
  CALL check("H5Pget_attr_phase_change_f",error,total_error)

  ! /* Close property list */
  CALL h5pclose_f(dcpl, error)
  CALL check("h5pclose_f",error,total_error)

  ! /* Check on dataset's attribute storage status */
  !  is_dense = H5O_is_attr_dense_test(dataset);
  !  VERIFY(is_dense, FALSE, "H5O_is_attr_dense_test");

  ! /* Add attributes, until just before converting to dense storage */

  DO u = 0, max_compact - 1
     ! /* Create attribute */
     WRITE(chr2,'(I2.2)') u
     attrname = 'attr '//chr2

     CALL h5acreate_f(dataset, attrname, H5T_NATIVE_INTEGER, sid, attr, error, H5P_DEFAULT_F, H5P_DEFAULT_F)
     CALL check("h5acreate_f",error,total_error)

     ! /* Write data into the attribute */

     data_dims(1) = 1 
     CALL h5awrite_f(attr, H5T_NATIVE_INTEGER, u, data_dims, error)
     CALL check("h5awrite_f",error,total_error)

     ! /* Close attribute */
     CALL h5aclose_f(attr, error)
     CALL check("h5aclose_f",error,total_error)

     ! /* Verify attributes written so far */
     CALL test_attr_dense_verify(dataset, u, total_error)
!!$        CHECK(ret, FAIL, "test_attr_dense_verify");
  ENDDO

  ! /* Check on dataset's attribute storage status */
!!$    is_dense = H5O_is_attr_dense_test(dataset);
!!$    VERIFY(is_dense, FALSE, "H5O_is_attr_dense_test");

!    /* Add one more attribute, to push into "dense" storage */
!    /* Create attribute */
  
  WRITE(chr2,'(I2.2)') u
  attrname = 'attr '//chr2

  CALL h5acreate_f(dataset, attrname, H5T_NATIVE_INTEGER, sid, attr, error, H5P_DEFAULT_F, H5P_DEFAULT_F)
  CALL check("h5acreate_f",error,total_error)

  ! /* Check on dataset's attribute storage status */
!!$    is_dense = H5O_is_attr_dense_test(dataset);
!!$    VERIFY(is_dense, TRUE, "H5O_is_attr_dense_test");

  
  ! /* Write data into the attribute */
  data_dims(1) = 1 
  CALL h5awrite_f(attr, H5T_NATIVE_INTEGER, u, data_dims, error)
  CALL check("h5awrite_f",error,total_error)

  ! /* Close attribute */
  CALL h5aclose_f(attr, error)
  CALL check("h5aclose_f",error,total_error)


  ! /* Close dataspace */
  CALL h5sclose_f(sid, error)
  CALL check("h5sclose_f",error,total_error)

  ! /* Verify all the attributes written */
  !  ret = test_attr_dense_verify(dataset, (u + 1));
  !  CHECK(ret, FAIL, "test_attr_dense_verify");

  ! /* CLOSE Dataset */
  CALL h5dclose_f(dataset, error)
  CALL check("h5dclose_f",error,total_error)

  ! /* Unlink dataset with attributes */
  CALL h5ldelete_f(fid, DSET1_NAME, H5P_DEFAULT_F, error)
  CALL check("H5Ldelete_f", error, total_error)

  ! /* Close file */
  CALL h5fclose_f(fid, error)
  CALL check("h5fclose_f",error,total_error)

  ! /* Check size of file */
  !  filesize = h5_get_file_size(FILENAME);
  !  VERIFY(filesize, empty_filesize, "h5_get_file_size")

END SUBROUTINE test_attr_dense_open

!/****************************************************************
!**
!**  test_attr_dense_verify(): Test basic H5A (attribute) code.
!**      Verify attributes on object
!**
!****************************************************************/

SUBROUTINE test_attr_dense_verify(loc_id, max_attr, total_error)

  USE HDF5
  
  IMPLICIT NONE

  INTEGER(HID_T), INTENT(IN) :: loc_id
  INTEGER, INTENT(IN) :: max_attr
  INTEGER, INTENT(INOUT) :: total_error

  INTEGER(SIZE_T), PARAMETER :: ATTR_NAME_LEN = 8 ! FIX, why if 7 does not work?

  INTEGER :: u
  CHARACTER(LEN=2) :: chr2
  CHARACTER(LEN=ATTR_NAME_LEN) :: attrname
  CHARACTER(LEN=ATTR_NAME_LEN) :: check_name
  INTEGER(HSIZE_T), DIMENSION(7) :: data_dims

  INTEGER(HID_T) :: attr        !String Attribute identifier 
  INTEGER :: error
  INTEGER :: value

  data_dims = 0
  

    ! /* Retrieve the current # of reported errors */
    ! old_nerrs = GetTestNumErrs();

    ! /* Re-open all the attributes by name and verify the data */

  DO u = 0, max_attr -1

     ! /* Open attribute */
     WRITE(chr2,'(I2.2)') u
     attrname = 'attr '//chr2

     CALL h5aopen_f(loc_id, attrname, H5P_DEFAULT_F, attr, error)
     CALL check("h5aopen_f",error,total_error)

     ! /* Read data from the attribute */

!     value = 103
     data_dims(1) = 1
     CALL h5aread_f(attr, H5T_NATIVE_INTEGER, value, data_dims, error)

     CALL CHECK("H5Aread_F", error, total_error)
     CALL VERIFY("H5Aread_F", value, u);

     ! /* Close attribute */
     CALL h5aclose_f(attr, error)
     CALL check("h5aclose_f",error,total_error)
  ENDDO

  ! /* Re-open all the attributes by index and verify the data */

  DO u=0, max_attr-1

!     size_t name_len;                /* Length of attribute name */
!     char check_name[ATTR_NAME_LEN]; /* Buffer for checking attribute names */

     ! /* Open attribute */

     CALL H5Aopen_by_idx_f(loc_id, ".", H5_INDEX_CRT_ORDER_F, H5_ITER_INC_F, INT(u,HSIZE_T), H5P_DEFAULT_F, H5P_DEFAULT_F, attr, error)

     ! /* Verify Name */
     
     WRITE(chr2,'(I2.2)') u
     attrname = 'attr '//chr2

     CALL H5Aget_name_f(attr, ATTR_NAME_LEN, check_name, error)
     CALL check('H5Aget_name',error,total_error)
     IF(check_name.NE.attrname) THEN
        WRITE(*,*) 'ERROR: attribute name different: attr_name = ',check_name, ', should be ', attrname
        total_error = total_error + 1
     ENDIF
     ! /* Read data from the attribute */
     data_dims(1) = 1
     CALL h5aread_f(attr, H5T_NATIVE_INTEGER, value, data_dims, error)
     CALL CHECK("H5Aread_f", error, total_error)
     CALL VERIFY("H5Aread_f", value, u)


     ! /* Close attribute */
     CALL h5aclose_f(attr, error)
     CALL check("h5aclose_f",error,total_error)
  ENDDO

END SUBROUTINE test_attr_dense_verify

!/****************************************************************
!**
!**  test_attr_corder_create_empty(): Test basic H5A (attribute) code.
!**      Tests basic code to create objects with attribute creation order info
!**
!****************************************************************/

SUBROUTINE test_attr_corder_create_basic( fcpl, fapl, total_error )

  USE HDF5
  
  IMPLICIT NONE

  INTEGER(HID_T), INTENT(IN) :: fcpl
  INTEGER(HID_T), INTENT(IN) :: fapl
  INTEGER, INTENT(IN) :: total_error
  CHARACTER(LEN=8) :: FileName = "tattr.h5"
  INTEGER(HID_T) :: fid
  INTEGER(HID_T) :: dcpl
  INTEGER(HID_T) :: sid

  CHARACTER(LEN=8) :: DSET1_NAME = "Dataset1"

  INTEGER :: curr_dset

  INTEGER(HID_T) :: dset1, dset2, dset3
  INTEGER(HID_T) :: dataset, dataset2
  INTEGER(HID_T) :: my_dataset

  INTEGER :: error

  INTEGER :: value
  INTEGER(HID_T) :: attr        !String Attribute identifier
  INTEGER(HSIZE_T), DIMENSION(7) :: data_dims
  INTEGER(HSIZE_T) :: storage_size   ! attributes storage requirements .MSB.

  LOGICAL :: f_corder_valid ! Indicates whether the the creation order data is valid for this attribute 
  INTEGER :: corder ! Is a positive integer containing the creation order of the attribute
  INTEGER :: cset ! Indicates the character set used for the attribute’s name
  INTEGER(HSIZE_T) :: data_size   ! indicates the size, in the number of characters
  INTEGER(HSIZE_T) :: n

  INTEGER :: max_compact ! Maximum # of links to store in group compactly
  INTEGER :: min_dense   ! Minimum # of links to store in group "densely"

  CHARACTER(LEN=2) :: chr2


  INTEGER, DIMENSION(1) ::  attr_integer_data
  CHARACTER(LEN=7) :: attrname

  INTEGER(SIZE_T) :: size
  
  INTEGER :: order
  INTEGER :: u
  INTEGER :: crt_order_flags

  ! /* Output message about test being performed */
  WRITE(*,*) "Testing Basic Code for Attributes with Creation Order Info"

  ! /* Create file */
  CALL h5fcreate_f(FileName, H5F_ACC_TRUNC_F, fid, error, fcpl, fapl)
  CALL check("h5fcreate_f",error,total_error)

  ! /* Create dataset creation property list */
  CALL H5Pcreate_f(H5P_DATASET_CREATE_F,dcpl,error)
  CALL check("h5Pcreate_f",error,total_error)
  
  ! /* Get creation order indexing on object */
  CALL H5Pget_attr_creation_order_f(dcpl, crt_order_flags, error)
  CALL check("H5Pget_attr_creation_order_f",error,total_error)
  CALL VERIFY("H5Pget_attr_creation_order_f",crt_order_flags , 0)

  ! /* Setting invalid combination of a attribute order creation order indexing on should fail */
  CALL H5Pset_attr_creation_order_f(dcpl, H5P_CRT_ORDER_INDEXED_F, error)
  CALL VERIFY("H5Pset_attr_creation_order_f",error , -1)
  CALL H5Pget_attr_creation_order_f(dcpl, crt_order_flags, error)
  CALL check("H5Pget_attr_creation_order_f",error,total_error)
  CALL VERIFY("H5Pget_attr_creation_order_f",crt_order_flags , 0)

  ! /* Set attribute creation order tracking & indexing for object */
  CALL h5pset_attr_creation_order_f(dcpl, IOR(H5P_CRT_ORDER_TRACKED_F, H5P_CRT_ORDER_INDEXED_F), error)
  CALL check("H5Pset_attr_creation_order_f",error,total_error)

  CALL H5Pget_attr_creation_order_f(dcpl, crt_order_flags, error)
  CALL check("H5Pget_attr_creation_order_f",error,total_error)
  CALL VERIFY("H5Pget_attr_creation_order_f",crt_order_flags , IOR(H5P_CRT_ORDER_TRACKED_F, H5P_CRT_ORDER_INDEXED_F))

  ! /* Create dataspace for dataset */
  CALL h5screate_f(H5S_SCALAR_F, sid, error)
  CALL check("h5screate_f",error,total_error)

  ! /* Create a dataset */
  CALL h5dcreate_f(fid, DSET1_NAME, H5T_NATIVE_CHARACTER, sid, dataset, error , H5P_DEFAULT_F, dcpl, H5P_DEFAULT_F)
  CALL check("h5dcreate_f",error,total_error)

  ! /* Close dataspace */
  CALL h5sclose_f(sid, error)
  CALL check("h5sclose_f",error,total_error)

  ! /* Check on dataset's attribute storage status */
!!$    is_empty = H5O_is_attr_empty_test(dataset);
!!$    VERIFY(is_empty, TRUE, "H5O_is_attr_empty_test");
!!$    is_dense = H5O_is_attr_dense_test(dataset);
!!$    VERIFY(is_dense, FALSE, "H5O_is_attr_dense_test");

  ! /* Close Dataset */
  CALL h5dclose_f(dataset, error)
  CALL check("h5dclose_f",error,total_error)

  ! /* Close property list */
  CALL h5pclose_f(dcpl, error)
  CALL check("h5pclose_f",error,total_error)

  ! /* Close file */
  CALL h5fclose_f(fid, error)
  CALL check("h5fclose_f",error,total_error)

  ! /* Re-open file */
  CALL h5fopen_f(FileName, H5F_ACC_RDWR_F, fid, error, fapl)
  CALL check("h5open_f",error,total_error)

  ! /* Open dataset created */
  CALL h5dopen_f(fid, DSET1_NAME, dataset, error, H5P_DEFAULT_F )
  CALL check("h5dopen_f",error,total_error)

  ! /* Check on dataset's attribute storage status */
!!$    is_empty = H5O_is_attr_empty_test(dataset);
!!$    VERIFY(is_empty, TRUE, "H5O_is_attr_empty_test");
!!$    is_dense = H5O_is_attr_dense_test(dataset);
!!$    VERIFY(is_dense, FALSE, "H5O_is_attr_dense_test");

  ! /* Retrieve dataset creation property list for group */
  CALL H5Dget_create_plist_f(dataset, dcpl, error)
  CALL check("H5Dget_create_plist_f",error,total_error)

  ! /* Query the attribute creation properties */
  CALL H5Pget_attr_creation_order_f(dcpl, crt_order_flags, error)
  CALL check("H5Pget_attr_creation_order_f",error,total_error)
  CALL VERIFY("H5Pget_attr_creation_order_f",crt_order_flags , IOR(H5P_CRT_ORDER_TRACKED_F, H5P_CRT_ORDER_INDEXED_F))

  ! /* Close property list */
  CALL h5pclose_f(dcpl, error)
  CALL check("h5pclose_f",error,total_error)
  
  ! /* Close Dataset */
  CALL h5dclose_f(dataset, error)
  CALL check("h5dclose_f",error,total_error)
  
  ! /* Close file */
  CALL h5fclose_f(fid, error)
  CALL check("h5fclose_f",error,total_error)


END SUBROUTINE test_attr_corder_create_basic

!/****************************************************************
!**
!**  test_attr_basic_write(): Test basic H5A (attribute) code.
!**      Tests integer attributes on both datasets and groups
!**
!****************************************************************/

SUBROUTINE test_attr_basic_write(fapl, total_error)

  USE HDF5
  
  IMPLICIT NONE

  INTEGER(HID_T), INTENT(IN) :: fapl
  INTEGER, INTENT(INOUT) :: total_error
  CHARACTER(LEN=8) :: FileName = "tattr.h5"
  INTEGER(HID_T) :: fid,fid1
  INTEGER(HID_T) :: dcpl
  INTEGER(HID_T) :: sid, sid1, sid2

  CHARACTER(LEN=8) :: DSET1_NAME = "Dataset1"

  INTEGER :: curr_dset

  INTEGER(HID_T) :: dset1, dset2, dset3
  INTEGER(HID_T) :: dataset, dataset2
  INTEGER(HID_T) :: my_dataset

  INTEGER :: error

  INTEGER :: value
  INTEGER(HID_T) :: attr,attr1,attr2        !String Attribute identifier
  INTEGER(HID_T) :: group
  INTEGER(HSIZE_T), DIMENSION(7) :: data_dims
  INTEGER(HSIZE_T) :: storage_size   ! attributes storage requirements .MSB.

  LOGICAL :: f_corder_valid ! Indicates whether the the creation order data is valid for this attribute 
  INTEGER :: corder ! Is a positive integer containing the creation order of the attribute
  INTEGER :: cset ! Indicates the character set used for the attribute’s name
  INTEGER(HSIZE_T) :: data_size   ! indicates the size, in the number of characters
  INTEGER(HSIZE_T) :: n

  INTEGER :: max_compact ! Maximum # of links to store in group compactly
  INTEGER :: min_dense   ! Minimum # of links to store in group "densely"

  CHARACTER(LEN=2) :: chr2


  INTEGER, DIMENSION(1) ::  attr_integer_data
  CHARACTER(LEN=7) :: attrname
  CHARACTER(LEN=20) :: check_name

  INTEGER :: order
  INTEGER :: u
  INTEGER :: crt_order_flags

  INTEGER, PARAMETER :: SPACE1_RANK = 3
  INTEGER, PARAMETER :: NX = 20
  INTEGER, PARAMETER :: NY = 5
  INTEGER, PARAMETER :: NZ = 10
!  INTEGER(HSIZE_T), DIMENSION(3) :: dims1 = (/NX,NY,NZ/)

  CHARACTER(LEN=5), PARAMETER ::  ATTR1_NAME="Attr1"
  INTEGER, PARAMETER :: ATTR1_RANK = 1
  INTEGER, PARAMETER ::  ATTR1_DIM1 = 3
  CHARACTER(LEN=7), PARAMETER :: ATTR1A_NAME ="Attr1_a"
  CHARACTER(LEN=18), PARAMETER :: ATTR_TMP_NAME = "Attr1_a-1234567890"
! int attr_data1a[ATTR1_DIM1]={256,11945,-22107};
  INTEGER, DIMENSION(ATTR1_DIM1) ::  attr_data1
  INTEGER, DIMENSION(ATTR1_DIM1) ::  attr_data1a
  INTEGER(HSIZE_T) :: attr_size   ! attributes storage requirements .MSB.
!  INTEGER :: attr_data1
  INTEGER(HSIZE_T), DIMENSION(2) :: dims2 = (/4,6/) ! Dataset dimensions

!!!! start
  INTEGER     ::   rank1 = 2               ! Dataspace1 rank
  INTEGER(HSIZE_T), DIMENSION(2) :: dims1 = (/4,6/) ! Dataset dimensions
  INTEGER(HSIZE_T), DIMENSION(2) :: maxdims1 = (/4,6/) ! maximum dimensions
  INTEGER(HID_T) :: space1_id   ! Dataspace identifiers

  INTEGER(HSIZE_T) :: size 

  attr_data1(1) = 258
  attr_data1(2) = 9987
  attr_data1(3) = -99890
  attr_data1a(1) = 258
  attr_data1a(2) = 1087
  attr_data1a(3) = -99890

  ! /* Output message about test being performed */
  WRITE(*,*) "Testing Basic Scalar Attribute Writing Functions"

  ! /* Create file */
  CALL h5fcreate_f(FileName, H5F_ACC_TRUNC_F, fid1, error, H5P_DEFAULT_F, fapl)
  CALL check("h5fcreate_f",error,total_error)

  ! /* Create dataspace for dataset */

  CALL h5screate_simple_f(rank1, dims1, sid1, error, maxdims1)
!  CALL h5screate_simple_f(SPACE1_RANK, dims1, sid1, error)
  CALL check("h5screate_simple_f",error,total_error)


  ! /* Create a dataset */
!  sid1 = H5Screate_simple(SPACE1_RANK, dims1, NULL);
  CALL h5dcreate_f(fid1, DSET1_NAME, H5T_NATIVE_CHARACTER, sid1, dataset, error, H5P_DEFAULT_F, H5P_DEFAULT_F, H5P_DEFAULT_F )
  CALL check("h5dcreate_f",error,total_error)


  ! /* Create dataspace for attribute */

  CALL h5screate_simple_f(ATTR1_RANK, dims2, sid2, error)
  CALL check("h5screate_simple_f",error,total_error)

  ! /* Try to create an attribute on the file (should create an attribute on root group) */

  CALL h5acreate_f(fid1, ATTR1_NAME, H5T_NATIVE_INTEGER, sid2, attr, error, H5P_DEFAULT_F, H5P_DEFAULT_F)
  CALL check("h5acreate_f",error,total_error)

  !  /* Close attribute */
  CALL h5aclose_f(attr, error)
  CALL check("h5aclose_f",error,total_error)

  ! /* Open the root group */

  CALL H5Gopen_f(fid1, "/", group, error, H5P_DEFAULT_F)
  CALL check("H5Gopen_f",error,total_error)

  ! /* Open attribute again */
  
  CALL h5aopen_f(group,  ATTR1_NAME, H5P_DEFAULT_F, attr, error)
  CALL check("h5aopen_f",error,total_error)


  ! /* Close attribute */
  CALL h5aclose_f(attr, error)
  CALL check("h5aclose_f",error,total_error)

  ! /* Close root group */
  CALL  H5Gclose_f(group, error)
  CALL check("h5gclose_f",error,total_error)


  ! /* Create an attribute for the dataset */

  CALL h5acreate_f(dataset, ATTR1_NAME, H5T_NATIVE_INTEGER, sid2, attr, error, H5P_DEFAULT_F, H5P_DEFAULT_F)
  CALL check("h5acreate_f",error,total_error)

  ! /* Try to create the same attribute again (should fail) */
!    ret = H5Acreate2(dataset, ATTR1_NAME, H5T_NATIVE_INT, sid2, H5P_DEFAULT, H5P_DEFAULT);
!    VERIFY(ret, FAIL, "H5Acreate2");

  ! /* Write attribute information */
  data_dims(1) = 3
  
  CALL h5awrite_f(attr, H5T_NATIVE_INTEGER, attr_data1, data_dims, error)
  CALL check("h5awrite_f",error,total_error)


  ! /* Create an another attribute for the dataset */

  CALL h5acreate_f(dataset, ATTR1A_NAME, H5T_NATIVE_INTEGER, sid2, attr2, error, H5P_DEFAULT_F, H5P_DEFAULT_F)
  CALL check("h5acreate_f",error,total_error)

    ! /* Write attribute information */
  CALL h5awrite_f(attr2, H5T_NATIVE_INTEGER, attr_data1a, data_dims, error)
  CALL check("h5awrite_f",error,total_error)

    ! /* Check storage size for attribute */

  CALL h5aget_storage_size_f(attr, attr_size, error)
  CALL check("h5aget_storage_size_f",error,total_error)
!  PRINT*,attr_size,ATTR1_DIM1,HSIZE_T
  CALL VERIFY("h5aget_storage_size_f",attr_size, 2*HSIZE_T, total_error)

!  attr_size = H5Aget_storage_size(attr);
!  VERIFY(attr_size, (ATTR1_DIM1 * sizeof(int)), "H5A_get_storage_size");




!!$    /* Read attribute information immediately, without closing attribute */
!!$    ret = H5Aread(attr, H5T_NATIVE_INT, read_data1);
!!$    CHECK(ret, FAIL, "H5Aread");
!!$
!!$    /* Verify values read in */
!!$    for(i = 0; i < ATTR1_DIM1; i++)
!!$        if(attr_data1[i] != read_data1[i])
!!$            TestErrPrintf("%d: attribute data different: attr_data1[%d]=%d, read_data1[%d]=%d\n", __LINE__, i, attr_data1[i], i, read_data1[i]);

  ! /* CLOSE attribute */
  CALL h5aclose_f(attr, error)
  CALL check("h5aclose_f",error,total_error)

  ! /* Close attribute */
  CALL h5aclose_f(attr2, error)
  CALL check("h5aclose_f",error,total_error)

  ! /* change attribute name */
  CALL H5Arename_f(dataset, ATTR1_NAME, ATTR_TMP_NAME, error)
  CALL check("H5Arename_f", error, total_error)

  ! /* Open attribute again */

  CALL h5aopen_f(dataset,  ATTR_TMP_NAME, H5P_DEFAULT_F, attr, error)
  CALL check("h5aopen_f",error,total_error)
  ! /* Verify new attribute name */

  ! Set a deliberately small size
  size = 1
  CALL H5Aget_name_f(attr, size, check_name, error)
  CALL check('H5Aget_name',error,total_error)

!!$  PRINT*,'checkname =', check_name
!!$
!!$  check_name ='                  '

  ! Now enter with the corrected size
  IF(error.NE.size)THEN
     size = error
     CALL H5Aget_name_f(attr, size, check_name, error)
     CALL check('H5Aget_name',error,total_error)
  ENDIF

  IF(TRIM(check_name).NE.TRIM(ATTR_TMP_NAME)) THEN
     PRINT*,'.'//TRIM(check_name)//'.',LEN_TRIM(check_name)
     PRINT*,'.'//TRIM(ATTR_TMP_NAME)//'.',LEN_TRIM(ATTR_TMP_NAME)
     WRITE(*,*) 'ERROR: attribute name different: attr_name ='//TRIM(check_name)//'.'
     WRITE(*,*) '                                 should be ='//TRIM(ATTR_TMP_NAME)//'.'
     total_error = total_error + 1
  ENDIF
!!$    attr_name_size = H5Aget_name(attr, (size_t)0, NULL);
!!$    CHECK(attr_name_size, FAIL, "H5Aget_name");
!!$
!!$    if(attr_name_size > 0)
!!$        attr_name = (char*)HDcalloc((size_t)(attr_name_size + 1), sizeof(char));
!!$
!!$    ret = (herr_t)H5Aget_name(attr, (size_t)(attr_name_size + 1), attr_name);
!!$    CHECK(ret, FAIL, "H5Aget_name");
!!$    ret = HDstrcmp(attr_name, ATTR_TMP_NAME);
!!$    VERIFY(ret, 0, "HDstrcmp");
!!$
!!$    if(attr_name)
!!$        HDfree(attr_name);
!!$
!!$    /* Read attribute information immediately, without closing attribute */
!!$    ret = H5Aread(attr, H5T_NATIVE_INT, read_data1);
!!$    CHECK(ret, FAIL, "H5Aread");
!!$
!!$    /* Verify values read in */
!!$    for(i=0; i<ATTR1_DIM1; i++)
!!$        if(attr_data1[i]!=read_data1[i])
!!$            TestErrPrintf("%d: attribute data different: attr_data1[%d]=%d, read_data1[%d]=%d\n",__LINE__,i,attr_data1[i],i,read_data1[i]);
!!$
  ! /* Close attribute */
  CALL h5aclose_f(attr, error)
  CALL check("h5aclose_f",error,total_error)
!!$
!!$    /* Open the second attribute again */
!!$    attr2=H5Aopen(dataset, ATTR1A_NAME, H5P_DEFAULT);
!!$    CHECK(attr, FAIL, "H5Aopen");
!!$
!!$    /* Verify new attribute name */
!!$    attr_name_size = H5Aget_name(attr2, (size_t)0, NULL);
!!$    CHECK(attr_name_size, FAIL, "H5Aget_name");
!!$
!!$    if(attr_name_size>0)
!!$        attr_name = (char*)HDcalloc((size_t)(attr_name_size+1), sizeof(char));
!!$
!!$    ret=(herr_t)H5Aget_name(attr2, (size_t)(attr_name_size+1), attr_name);
!!$    CHECK(ret, FAIL, "H5Aget_name");
!!$    ret=HDstrcmp(attr_name, ATTR1A_NAME);
!!$    VERIFY(ret, 0, "HDstrcmp");
!!$
!!$    if(attr_name)
!!$        HDfree(attr_name);
!!$
!!$    /* Read attribute information immediately, without closing attribute */
!!$    ret=H5Aread(attr2,H5T_NATIVE_INT,read_data1);
!!$    CHECK(ret, FAIL, "H5Aread");
!!$
!!$    /* Verify values read in */
!!$    for(i=0; i<ATTR1_DIM1; i++)
!!$        if(attr_data1a[i]!=read_data1[i])
!!$            TestErrPrintf("%d: attribute data different: attr_data1a[%d]=%d, read_data1[%d]=%d\n",__LINE__,i,attr_data1a[i],i,read_data1[i]);
!!$
!!$    /* Close attribute */
!!$    ret=H5Aclose(attr2);
!!$    CHECK(ret, FAIL, "H5Aclose");

  CALL h5sclose_f(sid1, error)
  CALL check("h5sclose_f",error,total_error)
  CALL h5sclose_f(sid2, error)
  CALL check("h5sclose_f",error,total_error)

  !/* Close Dataset */
  CALL h5dclose_f(dataset, error)
  CALL check("h5dclose_f",error,total_error)

!!$    /* Create group */
!!$    group = H5Gcreate2(fid1, GROUP1_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
!!$    CHECK(group, FAIL, "H5Gcreate2");
!!$
!!$    /* Create dataspace for attribute */
!!$    sid2 = H5Screate_simple(ATTR2_RANK, dims3, NULL);
!!$    CHECK(sid2, FAIL, "H5Screate_simple");
!!$
!!$    /* Create an attribute for the group */
!!$    attr = H5Acreate2(group, ATTR2_NAME, H5T_NATIVE_INT, sid2, H5P_DEFAULT, H5P_DEFAULT);
!!$    CHECK(attr, FAIL, "H5Acreate2");
!!$
!!$    /* Check storage size for attribute */
!!$    attr_size = H5Aget_storage_size(attr);
!!$    VERIFY(attr_size, (ATTR2_DIM1 * ATTR2_DIM2 * sizeof(int)), "H5Aget_storage_size");
!!$
!!$    /* Try to create the same attribute again (should fail) */
!!$    ret = H5Acreate2(group, ATTR2_NAME, H5T_NATIVE_INT, sid2, H5P_DEFAULT, H5P_DEFAULT);
!!$    VERIFY(ret, FAIL, "H5Acreate2");
!!$
!!$    /* Write attribute information */
!!$    ret = H5Awrite(attr, H5T_NATIVE_INT, attr_data2);
!!$    CHECK(ret, FAIL, "H5Awrite");
!!$
!!$    /* Check storage size for attribute */
!!$    attr_size = H5Aget_storage_size(attr);
!!$    VERIFY(attr_size, (ATTR2_DIM1 * ATTR2_DIM2 * sizeof(int)), "H5A_get_storage_size");
!!$
!!$    /* Close attribute */
!!$    ret = H5Aclose(attr);
!!$    CHECK(ret, FAIL, "H5Aclose");
!!$
!!$    /* Close Attribute dataspace */
!!$    ret = H5Sclose(sid2);
!!$    CHECK(ret, FAIL, "H5Sclose");

!!$    !/* Close Group */
!!$    ret = H5Gclose(group);
!!$    CHECK(ret, FAIL, "H5Gclose");

  ! /* Close file */
  CALL h5fclose_f(fid1, error)
  CALL check("h5fclose_f",error,total_error)

END SUBROUTINE test_attr_basic_write

!/****************************************************************
!**
!**  test_attr_many(): Test basic H5A (attribute) code.
!**      Tests storing lots of attributes
!**
!****************************************************************/

SUBROUTINE test_attr_many(new_format, fcpl, fapl, total_error)

  USE HDF5
  
  IMPLICIT NONE

  LOGICAL, INTENT(IN) :: new_format
  INTEGER(HID_T), INTENT(IN) :: fcpl
  INTEGER(HID_T), INTENT(IN) :: fapl
  INTEGER, INTENT(IN) :: total_error
  CHARACTER(LEN=8) :: FileName = "tattr.h5"
  INTEGER(HID_T) :: fid
  INTEGER(HID_T) :: dcpl
  INTEGER(HID_T) :: sid
  INTEGER(HID_T) :: gid
  INTEGER(HID_T) :: aid

  CHARACTER(LEN=8) :: DSET1_NAME = "Dataset1"

  INTEGER :: curr_dset

  INTEGER(HID_T) :: dset1, dset2, dset3
  INTEGER(HID_T) :: dataset, dataset2
  INTEGER(HID_T) :: my_dataset

  INTEGER :: error

  INTEGER :: value
  INTEGER(HID_T) :: attr        !String Attribute identifier
  INTEGER(HSIZE_T), DIMENSION(7) :: data_dims
  INTEGER(HSIZE_T) :: storage_size   ! attributes storage requirements .MSB.

  LOGICAL :: f_corder_valid ! Indicates whether the the creation order data is valid for this attribute 
  INTEGER :: corder ! Is a positive integer containing the creation order of the attribute
  INTEGER :: cset ! Indicates the character set used for the attribute’s name
  INTEGER(HSIZE_T) :: data_size   ! indicates the size, in the number of characters
  INTEGER(HSIZE_T) :: n

  INTEGER :: max_compact ! Maximum # of links to store in group compactly
  INTEGER :: min_dense   ! Minimum # of links to store in group "densely"

  CHARACTER(LEN=5) :: chr5


  INTEGER, DIMENSION(1) ::  attr_integer_data
  CHARACTER(LEN=11) :: attrname
  CHARACTER(LEN=8), PARAMETER :: GROUP1_NAME="/Group1"
  INTEGER(SIZE_T) :: size
  
  INTEGER :: order
  INTEGER :: u
  INTEGER :: crt_order_flags
  INTEGER :: nattr
  LOGICAL :: exists
  INTEGER, DIMENSION(1) ::  attr_data1

  data_dims = 0

  ! /* Output message about test being performed */
  WRITE(*,*) "Testing Storing Many Attributes"

  !/* Create file */
  CALL h5fcreate_f(FileName, H5F_ACC_TRUNC_F, fid, error, fcpl, fapl)
  CALL check("h5fcreate_f",error,total_error)

  ! /* Create dataspace for attribute */
  CALL h5screate_f(H5S_SCALAR_F, sid, error)
  CALL check("h5screate_f",error,total_error)

  ! /* Create group for attributes */

  CALL H5Gcreate_f(fid, GROUP1_NAME, H5P_DEFAULT_F, H5P_DEFAULT_F, H5P_DEFAULT_F, gid, error)
  CALL check("H5Gcreate_f", error, total_error)

  ! /* Create many attributes */

  IF(new_format)THEN
     nattr = 250
  ELSE
     nattr = 2
  ENDIF

  DO u = 0, nattr - 1

     WRITE(chr5,'(I5.5)') u
     attrname = 'attr '//chr5
     CALL H5Aexists_f( gid, attrname, exists, error)
     CALL VerifyLogical("H5Aexists",exists,.FALSE.,total_error )

     CALL H5Aexists_by_name_f(fid, GROUP1_NAME, attrname, H5P_DEFAULT_F, exists, error)
     CALL VerifyLogical("H5Aexists_by_name_f",exists,.FALSE.,total_error )

     CALL h5acreate_f(gid, attrname, H5T_NATIVE_INTEGER, sid, aid, error, H5P_DEFAULT_F, H5P_DEFAULT_F)
     CALL check("h5acreate_f",error,total_error)

     CALL H5Aexists_f(gid, attrname, exists, error)
     CALL VerifyLogical("H5Aexists",exists,.TRUE.,total_error )

     CALL H5Aexists_by_name_f(fid, GROUP1_NAME, attrname, H5P_DEFAULT_F, exists, error)
     CALL VerifyLogical("H5Aexists_by_name_f",exists,.TRUE.,total_error )

     attr_data1(1) = u
     data_dims(1) = 1

     CALL h5awrite_f(aid, H5T_NATIVE_INTEGER, attr_data1, data_dims, error)
     CALL check("h5awrite_f",error,total_error)

     CALL h5aclose_f(aid, error)
     CALL check("h5aclose_f",error,total_error)

     CALL H5Aexists_f(gid, attrname, exists, error)
     CALL VerifyLogical("H5Aexists",exists,.TRUE.,total_error )

     CALL H5Aexists_by_name_f(fid, GROUP1_NAME, attrname, H5P_DEFAULT_F, exists, error)
     CALL VerifyLogical("H5Aexists_by_name_f",exists,.TRUE.,total_error )

  ENDDO

  ! /* Close group */
  CALL  H5Gclose_f(gid, error)
  CALL check("h5gclose_f",error,total_error)

  ! /* Close file */
  CALL h5fclose_f(fid, error)
  CALL check("h5fclose_f",error,total_error)

!!$    /* Re-open the file and check on the attributes */
!!$
!!$    /* Re-open file */
!!$    fid = H5Fopen(FILENAME, H5F_ACC_RDONLY, fapl);
!!$    CHECK(fid, FAIL, "H5Fopen");
!!$
!!$    /* Re-open group */
!!$    gid = H5Gopen2(fid, GROUP1_NAME, H5P_DEFAULT);
!!$    CHECK(gid, FAIL, "H5Gopen2");
!!$
!!$    /* Verify attributes */
!!$    for(u = 0; u < nattr; u++) {
!!$        unsigned    value;          /* Attribute value */
!!$
!!$        sprintf(attrname, "a-%06u", u);
!!$
!!$        exists = H5Aexists(gid, attrname);
!!$        VERIFY(exists, TRUE, "H5Aexists");
!!$
!!$        exists = H5Aexists_by_name(fid, GROUP1_NAME, attrname, H5P_DEFAULT);
!!$        VERIFY(exists, TRUE, "H5Aexists_by_name");
!!$
!!$        aid = H5Aopen(gid, attrname, H5P_DEFAULT);
!!$        CHECK(aid, FAIL, "H5Aopen");
!!$
!!$        exists = H5Aexists(gid, attrname);
!!$        VERIFY(exists, TRUE, "H5Aexists");
!!$
!!$        exists = H5Aexists_by_name(fid, GROUP1_NAME, attrname, H5P_DEFAULT);
!!$        VERIFY(exists, TRUE, "H5Aexists_by_name");
!!$
!!$        ret = H5Aread(aid, H5T_NATIVE_UINT, &value);
!!$        CHECK(ret, FAIL, "H5Aread");
!!$        VERIFY(value, u, "H5Aread");
!!$
!!$        ret = H5Aclose(aid);
!!$        CHECK(ret, FAIL, "H5Aclose");
!!$    } /* end for */
!!$
  ! /* Close group */
!!$  CALL  H5Gclose_f(gid, error)
!!$  CALL check("h5gclose_f",error,total_error)

  ! /* Close file */
!!$  CALL h5fclose_f(fid, error)
!!$  CALL check("h5fclose_f",error,total_error)

!    /* Close dataspaces */
  CALL h5sclose_f(sid, error)
  CALL check("h5sclose_f",error,total_error)

END SUBROUTINE test_attr_many

!/*-------------------------------------------------------------------------
! * Function:    attr_open_check
! *
! * Purpose:     Check opening attribute on an object
! *
! * Return:      Success:        0
! *              Failure:        -1
! *
! * Programmer:  Quincey Koziol
! *              Wednesday, February 21, 2007
! *
! *-------------------------------------------------------------------------
! */

SUBROUTINE attr_open_check(fid, dsetname, obj_id, max_attrs, total_error )

  USE HDF5
  
  IMPLICIT NONE
  INTEGER(HID_T), INTENT(IN) :: fid
  CHARACTER(LEN=*), INTENT(IN) :: dsetname
  INTEGER(HID_T), INTENT(IN) :: obj_id
  INTEGER, INTENT(IN) :: max_attrs
  INTEGER, INTENT(INOUT) :: total_error

  INTEGER :: max_compact,min_dense,u
  CHARACTER (LEN=8) :: attrname

  CHARACTER(LEN=8) :: FileName = "tattr.h5"
  INTEGER(HID_T) :: dcpl
  INTEGER(HID_T) :: sid, null_sid
  INTEGER(HID_T) :: dataset

  CHARACTER(LEN=8) :: DSET1_NAME = "Dataset1"
  CHARACTER(LEN=8) :: DSET2_NAME = "Dataset2"
  CHARACTER(LEN=8) :: DSET3_NAME = "Dataset3"
  INTEGER, PARAMETER :: NUM_DSETS = 3

  INTEGER :: curr_dset

  INTEGER(HID_T) :: dset1, dset2, dset3
  INTEGER(HID_T) :: my_dataset
  INTEGER :: error

  INTEGER :: value
  INTEGER(HID_T) :: attr        !String Attribute identifier 
  INTEGER(HID_T) :: attr_sid
  INTEGER(HSIZE_T), DIMENSION(7) :: data_dims
  INTEGER(HSIZE_T) :: storage_size   ! attributes storage requirements .MSB.

  LOGICAL :: f_corder_valid ! Indicates whether the the creation order data is valid for this attribute 
  INTEGER :: corder ! Is a positive integer containing the creation order of the attribute
  INTEGER :: cset ! Indicates the character set used for the attribute’s name
  INTEGER(HSIZE_T) :: data_size   ! indicates the size, in the number of characters

  CHARACTER(LEN=2) :: chr2
  LOGICAL, DIMENSION(1:2) :: use_index = (/.FALSE.,.TRUE./) 
  INTEGER :: Input1
  INTEGER :: i
  INTEGER(HID_T) attr_id
  ! /* Open each attribute on object by index and check that it's the correct one */

  DO u = 0, max_attrs-1
     ! /* Open the attribute */

     WRITE(chr2,'(I2.2)') u
     attrname = 'attr '//chr2
     
     
     CALL h5aopen_f(obj_id, attrname, H5P_DEFAULT_F, attr_id, error)
     CALL check("h5aopen_f",error,total_error)


     ! /* Get the attribute's information */
     
     CALL h5aget_info_f(attr_id, f_corder_valid, corder, cset, data_size,  error)
     CALL check("h5aget_info_f",error,total_error)
     ! /* Check that the object is the correct one */
     CALL VERIFY("h5aget_info_f",corder,u,total_error)

     ! /* Close attribute */
     CALL h5aclose_f(attr_id, error)
     CALL check("h5aclose_f",error,total_error)

     ! /* Open the attribute */

     CALL H5Aopen_by_name_f(obj_id, ".", attrname, H5P_DEFAULT_F, H5P_DEFAULT_F, attr_id, error)
     CALL check("H5Aopen_by_name_f", error, total_error)

     CALL h5aget_info_f(attr_id, f_corder_valid, corder, cset, data_size,  error)
     CALL check("h5aget_info_f",error,total_error)
     ! /* Get the attribute's information */
     CALL VERIFY("h5aget_info_f",corder,u,total_error)


     ! /* Close attribute */
     CALL h5aclose_f(attr_id, error)
     CALL check("h5aclose_f",error,total_error)


     ! /* Open the attribute */
     CALL H5Aopen_by_name_f(fid, dsetname, attrname, H5P_DEFAULT_F, H5P_DEFAULT_F, attr_id, error)
     CALL check("H5Aopen_by_name_f", error, total_error)


     ! /* Get the attribute's information */
     CALL h5aget_info_f(attr_id, f_corder_valid, corder, cset, data_size,  error)
     CALL check("h5aget_info_f",error,total_error)

     ! /* Check that the object is the correct one */
     CALL VERIFY("h5aget_info_f",corder,u,total_error)

     ! /* Close attribute */
     CALL h5aclose_f(attr_id, error)
     CALL check("h5aclose_f",error,total_error)
  ENDDO

END SUBROUTINE attr_open_check


