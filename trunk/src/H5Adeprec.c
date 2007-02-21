/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at the  *
 * root level of an installed copy of the electronic HDF5 document set and   *
 * is linked from the top-level documents page.  It can also be found at     *
 * http://hdfgroup.org/HDF5/doc/Copyright.html.  If you do not have          *
 * access to either file, you may request a copy from help@hdfgroup.org.     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*-------------------------------------------------------------------------
 *
 * Created:	H5Adeprec.c
 *		November 27 2006
 *		Quincey Koziol <koziol@hdfgroup.org>
 *
 * Purpose:	Deprecated functions from the H5A interface.  These
 *              functions are here for compatibility purposes and may be
 *              removed in the future.  Applications should switch to the
 *              newer APIs.
 *
 *-------------------------------------------------------------------------
 */

/****************/
/* Module Setup */
/****************/

#define H5A_PACKAGE		/*suppress error about including H5Apkg   */
#define H5O_PACKAGE		/*suppress error about including H5Opkg	*/

/* Interface initialization */
#define H5_INTERFACE_INIT_FUNC	H5A_init_deprec_interface


/***********/
/* Headers */
/***********/
#include "H5private.h"		/* Generic Functions			*/
#include "H5Apkg.h"		/* Attributes				*/
#include "H5Dprivate.h"		/* Datasets				*/
#include "H5Eprivate.h"		/* Error handling		  	*/
#include "H5Iprivate.h"		/* IDs			  		*/
#include "H5Opkg.h"             /* Object headers			*/


/****************/
/* Local Macros */
/****************/


/******************/
/* Local Typedefs */
/******************/


/********************/
/* Package Typedefs */
/********************/


/********************/
/* Local Prototypes */
/********************/


/*********************/
/* Package Variables */
/*********************/


/*****************************/
/* Library Private Variables */
/*****************************/


/*******************/
/* Local Variables */
/*******************/



/*--------------------------------------------------------------------------
NAME
   H5A_init_deprec_interface -- Initialize interface-specific information
USAGE
    herr_t H5A_init_deprec_interface()
RETURNS
    Non-negative on success/Negative on failure
DESCRIPTION
    Initializes any interface-specific data or routines.  (Just calls
    H5A_init() currently).

--------------------------------------------------------------------------*/
static herr_t
H5A_init_deprec_interface(void)
{
    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5A_init_deprec_interface)

    FUNC_LEAVE_NOAPI(H5A_init())
} /* H5A_init_deprec_interface() */


/*--------------------------------------------------------------------------
 NAME
    H5Aopen_idx
 PURPOSE
    Opens the n'th attribute for an object
 USAGE
    hid_t H5Aopen_idx (loc_id, idx)
        hid_t loc_id;       IN: Object that attribute is attached to
        unsigned idx;       IN: Index (0-based) attribute to open
 RETURNS
    ID of attribute on success, negative on failure

 DESCRIPTION
        This function opens an existing attribute for access.  The attribute
    index specified is used to look up the corresponding attribute for the
    object.  The attribute ID returned from this function must be released with
    H5Aclose or resource leaks will develop.
        The location object may be either a group or a dataset, both of
    which may have any sort of attribute.
--------------------------------------------------------------------------*/
hid_t
H5Aopen_idx(hid_t loc_id, unsigned idx)
{
    H5G_loc_t	loc;	        /* Object location */
    H5A_t       *attr = NULL;   /* Attribute opened */
    hid_t	ret_value;

    FUNC_ENTER_API(H5Aopen_idx, FAIL)
    H5TRACE2("i", "iIu", loc_id, idx);

    /* check arguments */
    if(H5I_ATTR == H5I_get_type(loc_id))
	HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "location is not valid for an attribute")
    if(H5G_loc(loc_id, &loc) < 0)
	HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a location")

    /* Open the attribute in the object header */
    if(NULL == (attr = H5A_open_by_idx(&loc, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, (hsize_t)idx, H5P_LINK_ACCESS_DEFAULT, H5AC_ind_dxpl_id)))
        HGOTO_ERROR(H5E_ATTR, H5E_CANTOPENOBJ, FAIL, "unable to open attribute")

    /* Register the attribute and get an ID for it */
    if((ret_value = H5I_register(H5I_ATTR, attr)) < 0)
        HGOTO_ERROR(H5E_ATOM, H5E_CANTREGISTER, FAIL, "unable to register attribute for ID")

