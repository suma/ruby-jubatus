#include "jubatus.hpp"
#include "classifier.hpp"
#include "storage.hpp"
#include "feature_extraction.hpp"
#include <jubatus/core/classifier/classifier_base.hpp>
#include <jubatus/core/classifier/classifier_config.hpp>

// include each algorithm without factory
#include <jubatus/core/classifier/classifier.hpp>

using jubatus::core::classifier::perceptron;
using jubatus::core::classifier::passive_aggressive;
using jubatus::core::classifier::passive_aggressive_1;
using jubatus::core::classifier::passive_aggressive_2;
using jubatus::core::classifier::confidence_weighted;
using jubatus::core::classifier::arow;
using jubatus::core::classifier::normal_herd;
using jubatus::core::classifier::classifier_base;
using jubatus::core::classifier::classifier_config;
using jubatus::core::storage::storage_base;
using jubatus::core::storage_ptr;

namespace JubaRuby {
namespace Classifier {
VALUE rb_cClassifier;   // multiclass classifier
VALUE rb_cAlgorithm;    // multiclass algorithm base
VALUE rb_cConfig;

namespace {
VALUE rb_cPerceptron;
VALUE rb_cPA;
VALUE rb_cPA1;
VALUE rb_cPA2;
VALUE rb_cCW;
VALUE rb_cAROW;
VALUE rb_cNHERD;

void config_free(classifier_config* p)
{
    p->~classifier_config();
    ruby_xfree(p);
}

VALUE config_alloc(VALUE klass)
{
    classifier_config* p = static_cast<classifier_config*>(ruby_xmalloc(sizeof(classifier_config)));
    new(p) classifier_config;
    return Data_Wrap_Struct(klass, 0, config_free, p);
}

classifier_config* config_get_ptr(VALUE self)
{
    classifier_config* p;
    Data_Get_Struct(self, classifier_config, p);
    return p;
}

VALUE config_c_set(VALUE self, VALUE c)
{
    classifier_config* conf = config_get_ptr(self);
    conf->regularization_weight = static_cast<float>(NUM2DBL(c));
    rb_iv_set(self, "@regularization_weight", rb_float_new(conf->regularization_weight));
    return rb_float_new(conf->regularization_weight);
}

VALUE config_c_get(VALUE self)
{
    return rb_iv_get(self, "@regularization_weight");
}

VALUE config_new(VALUE self, VALUE c)
{
    config_c_set(self, c);
    return self;
}

typedef struct {
    classifier_base* algorithm;
} algorithm_t;

void algorithm_free(algorithm_t* p)
{
    delete p->algorithm;
    ruby_xfree(p);
}

algorithm_t* get_algorithm_ptr(VALUE self)
{
    algorithm_t* p;
    Data_Get_Struct(self, algorithm_t, p);
    return p;
}

VALUE algorithm_alloc(VALUE klass)
{
    algorithm_t* p = static_cast<algorithm_t*>(ruby_xmalloc(sizeof(algorithm_t)));
    p->algorithm = NULL;
    return Data_Wrap_Struct(klass, 0, algorithm_free, p);
}

#define DEFINE_CLASSIFIER_NEW(prefix, class_name) \
    VALUE prefix##_new(VALUE self, VALUE storage) \
    { \
        storage_ptr ptr = Storage::get_storage_ptr(storage); \
        get_algorithm_ptr(self)->algorithm = new class_name(ptr); \
        rb_iv_set(self, "@classifier_config", Qnil); \
        return self; \
    } \
    VALUE prefix##_name(VALUE self) \
    { \
        std::string name = get_algorithm_ptr(self)->algorithm->name(); \
        return rb_str_new(name.c_str(), name.size()); \
    }

#define DEFINE_CLASSIFIER_NEW2(prefix, class_name) \
    VALUE prefix##_new(VALUE self, VALUE storage, VALUE config) \
    { \
        storage_ptr ptr = Storage::get_storage_ptr(storage); \
        classifier_config* conf = config_get_ptr(config); \
        get_algorithm_ptr(self)->algorithm = new class_name(*conf, ptr); \
        rb_iv_set(self, "@classifier_config", rb_obj_clone(config)); \
        return self; \
    } \
    VALUE prefix##_name(VALUE self) \
    { \
        std::string name = get_algorithm_ptr(self)->algorithm->name(); \
        return rb_str_new(name.c_str(), name.size()); \
    }

DEFINE_CLASSIFIER_NEW(perceptron, perceptron)
DEFINE_CLASSIFIER_NEW(pa, passive_aggressive)
DEFINE_CLASSIFIER_NEW2(pa1, passive_aggressive_1)
DEFINE_CLASSIFIER_NEW2(pa2, passive_aggressive_2)
DEFINE_CLASSIFIER_NEW2(cw, confidence_weighted)
DEFINE_CLASSIFIER_NEW2(arow, arow)
DEFINE_CLASSIFIER_NEW2(nherd, normal_herd)

VALUE classifier_new(int argc, VALUE* argv, VALUE self)
{
    if (argc < 1) {
        rb_raise(rb_eArgError, "Missing argument");
    }

    VALUE algorithm = argv[0];
    if (!rb_obj_is_kind_of(algorithm, rb_cAlgorithm)) {
        rb_raise(rb_eTypeError, "invalid type algorithm");
    }
    rb_iv_set(self, "@algorithm", algorithm);

    if (argc > 1) {
        VALUE fvc = argv[1];
        if (!rb_obj_is_instance_of(fvc, FeatureExtraction::rb_cConverter)) {
            rb_raise(rb_eTypeError, "invalid type FeatureExtraction::Converter");
        }
        rb_iv_set(self, "@converter", fvc);
    }

    return self;
}

VALUE classifier_train(VALUE self, VALUE label, VALUE dat)
{
    using jubatus::core::common::sfv_t;
    VALUE fv;
    if (rb_obj_is_instance_of(dat, rb_cDatum)) {
        rb_raise(rb_eArgError, "Current datum input not supported");
    } else if (!rb_obj_is_instance_of(dat, rb_cFeatureVector)) {
        rb_raise(rb_eTypeError, "Allows only Datum or FeatureVector");
    }

    fv = dat;
    sfv_t* sfv = get_sfv_ptr(fv);

    algorithm_t *algo = get_algorithm_ptr(rb_iv_get(self, "@algorithm"));
    if (!algo || !algo->algorithm) {
        // TODO: fix exception type
        rb_raise(rb_eTypeError, "failed to get @algorithm");
    }
    classifier_base* c = algo->algorithm;

    // call train
    std::string label_str(RSTRING_PTR(label), RSTRING_LEN(label));
    c->train(*sfv, label_str);

    return Qnil;
}

VALUE classifier_classify(VALUE self, VALUE dat)
{
    using jubatus::core::common::sfv_t;
    using jubatus::core::classifier::classify_result;
    VALUE fv;

    if (rb_obj_is_instance_of(dat, rb_cDatum)) {
        rb_raise(rb_eArgError, "Current datum input not supported");
    } else if (!rb_obj_is_instance_of(dat, rb_cFeatureVector)) {
        rb_raise(rb_eTypeError, "Allows only Datum or FeatureVector");
    }

    fv = dat;
    sfv_t* sfv = get_sfv_ptr(fv);
    classify_result scores;
    algorithm_t *algo = get_algorithm_ptr(rb_iv_get(self, "@algorithm"));
    if (!algo || !algo->algorithm) {
        // TODO: fix exception type
        rb_raise(rb_eTypeError, "failed to get @algorithm");
    }
    classifier_base* c = algo->algorithm;
    c->classify_with_scores(*sfv, scores);

    VALUE rb_cResult = rb_path2class("JubaRuby::MulticalssResult");
    VALUE rb_cScore = rb_path2class("JubaRuby::MulticalssResult::Score");

    VALUE result_ary = rb_ary_new2(scores.size());
    for (classify_result::iterator it = scores.begin(), end = scores.end();
            it != end; ++it) {
        VALUE score = rb_obj_alloc(rb_cScore);
        VALUE argv[2] = {
            rb_str_new(it->label.data(), it->label.size()),
            rb_float_new(it->score)
        };
        rb_obj_call_init(score, 2, argv);
        rb_ary_push(result_ary, score);
    }

    VALUE argv[1] = { result_ary };
    VALUE result = rb_obj_alloc(rb_cResult);
    rb_obj_call_init(result, 1, argv);
    return result;
}

}   // namespace

void Initialize()
{
    rb_cClassifier = rb_define_class_under(rb_mJubatus, "Classifier", rb_cObject);

    rb_cAlgorithm = rb_define_class_under(rb_cClassifier, "Algorithm", rb_cObject);
    rb_cConfig = rb_define_class_under(rb_cClassifier, "Config", rb_cObject);

    rb_cPerceptron = rb_define_class_under(rb_cClassifier, "Perceptron", rb_cAlgorithm);
    rb_cPA = rb_define_class_under(rb_cClassifier, "PA", rb_cAlgorithm);
    rb_cPA1 = rb_define_class_under(rb_cClassifier, "PA1", rb_cAlgorithm);
    rb_cPA2 = rb_define_class_under(rb_cClassifier, "PA2", rb_cAlgorithm);
    rb_cCW = rb_define_class_under(rb_cClassifier, "CW", rb_cAlgorithm);
    rb_cAROW = rb_define_class_under(rb_cClassifier, "AROW", rb_cAlgorithm);
    rb_cNHERD = rb_define_class_under(rb_cClassifier, "NHERED", rb_cAlgorithm);

    rb_define_alloc_func(rb_cConfig, config_alloc);
    rb_define_method(rb_cConfig, "initialize", RUBY_METHOD_FUNC(config_new), 1);
    rb_define_method(rb_cConfig, "regularization_weight", RUBY_METHOD_FUNC(config_c_get), 0);
    rb_define_method(rb_cConfig, "regularization_weight=", RUBY_METHOD_FUNC(config_c_set), 1);

#define DEFINE_ALGORITHM_INIT(name, class_name, num) \
    rb_define_alloc_func(class_name, algorithm_alloc); \
    rb_define_method(class_name, "initialize", RUBY_METHOD_FUNC(name##_new), num); \
    rb_define_method(class_name, "name", RUBY_METHOD_FUNC(name##_name), 0);

    DEFINE_ALGORITHM_INIT(perceptron, rb_cPerceptron, 1);
    DEFINE_ALGORITHM_INIT(pa, rb_cPA, 1);
    DEFINE_ALGORITHM_INIT(pa1, rb_cPA1, 2);
    DEFINE_ALGORITHM_INIT(pa2, rb_cPA2, 2);
    DEFINE_ALGORITHM_INIT(nherd, rb_cNHERD, 2);
    DEFINE_ALGORITHM_INIT(arow, rb_cAROW, 2);
#undef DEFINE_ALGORITHM_INIT

    rb_define_method(rb_cClassifier, "initialize", RUBY_METHOD_FUNC(classifier_new), -1);
    rb_define_method(rb_cClassifier, "train", RUBY_METHOD_FUNC(classifier_train), 2);
    rb_define_method(rb_cClassifier, "classify", RUBY_METHOD_FUNC(classifier_classify), 1);
}

}   // Classifier namespace
}   // JubaRuby namespace

