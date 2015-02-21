#ifndef JUBARUBY_FEATURE_EXTRACTION_H_
#define JUBARUBY_FEATURE_EXTRACTION_H_

#include <ruby.h>

namespace JubaRuby {
namespace FeatureExtraction {

extern VALUE rb_mFeatureExtraction;
extern VALUE rb_cConverter;

void Initialize();

}   // namespace
}   // namespace

#endif

