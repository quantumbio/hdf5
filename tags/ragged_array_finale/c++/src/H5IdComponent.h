#ifndef _IdComponent_H
#define _IdComponent_H

// IdComponent provides a mechanism to handle
// reference counting for an identifier of any HDF5 object.

#ifndef H5_NO_NAMESPACE
namespace H5 {
#endif

class IdComponent {
   public:
	// Parent classes must reset the current IdComponent copy
	// before setting new id to control reference count
	void setId( hid_t new_id );

	// Pure virtual function so appropriate close function can
	// be called by subclasses' for the corresponding object
	virtual void p_close() const = 0;

	// Creates an object to hold an HDF5 identifier
	IdComponent( const hid_t h5_id );

	// Copy constructor: makes copy of the original IdComponent object.
	IdComponent( const IdComponent& original );

	// Gets the value of IdComponent's data member
	hid_t getId () const;

	// Increment reference counter
	void incRefCount();

	// Decrement reference counter
	void decRefCount();

	// Get the reference counter to this identifier
	int getCounter();

	// Decrements the reference counter then determines if there are no more
	// reference to this object
	bool noReference();

	// Resets this IdComponent instance
	//template <class Type>
	//void reset( Type* parent );
	void reset();
	void resetId();

	// Destructor
	virtual ~IdComponent();

   protected:
	hid_t id;	// HDF5 object id
	RefCounter* ref_count; // used to keep track of the
	                              // number of copies of an object

	// Assignment operator
	IdComponent& operator=( const IdComponent& rhs );

	// Default constructor
	IdComponent();

}; // end class IdComponent

// BMR - including template member function implementation here to 
// prevent compilation errors.  When the compilers support template
// member functions in *.C files, move them to IdComponent.C.

// This function makes sure that this IdComponent instance actually
// represents an HDF5 component and that this HDF5 component is no longer
// referenced, then calls the parent function p_close to close the
// appropriate HDF5 component.  In addition, this identifier instance 
// will delete itself.
// Type is the class of the instance to whom this IdComponent object
// belongs.
/* 11/10/00 - BMR: commented this member function because many compilers
   still have no support for member template function.  The function is
   replaced by resetIdComponent in H5Idtemplates.h
template <class Type>
void IdComponent::reset( Type* parent ) 
{
   if( ref_count->noReference())  // ref_count is decremented here
   {
      if( id > 0 )
         parent->p_close();  // which p_close depends on whom this 
			     // IdComponent object belongs to
      delete ref_count; // delete the reference counter
      delete this;	// this IdComponent object deletes itself
   }
}
*/
#ifndef H5_NO_NAMESPACE
}
#endif
#endif
