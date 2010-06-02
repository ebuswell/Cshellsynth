require 'mkmf'

$CFLAGS << " #{ENV["CFLAGS"]}"
$CFLAGS << " -Wall"

extension_name = 'cshellsynth'

dir_config(extension_name)

if(!have_library('jack', 'jack_client_open'))
  exit(0)
end

if(!have_library('cshellsynth', 'jclient_init'))
  exit(0)
end

create_makefile(extension_name)
