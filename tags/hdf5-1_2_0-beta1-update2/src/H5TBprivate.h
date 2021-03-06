/****************************************************************************
 * NCSA HDF								    *
 * Software Development Group						    *
 * National Center for Supercomputing Applications			    *
 * University of Illinois at Urbana-Champaign				    *
 * 605 E. Springfield, Champaign IL 61820				    *
 *									    *
 * For conditions of distribution and use, see the accompanying		    *
 * hdf/COPYING file.							    *
 *									    *
 ****************************************************************************/

/*
 * This file contains private information about the H5TB module
 */
#ifndef _H5TBprivate_H
#define _H5TBprivate_H

/* Functions defined in H5TB.c */
__DLL__ hid_t H5TB_get_buf(hsize_t size, hbool_t resize, void **ptr);
__DLL__ void *H5TB_buf_ptr(hid_t tbid);
__DLL__ herr_t H5TB_resize_buf(hid_t tbid, hsize_t size, void **ptr);
__DLL__ herr_t H5TB_garbage_coll(void);
__DLL__ herr_t H5TB_release_buf(hid_t tbid);

#endif
