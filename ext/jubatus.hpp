#ifndef JUBARUBY_JUBATUS_EXT_H_
#define JUBARUBY_JUBATUS_EXT_H_

#include <ruby.h>
#include <jubatus/core/common/type.hpp>
#include <jubatus/core/fv_converter/datum.hpp>

namespace JubaRuby {
extern VALUE rb_mJubatus;
extern VALUE rb_cDatum;
extern VALUE rb_cFeatureVector;

jubatus::core::common::sfv_t*
    get_sfv_ptr(VALUE self);

jubatus::core::fv_converter::datum*
    get_datum_ptr(VALUE self);

}

extern "C" {
void Init_jubatus_ext();
}

#endif
