$CFLAGS = "-Wall"

require 'mkmf'

extension_name = 'jackruby'

dir_config(extension_name)

if(!have_library('jack', 'jack_client_open'))
  exit(0)
end

create_makefile(extension_name)
