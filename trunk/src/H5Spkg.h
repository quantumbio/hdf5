/*
 * Copyright (C) 2000 NCSA
 *		      All rights reserved.
 *
 * Programmer:	Quincey Koziol <koziol@ncsa.uiuc.edu>
 *		Thursday, September 28, 2000
 *
 * Purpose:	This file contains declarations which are visible only within
 *		the H5S package.  Source files outside the H5S package should
 *		include H5Sprivate.h instead.
 */
#ifndef H5S_PACKAGE
#error "Do not include this file outside the H5S package!"
#endif

#ifndef _H5Spkg_H
#define _H5Spkg_H

#include "H5Sprivate.h"

/* 
 * Dataspace extent information
 */
/* Simple extent container */
typedef struct H5S_simple_t {
    unsigned rank;         /* Number of dimensions */
    hsize_t *size;      /* Current size of the dimensions */
    hsize_t *max;       /* Maximum size of the dimensions */
#ifdef LATER
    hsize_t *perm;      /* Dimension permutation array */
#endif /* LATER */
} H5S_simple_t;

/* Extent container */
typedef struct {
    H5S_class_t	type;   /* Type of extent */
    hsize_t nelem;      /* Number of elements in extent */
    union {
        H5S_simple_t	simple;	/* Simple dimensionality information  */
    } u;
} H5S_extent_t;

/* 
 * Dataspace selection information
 */
/* Node in point selection list (typedef'd in H5Sprivate.h) */
struct H5S_pnt_node_t {
    hssize_t *pnt;          /* Pointer to a selected point */
    struct H5S_pnt_node_t *next;  /* pointer to next point in list */
};

/* Information about point selection list */
typedef struct {
    H5S_pnt_node_t *head;   /* Pointer to head of point list */
} H5S_pnt_list_t;

/* Information about new-style hyperslab spans */

/* Information a particular hyperslab span */
struct H5S_hyper_span_t {
    hssize_t low, high;         /* Low & high bounds of span */
    hsize_t nelem;              /* Number of elements in span (only needed during I/O) */
    hsize_t pstride;            /* Pseudo-stride from start of previous span (only used during I/O) */
    struct H5S_hyper_span_info_t *down;     /* Pointer to list of spans in next dimension down */
    struct H5S_hyper_span_t *next;     /* Pointer to next span in list */
};

/* Information about a list of hyperslab spans */
struct H5S_hyper_span_info_t {
    unsigned count;                    /* Ref. count of number of spans which share this span */
    struct H5S_hyper_span_info_t *scratch;  /* Scratch pointer (used during copies & as mark during precomputes for I/O) */
    struct H5S_hyper_span_t *head;  /* Pointer to list of spans in next dimension down */
};

/* Information about one dimension in a hyperslab selection */
struct H5S_hyper_dim_t {
    hssize_t start;
    hsize_t  stride;
    hsize_t  count;
    hsize_t  block;
};

/* Information about new-style hyperslab selection */
typedef struct {
    H5S_hyper_dim_t *diminfo;    /* ->[rank] of per-dim selection info */
	/* diminfo only points to one array, which holds the information
	 * for one hyperslab selection. Perhaps this might need to be
	 * expanded into a list of arrays when the H5Sselect_hyperslab's
	 * restriction to H5S_SELECT_SET is removed. */
    H5S_hyper_dim_t *app_diminfo;/* ->[rank] of per-dim selection info */
	/* 'diminfo' points to a [potentially] optimized version of the user's
         * hyperslab information.  'app_diminfo' points to the actual parameters
         * that the application used for setting the hyperslab selection.  These
         * are only used for re-gurgitating the original values used to set the
         * hyperslab to the application when it queries the hyperslab selection
         * information. */
    H5S_hyper_span_info_t *span_lst; /* List of hyperslab span information */
} H5S_hyper_sel_t;

/* Selection information methods */
/* Method to retrieve a list of offset/length sequences for selection */
typedef herr_t (*H5S_sel_get_seq_list_func_t)(const H5S_t *space, unsigned flags,
    H5S_sel_iter_t *iter, size_t elem_size, size_t maxseq, size_t maxbytes,
    size_t *nseq, size_t *nbytes, hsize_t *off, size_t *len);