done:
    /* Cleanup on failure */
    if(ret_value < 0)
        if(attr && H5A_close(attr) < 0)
            HDONE_ERROR(H5E_ATTR, H5E_CANTFREE, FAIL, "can't close attribute")

    FUNC_LEAVE_API(ret_value)
} /* H5Aopen_idx() */


/*--------------------------------------------------------------------------
 NAME
    H5Aget_num_attrs
 PURPOSE
    Determines the number of attributes attached to an object
 NOTE
    Deprecated in favor of H5Oget_info[_by_idx]
 USAGE
    int H5Aget_num_attrs (loc_id)
        hid_t loc_id;       IN: Object (dataset or group) to be queried
 RETURNS
    Number of attributes on success, negative on failure
 DESCRIPTION
        This function returns the number of attributes attached to a dataset or
    group, 'location_id'.
--------------------------------------------------------------------------*/
int
H5Aget_num_attrs(hid_t loc_id)
{
    H5O_loc_t    	*loc;	/* Object location for attribute */
    void           	*obj;
    int			ret_value;

    FUNC_ENTER_API(H5Aget_num_attrs, FAIL)
    H5TRACE1("Is", "i", loc_id);

    /* check arguments */
    if(H5I_FILE == H5I_get_type(loc_id) || H5I_ATTR == H5I_get_type(loc_id))
	HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "location is not valid for an attribute")
    if(NULL == (obj = H5I_object(loc_id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADATOM, FAIL, "illegal object atom")
    switch(H5I_get_type (loc_id)) {
        case H5I_DATASET:
            if(NULL == (loc = H5D_oloc((H5D_t*)obj)))
                HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, FAIL, "can't get location for object")
            break;

        case H5I_DATATYPE:
            if(NULL == (loc = H5T_oloc((H5T_t*)obj)))
                HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "target datatype is not committed")
            break;

        case H5I_GROUP:
            if(NULL == (loc = H5G_oloc((H5G_t*)obj)))
                HGOTO_ERROR(H5E_ATTR, H5E_CANTGET, FAIL, "can't get location for object")
            break;

        default:
            HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "inappropriate attribute target")
    } /*lint !e788 All appropriate cases are covered */

    /* Look up the # of attributes for the object */
    if((ret_value = H5O_attr_count(loc, H5AC_ind_dxpl_id)) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_CANTCOUNT, FAIL, "can't get attribute count for object")

done:
    FUNC_LEAVE_API(ret_value)
} /* H5Aget_num_attrs() */


/*-------------------------------------------------------------------------
 * Function:	H5Arename
 *
 * Purpose:     Rename an attribute
 *
 * Return:	Success:             Non-negative
 *		Failure:             Negative
 *
 * Programmer:	Raymond Lu
 *              October 23, 2002
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Arename(hid_t loc_id, const char *old_name, const char *new_name)
{
    H5G_loc_t	loc;	                /* Object location */
    herr_t	ret_value = SUCCEED;    /* Return value */

    FUNC_ENTER_API(H5Arename, FAIL)
    H5TRACE3("e", "iss", loc_id, old_name, new_name);

    /* check arguments */
    if(!old_name || !new_name)
	HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "name is nil")
    if(H5I_ATTR == H5I_get_type(loc_id))
	HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "location is not valid for an attribute")
    if(H5G_loc(loc_id, & loc) < 0)
	HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a location")

    /* Avoid thrashing things if the names are the same */
    if(HDstrcmp(old_name, new_name))
        /* Call attribute rename routine */
        if(H5O_attr_rename(loc.oloc, H5AC_dxpl_id, old_name, new_name) < 0)
            HGOTO_ERROR(H5E_ATTR, H5E_CANTRENAME, FAIL, "can't rename attribute")

done:
    FUNC_LEAVE_API(ret_value)
} /* H5Arename() */


