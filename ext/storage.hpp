#ifndef JUBARUBY_STORAGE_H_
#define JUBARUBY_STORAGE_H_

#include <ruby.h>
#include <jubatus/core/storage/storage_base.hpp>

// Storage for classifier and regression algorithm

namespace JubaRuby {
namespace Storage {

extern VALUE rb_mStorage;
extern VALUE rb_cBasicStorage;

jubatus::core::storage_ptr
    get_storage_ptr(VALUE self);

void InitStorage();

}   // Storage namespace
}   // JubaRuby namespace

#endif
