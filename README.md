ruby-jubatus
============

Ruby native extension to use Jubatus

## Requirements

- Ruby 2.2.0 (tested!)
- Jubatus core 0.6.0 (patched)


## Install

-  Apply ./patch/jubatus.patch file when installing Jubatus core.
- install gem (needs to set environment envs Installed jubatus.pc, include headers)

```
$ rake build
$ sudo gem install ./pkg/ruby-jubatus-0.0.1.gem
```

## Example

### Basic

Create classifier and fv_conveter(FeatureExtraction::Converter)

```ruby
require 'ruby-jubatus'
classifier = JubaRuby::Classifier::AROW.new(JubaRuby::Storage::BasicStorage.new, JubaRuby::Classifier::Config.new(1.0))
conv = JubaRuby::FeatureExtraction.converter_from_config <<EOS
  {
    "string_filter_types" : {},
    "string_filter_rules" : [],
    "num_filter_types" : {},
    "num_filter_rules" : [],
    "string_types" : {
      "unigram": { "method": "ngram", "char_num": "1" }
    },
    "string_rules" : [
      { "key" : "*", "type" : "unigram", "sample_weight" : "bin", "global_weight" : "bin" }
    ],
    "num_types" : {},
    "num_rules" : [
      { "key" : "*", "type" : "num" }
    ]
  }
EOS
```

### Train

```ruby
d = JubaRuby::Datum.new
d.push_str("key", "string value")
#d.push_num("key", 1.0) 
fv = conv.convert(d)
classifier.train("label", fv)
```

### Classify

```ruby
d = JubaRuby::Datum.new
d.push_str("key", "string value")
#d.push_num("key", 1.0) 
fv = conv.convert(d)
res = classifier.classify("label", fv)
p res.prediction # highest label
p res.results # all results
```

