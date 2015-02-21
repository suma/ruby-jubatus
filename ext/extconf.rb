require 'mkmf'

have_header('jubatus/config.hpp')

pkg_config('jubatus')
if (/darwin/ =~ RUBY_PLATFORM) != nil
  $CXXFLAGS = "-std=c++11"
end
create_makefile('ruby-jubatus/jubatus_ext')
