#############################
Expected output for 'h5dump tvlstr.h5'
#############################
HDF5 "tvlstr.h5" {
GROUP "/" {
   ATTRIBUTE "test_scalar" {
      DATATYPE  "/vl_string_type"
      DATASPACE  SCALAR
      DATA {
         "This is the string for the attribute"
      }
   }
   DATASET "Dataset1" {
      DATATYPE  H5T_STRING {
            STRSIZE H5T_VARIABLE;
            STRPAD H5T_STR_NULLTERM;
            CSET H5T_CSET_ASCII;
            CTYPE H5T_C_S1;
         }
      DATASPACE  SIMPLE { ( 4 ) / ( 4 ) }
      DATA {
         "Four score and seven years ago our forefathers brought forth on this continent a new nation,",
         "conceived in liberty and dedicated to the proposition that all men are created equal.",
         "", NULL
      }
   }
   DATATYPE "vl_string_type"       H5T_STRING {
         STRSIZE H5T_VARIABLE;
         STRPAD H5T_STR_NULLPAD;
         CSET H5T_CSET_ASCII;
         CTYPE H5T_C_S1;
      };

}
}
