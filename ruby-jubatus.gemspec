# encoding: utf-8
$:.push File.expand_path('../lib', __FILE__)
require 'ruby-jubatus/version.rb'

Gem::Specification.new do |gem|

  gem.name        = "ruby-jubatus"
  gem.version     = JubaRuby::Version
  gem.authors     = ["Shuzo Kashihara"]
  gem.email       = %q{suma@sourceforge.jp}
  gem.description = "Jubatus native extension for Ruby"
  gem.summary     = gem.description

  gem.homepage      = "https://github.com/suma/ruby-jubatus"
  gem.has_rdoc      = false
  gem.files         = `git ls-files`.split("\n")
  #gem.test_files    = `git ls-files -- {test,spec,features}/*`.split("\n")
  #gem.executables   = `git ls-files -- bin/*`.split("\n").map{ |f| File.basename(f) }
  gem.require_paths = ['lib']

  gem.extensions = ["ext/extconf.rb"]

  gem.required_ruby_version = '>= 2.1.0'
end
