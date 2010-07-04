require 'mkmf'

$CFLAGS << " -Wall"

d = Dir.open($curdir)
head = "/* WARNING: autogenerated by extconf.rb */\n"
foot = "\nvoid Init_cshellsynth() {\n"
order = ["jackruby", "synths", "synth", "controllers", "controller", "mixer", "filters", "filter"]
order.each do |f|
  head += "void Init_" + f + "();\n"
  foot += "    Init_" + f + "();\n"
end
d.each do |f|
  if f.match ".*\.c\$"
    if((! order.member? f[0...-2]) && (f[0...-2] != "cshellsynth"))
      head += "void Init_" + f[0...-2] + "();\n"
      foot += "    Init_" + f[0...-2] + "();\n"
    end
  end
end

foot += "}\n"
if File.exist? "#$curdir/cshellsynth.c"
  File.delete("#$curdir/cshellsynth.c")
end
lib_main = File.open("#$curdir/cshellsynth.c", "w")
lib_main.puts head + foot
lib_main.close


$LIBPATH << "#$curdir/../src/.libs/"
$INCFLAGS << " " << "-I#$curdir/../src".quote

if(!have_library('jack', 'jack_client_open'))
  exit(1);
end

$libs = append_library($libs, 'cshellsynth')

create_makefile('cshellsynth')