/* Method to compute number of elements in current selection */
typedef hsize_t (*H5S_sel_get_npoints_func_t)(const H5S_t *space);
/* Method to release current selection */
typedef herr_t (*H5S_sel_release_func_t)(H5S_t *space);
/* Method to initialize iterator for current selection */
typedef herr_t (*H5S_sel_iter_init_func_t)(const H5S_t *space, size_t elmt_size, H5S_sel_iter_t *sel_iter);
/* Method to determine number of elements left in iterator for current selection */
typedef hsize_t (*H5S_sel_iter_nelmts_func_t)(const H5S_sel_iter_t *iter);
/* Method to release iterator for current selection */
typedef herr_t (*H5S_sel_iter_release_func_t)(H5S_sel_iter_t *iter);
/* Method to determine if current selection is valid for dataspace */
typedef htri_t (*H5S_sel_is_valid_func_t)(const H5S_t *space);
/* Method to determine number of bytes required to store current selection */
typedef hssize_t (*H5S_sel_serial_size_func_t)(const H5S_t *space);
/* Method to store current selection in "serialized" form (a byte sequence suitable for storing on disk) */
typedef herr_t (*H5S_sel_serialize_func_t)(const H5S_t *space, uint8_t *buf);
/* Method to determine to smallest n-D bounding box containing the current selection */
typedef herr_t (*H5S_sel_bounds_func_t)(const H5S_t *space, hsize_t *start, hsize_t *end);
/* Method to determine if current selection is contiguous */
typedef htri_t (*H5S_sel_is_contiguous_func_t)(const H5S_t *space);
/* Method to determine if current selection is a single block */
typedef htri_t (*H5S_sel_is_single_func_t)(const H5S_t *space);
/* Method to determine if current selection is "regular" */
typedef htri_t (*H5S_sel_is_regular_func_t)(const H5S_t *space);

/* Selection information object */
typedef struct {
    H5S_sel_type type;  /* Type of selection (list of points or hyperslabs) */
    hssize_t *offset;   /* Offset within the extent (NULL means a 0 offset) */
    hsize_t *order;     /* Selection order.  (NULL means a specific ordering of points) */
    hsize_t num_elem;   /* Number of elements in selection */
    union {
        H5S_pnt_list_t *pnt_lst; /* List of selected points (order is important) */
        H5S_hyper_sel_t hslab;   /* Info about new-style hyperslab selections */
    } sel_info;
    H5S_sel_get_seq_list_func_t get_seq_list;   /* Method to retrieve a list of offset/length sequences for selection */
    H5S_sel_get_npoints_func_t get_npoints;     /* Method to compute number of elements in current selection */
    H5S_sel_release_func_t release;             /* Method to release current selection */
    H5S_sel_iter_init_func_t iter_init;         /* Method to initialize iterator for current selection */
    H5S_sel_iter_nelmts_func_t iter_nelmts;     /* Method to determine number of elements left in iterator for current selection */
    H5S_sel_iter_release_func_t iter_release;   /* Method to release iterator for current selection */
    H5S_sel_is_valid_func_t is_valid;           /* Method to determine if current selection is valid for dataspace */
    H5S_sel_serial_size_func_t serial_size;     /* Method to determine number of bytes required to store current selection */
    H5S_sel_serialize_func_t serialize;         /* Method to store current selection in "serialized" form (a byte sequence suitable for storing on disk) */
    H5S_sel_bounds_func_t bounds;               /* Method to determine to smallest n-D bounding box containing the current selection */
    H5S_sel_is_contiguous_func_t is_contiguous; /* Method to determine if current selection is contiguous */
    H5S_sel_is_single_func_t is_single;         /* Method to determine if current selection is a single block */
    H5S_sel_is_regular_func_t is_regular;       /* Method to determine if current selection is "regular" */
} H5S_select_t;

/* Main dataspace structure (typedef'd in H5Sprivate.h) */
struct H5S_t {
    H5S_extent_t extent;        /* Dataspace extent */
    H5S_select_t select;		/* Dataspace selection */
};

/* Extent functions */
H5_DLL herr_t H5S_close_simple(H5S_simple_t *simple);
H5_DLL herr_t H5S_release_simple(H5S_simple_t *simple);
H5_DLL herr_t H5S_extent_copy(H5S_extent_t *dst, const H5S_extent_t *src);

/* Point select functions */
H5_DLL herr_t H5S_point_iter_init (const H5S_t *space, size_t elmt_size,
        H5S_sel_iter_t *iter);
H5_DLL hsize_t H5S_point_iter_nelmts (const H5S_sel_iter_t *iter);
H5_DLL herr_t H5S_point_iter_release(H5S_sel_iter_t *sel_iter);
H5_DLL herr_t H5S_point_release(H5S_t *space);
H5_DLL hsize_t H5S_point_npoints(const H5S_t *space);
H5_DLL herr_t H5S_point_copy(H5S_t *dst, const H5S_t *src);
H5_DLL htri_t H5S_point_is_valid(const H5S_t *space);
H5_DLL hssize_t H5S_point_serial_size(const H5S_t *space);
H5_DLL herr_t H5S_point_serialize(const H5S_t *space, uint8_t *buf);
H5_DLL herr_t H5S_point_deserialize(H5S_t *space, const uint8_t *buf);
H5_DLL herr_t H5S_point_bounds(const H5S_t *space, hsize_t *start, hsize_t *end);
H5_DLL htri_t H5S_point_is_contiguous(const H5S_t *space);
H5_DLL htri_t H5S_point_is_single(const H5S_t *space);
H5_DLL htri_t H5S_point_is_regular(const H5S_t *space);
H5_DLL herr_t H5S_point_get_seq_list(const H5S_t *space, unsigned flags,
    H5S_sel_iter_t *iter, size_t elem_size, size_t maxseq, size_t maxbytes,
    size_t *nseq, size_t *nbytes, hsize_t *off, size_t *len);

