#############################
Expected output for 'h5dump tarray5.h5'
#############################
HDF5 "tarray5.h5" {
GROUP "/" {
   DATASET "Dataset1" {
      DATATYPE  H5T_ARRAY { [4] H5T_COMPOUND {
         H5T_STD_I32LE "i";
         H5T_ARRAY { [4] H5T_IEEE_F32LE } "f";
      } }  
      DATASPACE  SIMPLE { ( 4 ) / ( 4 ) } 
      DATA {
         [ {
               0,
               [ 0, 1, 2, 3 ]
            }, {
               1,
               [ 2.5, 3.5, 4.5, 5.5 ]
            }, {
               2,
               [ 5, 6, 7, 8 ]
            }, {
               3,
               [ 7.5, 8.5, 9.5, 10.5 ]
            } ],
         [ {
               10,
               [ 10, 11, 12, 13 ]
            }, {
               11,
               [ 12.5, 13.5, 14.5, 15.5 ]
            }, {
               12,
               [ 15, 16, 17, 18 ]
            }, {
               13,
               [ 17.5, 18.5, 19.5, 20.5 ]
            } ],
         [ {
               20,
               [ 20, 21, 22, 23 ]
            }, {
               21,
               [ 22.5, 23.5, 24.5, 25.5 ]
            }, {
               22,
               [ 25, 26, 27, 28 ]
            }, {
               23,
               [ 27.5, 28.5, 29.5, 30.5 ]
            } ],
         [ {
               30,
               [ 30, 31, 32, 33 ]
            }, {
               31,
               [ 32.5, 33.5, 34.5, 35.5 ]
            }, {
               32,
               [ 35, 36, 37, 38 ]
            }, {
               33,
               [ 37.5, 38.5, 39.5, 40.5 ]
            } ]
      } 
   } 
} 
} 
