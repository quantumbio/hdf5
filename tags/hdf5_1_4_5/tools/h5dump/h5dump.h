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
#ifndef H5DUMP_H__
#define H5DUMP_H__

#include "hdf5.h"

#define H5DUMP_MAX_RANK		H5S_MAX_RANK

#define begin_obj(obj,name,begin)                               \
    if (name)                                                   \
        printf("%s \"%s\" %s\n", (obj), (name), (begin));       \
    else                                                        \
        printf("%s %s\n", (obj), (begin));

#define end_obj(obj,end)                                        \
    printf("%s %s\n", (end), (obj));

#endif  /* !H5DUMP_H__ */