/* "All" select functions */
H5_DLL herr_t H5S_all_iter_init (const H5S_t *space, size_t elmt_size,
        H5S_sel_iter_t *iter);
H5_DLL hsize_t H5S_all_iter_nelmts (const H5S_sel_iter_t *iter);
H5_DLL herr_t H5S_all_iter_release(H5S_sel_iter_t *sel_iter);
H5_DLL herr_t H5S_all_release(H5S_t *space);
H5_DLL hsize_t H5S_all_npoints(const H5S_t *space);
H5_DLL htri_t H5S_all_is_valid(const H5S_t *space);
H5_DLL hssize_t H5S_all_serial_size(const H5S_t *space);
H5_DLL herr_t H5S_all_serialize(const H5S_t *space, uint8_t *buf);
H5_DLL herr_t H5S_all_deserialize(H5S_t *space, const uint8_t *buf);
H5_DLL herr_t H5S_all_bounds(const H5S_t *space, hsize_t *start, hsize_t *end);
H5_DLL htri_t H5S_all_is_contiguous(const H5S_t *space);
H5_DLL htri_t H5S_all_is_single(const H5S_t *space);
H5_DLL htri_t H5S_all_is_regular(const H5S_t *space);
H5_DLL herr_t H5S_all_get_seq_list(const H5S_t *space, unsigned flags,
    H5S_sel_iter_t *iter, size_t elem_size, size_t maxseq, size_t maxbytes,
    size_t *nseq, size_t *nbytes, hsize_t *off, size_t *len);

/* Hyperslab selection functions */
H5_DLL herr_t H5S_hyper_iter_init (const H5S_t *space, size_t elmt_size,
        H5S_sel_iter_t *iter);
H5_DLL hsize_t H5S_hyper_iter_nelmts (const H5S_sel_iter_t *iter);
H5_DLL herr_t H5S_hyper_iter_release(H5S_sel_iter_t *sel_iter);
H5_DLL herr_t H5S_hyper_release(H5S_t *space);
H5_DLL hsize_t H5S_hyper_npoints(const H5S_t *space);
H5_DLL herr_t H5S_hyper_copy(H5S_t *dst, const H5S_t *src);
H5_DLL htri_t H5S_hyper_is_valid(const H5S_t *space);
H5_DLL hssize_t H5S_hyper_serial_size(const H5S_t *space);
H5_DLL herr_t H5S_hyper_serialize(const H5S_t *space, uint8_t *buf);
H5_DLL herr_t H5S_hyper_deserialize(H5S_t *space, const uint8_t *buf);
H5_DLL herr_t H5S_hyper_bounds(const H5S_t *space, hsize_t *start, hsize_t *end);
H5_DLL htri_t H5S_hyper_is_contiguous(const H5S_t *space);
H5_DLL htri_t H5S_hyper_is_single(const H5S_t *space);
H5_DLL htri_t H5S_hyper_is_regular(const H5S_t *space);
H5_DLL herr_t H5S_hyper_get_seq_list(const H5S_t *space, unsigned flags,
    H5S_sel_iter_t *iter, size_t elem_size, size_t maxseq, size_t maxbytes,
    size_t *nseq, size_t *nbytes, hsize_t *off, size_t *len);

/* "None" selection functions */
H5_DLL herr_t H5S_none_iter_init (const H5S_t *space, size_t elmt_size,
        H5S_sel_iter_t *iter);
H5_DLL hsize_t H5S_none_iter_nelmts (const H5S_sel_iter_t *iter);
H5_DLL herr_t H5S_none_iter_release(H5S_sel_iter_t *sel_iter);
H5_DLL herr_t H5S_none_release(H5S_t *space);
H5_DLL hsize_t H5S_none_npoints(const H5S_t *space);
H5_DLL htri_t H5S_none_is_valid(const H5S_t *space);
H5_DLL hssize_t H5S_none_serial_size(const H5S_t *space);
H5_DLL herr_t H5S_none_serialize(const H5S_t *space, uint8_t *buf);
H5_DLL herr_t H5S_none_deserialize(H5S_t *space, const uint8_t *buf);
H5_DLL herr_t H5S_none_bounds(const H5S_t *space, hsize_t *start, hsize_t *end);
H5_DLL htri_t H5S_none_is_contiguous(const H5S_t *space);
H5_DLL htri_t H5S_none_is_single(const H5S_t *space);
H5_DLL htri_t H5S_none_is_regular(const H5S_t *space);
H5_DLL herr_t H5S_none_get_seq_list(const H5S_t *space, unsigned flags,
    H5S_sel_iter_t *iter, size_t elem_size, size_t maxseq, size_t maxbytes,
    size_t *nseq, size_t *nbytes, hsize_t *off, size_t *len);
/* Needed for use in hyperslab code (H5Shyper.c) */
H5_DLL herr_t H5S_select_none(H5S_t *space);

#endif
