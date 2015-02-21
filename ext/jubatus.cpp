
#include "jubatus.hpp"

using jubatus::core::common::sfv_t;
using jubatus::core::fv_converter::datum;

namespace JubaRuby {

VALUE rb_mJubatus;
VALUE rb_cDatum;
VALUE rb_cFeatureVector;

namespace {

void datum_free(datum* p)
{
    p->~datum();
    ruby_xfree(p);
}

VALUE datum_alloc(VALUE klass)
{
    datum* p = static_cast<datum*>(ruby_xmalloc(sizeof(datum)));
    new(p) datum;
    return Data_Wrap_Struct(klass, 0, datum_free, p);
}

VALUE datum_init(VALUE self)
{
    return self;
}

VALUE datum_push_str(VALUE self, VALUE k, VALUE v)
{
    std::string key(RSTRING_PTR(k), RSTRING_LEN(k));
    std::string str(RSTRING_PTR(v), RSTRING_LEN(v));
    datum* p = get_datum_ptr(self);
    p->string_values_.push_back(std::pair<std::string, std::string>(key, str));
    return self;
}

VALUE datum_push_num(VALUE self, VALUE k, VALUE num)
{
    std::string key(RSTRING_PTR(k), RSTRING_LEN(k));
    datum* p = get_datum_ptr(self);
    p->num_values_.push_back(std::pair<std::string, float>(key, NUM2DBL(num)));
    return self;
}

VALUE datum_str_size(VALUE self)
{
    datum* p = get_datum_ptr(self);
    return rb_int_new(p->string_values_.size());
}

VALUE datum_num_size(VALUE self)
{
    datum* p = get_datum_ptr(self);
    return rb_int_new(p->num_values_.size());
}

void sfv_free(sfv_t* p)
{
    p->~sfv_t();
    ruby_xfree(p);
}

VALUE sfv_alloc(VALUE klass)
{
    sfv_t* p = static_cast<sfv_t*>(ruby_xmalloc(sizeof(sfv_t)));
    new(p) sfv_t;
    return Data_Wrap_Struct(klass, 0, sfv_free, p);
}

VALUE sfv_init(VALUE self)
{
    return self;
}

VALUE sfv_aref(VALUE self, VALUE index)
{
    unsigned int i = NUM2UINT(index);
    sfv_t* p = get_sfv_ptr(self);
    if (p->size() > i) {
        VALUE ary = rb_ary_new2(2);
        const std::string& s = (*p)[i].first;
        rb_ary_store(ary, 0, rb_str_new(s.data(), s.size()));
        rb_ary_store(ary, 1, rb_float_new((*p)[i].second));
        return ary;
    }
    return Qnil;
}

VALUE sfv_aset(VALUE self, VALUE index, VALUE v)
{
    if (!rb_obj_is_kind_of(v, rb_cArray)) {
        rb_raise(rb_eTypeError, "invalid type feature");
    }

    if (RARRAY_LEN(v) < 2) {
        rb_raise(rb_eArgError, "feature size differs (%ld should be 2)", RARRAY_LEN(v));
    }

    unsigned int i = NUM2UINT(index);
    sfv_t* p = get_sfv_ptr(self);
    if (p->empty() || p->size() <= i) {
        return Qnil;
    }

    VALUE key = rb_ary_entry(v, 0);
    VALUE num = rb_ary_entry(v, 1);
    if (!rb_obj_is_kind_of(key, rb_cString)) {
        rb_raise(rb_eArgError, "feature key must be string");
    }

    std::string k(RSTRING_PTR(key), RSTRING_LEN(key));
    (*p)[i] = std::pair<std::string, float>(k, NUM2DBL(num));
    return v;
}

VALUE sfv_push(VALUE self, VALUE v)
{
    if (!rb_obj_is_kind_of(v, rb_cArray)) {
        rb_raise(rb_eTypeError, "invalid type feature");
    }

    if (RARRAY_LEN(v) < 2) {
        rb_raise(rb_eArgError, "feature size differs (%ld should be 2)", RARRAY_LEN(v));
    }

    VALUE key = rb_ary_entry(v, 0);
    VALUE num = rb_ary_entry(v, 1);
    if (!rb_obj_is_kind_of(key, rb_cString)) {
        rb_raise(rb_eArgError, "feature key must be string");
        return Qnil;
    }

    std::string k(RSTRING_PTR(key), RSTRING_LEN(key));
    sfv_t* p = get_sfv_ptr(self);
    p->push_back(std::pair<std::string, float>(k, NUM2DBL(num)));
    return v;
}

VALUE sfv_size(VALUE self)
{
    sfv_t* p = get_sfv_ptr(self);
    return rb_int_new(p->size());
}

}   // namespace

datum* get_datum_ptr(VALUE self)
{
    datum* p;
    Data_Get_Struct(self, datum, p);
    return p;
}

sfv_t* get_sfv_ptr(VALUE self)
{
    sfv_t* p;
    Data_Get_Struct(self, sfv_t, p);
    return p;
}

void InitJubaRuby()
{
    rb_mJubatus = rb_define_module("JubaRuby");
    rb_cDatum = rb_define_class_under(rb_mJubatus, "Datum", rb_cObject);
    rb_cFeatureVector = rb_define_class_under(rb_mJubatus, "FeatureVector", rb_cObject);

    rb_define_alloc_func(rb_cDatum, datum_alloc);
    rb_define_method(rb_cDatum, "initialize", RUBY_METHOD_FUNC(datum_init), 0);
    rb_define_method(rb_cDatum, "push_str", RUBY_METHOD_FUNC(datum_push_str), 2);
    rb_define_method(rb_cDatum, "push_num", RUBY_METHOD_FUNC(datum_push_num), 2);
    rb_define_method(rb_cDatum, "str_size", RUBY_METHOD_FUNC(datum_str_size), 0);
    rb_define_method(rb_cDatum, "num_size", RUBY_METHOD_FUNC(datum_str_size), 0);


    rb_define_alloc_func(rb_cFeatureVector, sfv_alloc);
    rb_define_method(rb_cFeatureVector, "initialize", RUBY_METHOD_FUNC(sfv_init), 0);
    rb_define_method(rb_cFeatureVector, "[]", RUBY_METHOD_FUNC(sfv_aref), 1);
    rb_define_method(rb_cFeatureVector, "[]=", RUBY_METHOD_FUNC(sfv_aset), 2);
    rb_define_method(rb_cFeatureVector, "<<", RUBY_METHOD_FUNC(sfv_push), 1);
    rb_define_method(rb_cFeatureVector, "size", RUBY_METHOD_FUNC(sfv_size), 0);
    rb_define_method(rb_cFeatureVector, "length", RUBY_METHOD_FUNC(sfv_size), 0);
}

namespace Classifier { void Initialize(); }
namespace Storage { void Initialize(); }
namespace FeatureExtraction { void Initialize(); }

}   // namespace

extern "C" void Init_jubatus_ext()
{
    JubaRuby::InitJubaRuby();
    JubaRuby::Storage::Initialize();
    JubaRuby::Classifier::Initialize();
    JubaRuby::FeatureExtraction::Initialize();
}

