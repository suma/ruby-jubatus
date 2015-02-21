#include "jubatus.hpp"
#include "feature_extraction.hpp"

#include <jubatus/util/text/json.h>
#include <jubatus/core/fv_converter/datum_to_fv_converter.hpp>
#include <jubatus/core/fv_converter/converter_config.hpp>

using jubatus::core::fv_converter::converter_config;

namespace JubaRuby {
namespace FeatureExtraction {

VALUE rb_mFeatureExtraction;
VALUE rb_cConverter;

namespace {

typedef jubatus::core::fv_converter::datum_to_fv_converter fv_converter;

void conv_free(fv_converter* p)
{
    p->~fv_converter();
    ruby_xfree(p);
}

VALUE conv_alloc(VALUE klass)
{
    fv_converter* p = static_cast<fv_converter*>(ruby_xmalloc(sizeof(fv_converter)));
    new(p) fv_converter;
    return Data_Wrap_Struct(klass, 0, conv_free, p);
}

fv_converter* conv_get_ptr(VALUE self)
{
    fv_converter* p;
    Data_Get_Struct(self, fv_converter, p);
    return p;
}

VALUE conv_new(VALUE self, VALUE c)
{
    return self;
}

VALUE conv_from_config(VALUE self, VALUE json_str)
{
    std::string config_str(RSTRING_PTR(json_str), RSTRING_LEN(json_str));

    VALUE obj = rb_obj_alloc(rb_cConverter);
    converter_config c;
    try {
        std::stringstream ss(config_str);
        ss >> jubatus::util::text::json::via_json(c);

        fv_converter* p = conv_get_ptr(obj);
        initialize_converter(c, *p);
    } catch (const std::bad_cast& e) {
        rb_raise(rb_eArgError, "faield to parse json: %s", e.what());
    } catch (const std::exception& e) {
        rb_raise(rb_eArgError, "faield to parse json: %s", e.what());
    }

    return obj;
}

VALUE conv_convert(VALUE self, VALUE dat)
{
    using jubatus::core::common::sfv_t;
    using jubatus::core::fv_converter::datum;

    if (!rb_obj_is_instance_of(dat, rb_cDatum)) {
        rb_raise(rb_eArgError, "arumgnet must be Datum");
    }

    VALUE fv = rb_obj_alloc(rb_cFeatureVector);
    sfv_t* sfv = get_sfv_ptr(fv);
    datum* d = get_datum_ptr(dat);

    fv_converter* conv = conv_get_ptr(self);
    conv->convert(*d, *sfv);

    return fv;
}

VALUE conv_convert_w(VALUE self, VALUE dat)
{
    using jubatus::core::common::sfv_t;
    using jubatus::core::fv_converter::datum;

    if (!rb_obj_is_instance_of(dat, rb_cDatum)) {
        rb_raise(rb_eArgError, "arumgnet must be Datum");
    }

    VALUE fv = rb_obj_alloc(rb_cFeatureVector);
    sfv_t* sfv = get_sfv_ptr(fv);
    datum* d = get_datum_ptr(dat);

    fv_converter* conv = conv_get_ptr(self);
    conv->convert_and_update_weight(*d, *sfv);

    return fv;
}

}   // namespace

void Initialize()
{
    rb_mFeatureExtraction = rb_define_module_under(rb_mJubatus, "FeatureExtraction");

    rb_cConverter = rb_define_class_under(rb_mFeatureExtraction, "Converter", rb_cObject);

    rb_define_alloc_func(rb_cConverter, conv_alloc);
    rb_define_method(rb_cConverter, "initialize", RUBY_METHOD_FUNC(conv_new), 0);
    rb_define_method(rb_cConverter, "convert", RUBY_METHOD_FUNC(conv_convert), 1);
    rb_define_method(rb_cConverter, "convert_and_update_weight", RUBY_METHOD_FUNC(conv_convert_w), 1);

    rb_define_singleton_method(rb_mFeatureExtraction, "converter_from_config", RUBY_METHOD_FUNC(conv_from_config), 1);
}

}   // Classifier namespace
}   // JubaRuby namespace

