#ifndef _DYNAMIC_OBJECT_H
#define _DYNAMIC_OBJECT_H

#include <string>

#include "dynamic-parameters.h"

/**
 * Virtual class for objects supporting dynamic parameter
 * control.
 *
 * @author Kai Vehmanen
 */
template<class T>
class DYNAMIC_OBJECT : public DYNAMIC_PARAMETERS<T> {

 public:

  /**
   * Object name. Identifies the type represented by dynamic parameters.
   * Must be implemented in subclasses.
   */
  virtual string name(void) const = 0;

  /**
   * Virtual method that clones the current object and returns 
   * a pointer to it. This must be implemented by all subclasses!
   */
  virtual DYNAMIC_OBJECT* clone(void) = 0;

  /**
   * Virtual method that creates a new object of current type.
   * This must be implemented by all subclasses!
   */
  virtual DYNAMIC_OBJECT* new_expr(void) = 0;

  virtual ~DYNAMIC_OBJECT (void) { }
};

#endif
