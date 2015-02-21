#include "jubatus.hpp"
#include "storage.hpp"
#include <jubatus/core/storage/storage_factory.hpp>
#include <jubatus/core/storage/local_storage.hpp>

using jubatus::core::storage::storage_base;
using jubatus::core::storage::local_storage;
using jubatus::core::storage_ptr;

namespace JubaRuby {
namespace Storage {

VALUE rb_mStorage;
VALUE rb_cBasicStorage;

namespace {
struct storage_t {
    storage_ptr storage;
};

void storage_free(void* ptr)
{
    storage_t* p = static_cast<storage_t*>(ptr);
    if (p) {
        p->~storage_t();
        ruby_xfree(p);
    }
}

storage_t* get_storage(VALUE self)
{
    storage_t* p;
    Data_Get_Struct(self, storage_t, p);
    return p;
}

VALUE storage_alloc(VALUE klass)
{
    storage_t* p = static_cast<storage_t*>(ruby_xmalloc(sizeof(storage_t)));
    new(p) storage_t;
    return Data_Wrap_Struct(klass, 0, storage_free, p);
}

VALUE storage_init(VALUE self)
{
    // TODO: switch local_storage_mixture
    storage_t* p = get_storage(self);
    if (p) {
        p->storage.reset(new local_storage);
    }
    return self;
}

VALUE storage_type(VALUE self)
{
    storage_ptr s = get_storage_ptr(self);
    std::string type = s->type();
    return rb_str_new(type.c_str(), type.size());
}

}   // namespace

// for regression and classifier
storage_ptr get_storage_ptr(VALUE self)
{
    storage_t* p;
    Data_Get_Struct(self, storage_t, p);
    return p->storage;
}

void Initialize()
{
    rb_mStorage = rb_define_module_under(rb_mJubatus, "Storage");
    rb_cBasicStorage = rb_define_class_under(rb_mStorage, "BasicStorage", rb_cObject);

    rb_define_alloc_func(rb_cBasicStorage, storage_alloc);
    rb_define_method(rb_cBasicStorage, "initialize", RUBY_METHOD_FUNC(storage_init), 0);
    rb_define_method(rb_cBasicStorage, "type", RUBY_METHOD_FUNC(storage_type), 0);
}

}   // namespace
}   // namespace