/*--------------------------------------------------------------------------
 NAME
    H5Aiterate
 PURPOSE
    Calls a user's function for each attribute on an object
 USAGE
    herr_t H5Aiterate (loc_id, attr_num, op, data)
        hid_t loc_id;       IN: Object (dataset or group) to be iterated over
        unsigned *attr_num; IN/OUT: Starting (IN) & Ending (OUT) attribute number
        H5A_operator_t op;  IN: User's function to pass each attribute to
        void *op_data;      IN/OUT: User's data to pass through to iterator operator function
 RETURNS
        Returns a negative value if something is wrong, the return value of the
    last operator if it was non-zero, or zero if all attributes were processed.

 DESCRIPTION
        This function interates over the attributes of dataset or group
    specified with 'loc_id'.  For each attribute of the object, the
    'op_data' and some additional information (specified below) are passed
    to the 'op' function.  The iteration begins with the '*attr_number'
    object in the group and the next attribute to be processed by the operator
    is returned in '*attr_number'.
        The operation receives the ID for the group or dataset being iterated
    over ('loc_id'), the name of the current attribute about the object
    ('attr_name') and the pointer to the operator data passed in to H5Aiterate
    ('op_data').  The return values from an operator are:
        A. Zero causes the iterator to continue, returning zero when all
            attributes have been processed.
        B. Positive causes the iterator to immediately return that positive
            value, indicating short-circuit success.  The iterator can be
            restarted at the next attribute.
        C. Negative causes the iterator to immediately return that value,
            indicating failure.  The iterator can be restarted at the next
            attribute.
--------------------------------------------------------------------------*/
herr_t
H5Aiterate(hid_t loc_id, unsigned *attr_num, H5A_operator_t op, void *op_data)
{
    H5A_attr_iter_op_t  attr_op;        /* Attribute operator */
    hsize_t		start_idx;      /* Index of attribute to start iterating at */
    hsize_t		last_attr;      /* Index of last attribute examined */
    herr_t	        ret_value;      /* Return value */

    FUNC_ENTER_API(H5Aiterate, FAIL)
    H5TRACE4("e", "i*Iuxx", loc_id, attr_num, op, op_data);

    /* check arguments */
    if(H5I_ATTR == H5I_get_type(loc_id))
	HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "location is not valid for an attribute")

    /* Build attribute operator info */
    attr_op.op_type = H5A_ATTR_OP_APP;
    attr_op.u.app_op = op;

    /* Call attribute iteration routine */
    last_attr = start_idx = (hsize_t)(attr_num ? *attr_num : 0);
    if((ret_value = H5O_attr_iterate(loc_id, H5AC_ind_dxpl_id, H5_INDEX_CRT_ORDER, H5_ITER_INC, start_idx, &last_attr, &attr_op, op_data)) < 0)
        HERROR(H5E_ATTR, H5E_BADITER, "error iterating over attributes");

    /* Set the last attribute information */
    if(attr_num)
        *attr_num = (unsigned)last_attr;

done:
    FUNC_LEAVE_API(ret_value)
} /* H5Aiterate() */


/*--------------------------------------------------------------------------
 NAME
    H5Adelete
 PURPOSE
    Deletes an attribute from a location
 USAGE
    herr_t H5Adelete (loc_id, name)
        hid_t loc_id;       IN: Object (dataset or group) to have attribute deleted from
        const char *name;   IN: Name of attribute to delete
 RETURNS
    Non-negative on success/Negative on failure
 DESCRIPTION
    This function removes the named attribute from a dataset or group.
--------------------------------------------------------------------------*/
herr_t
H5Adelete(hid_t loc_id, const char *name)
{
    H5G_loc_t	loc;		        /* Object location */
    herr_t	ret_value = SUCCEED;    /* Return value */

    FUNC_ENTER_API(H5Adelete, FAIL)
    H5TRACE2("e", "is", loc_id, name);

    /* check arguments */
    if(H5I_ATTR == H5I_get_type(loc_id))
	HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "location is not valid for an attribute")
    if(H5G_loc(loc_id, &loc) < 0)
	HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a location")
    if(!name || !*name)
	HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "no name")

    /* Delete the attribute from the location */
    if(H5O_attr_remove(loc.oloc, name, H5AC_dxpl_id) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_CANTDELETE, FAIL, "unable to delete attribute")

done:
    FUNC_LEAVE_API(ret_value)
} /* H5Adelete() */

