# -*- coding: utf-8 -*-

require 'ruby-jubatus'
s = JubaRuby::Storage::BasicStorage.new
config = JubaRuby::Classifier::Config.new(1.0)
arow = JubaRuby::Classifier::AROW.new(JubaRuby::Storage::BasicStorage.new, config)

pa = JubaRuby::Classifier::PA.new(s)

classifier = JubaRuby::Classifier.new(arow)

datum = JubaRuby::Datum.new
fv = JubaRuby::FeatureVector.new

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
classifier_fv = JubaRuby::Classifier.new(pa)

p s
puts "s.type = #{s.type}"
p config
#p pa
p arow
puts "arow.name = #{arow.name}"
p classifier
p classifier_fv
p datum
puts "datum.str_size = #{datum.str_size}"
puts "datum.num_size = #{datum.num_size}"
#datum.push_str("key", "value")
datum.push_num("key", 1.0)
puts "datum.str_size = #{datum.str_size}"
puts "datum.num_size = #{datum.num_size}"
p fv
puts "fv.size = #{fv.size}"
p fv[0]
fv << ["key1@*", 1.0]
fv << ["key2@*", 2.0]
p fv[0]
puts "fv.size = #{fv.size}"



p classifier.classify(fv)

tokugawa = %w(家康 秀忠 家光 家綱 綱吉 家宣 家継 吉宗 家重 家治 家斉 家慶 家定 家茂)
ashikaga = %w(尊氏 義詮 義満 義持 義量 義教 義勝 義政 義尚 義稙 義澄 義稙 義晴 義輝 義栄)
hojo = %w(時政 義時 泰時 経時 時頼 長時 政村 時宗 貞時 師時 宗宣 煕時 基時 高時 貞顕)
train = [
  ['徳川', tokugawa], ['足利', ashikaga], ['北条', hojo]
]

train.each { |t|
  label = t[0]
  t[1].each { |name|
    d = JubaRuby::Datum.new
    d.push_str("name", name)
    f = conv.convert(d)
    classifier.train(label, f)
  }
}

test = %w(慶喜 義昭 守時)
test.each { |name|
  puts name
  d = JubaRuby::Datum.new
  d.push_str("name", name)
  f = conv.convert(d)
  res = classifier.classify(f)
  puts "prediction = #{res.prediction.label}, #{res.prediction.score}"
  p res
}



