#
# Build amalgalite and install to site-ruby.
#
# amalgalite's ./ext structure is non-standard and doesn't
# work with the usual setup.rb.
#

require 'rbconfig'
require 'shellwords'
require 'fileutils'

RB_EXE = ARGV.shift
(RB_EXE && test(?x, RB_EXE)) or abort("path to ruby binary '#{RB_EXE}' not found, or not executable")

_rb_dir = ENV['RUBY_INSTALL_PREFIX'].to_s
if _rb_dir.empty?
  if RB_EXE =~ %r{\A(.*)/bin/[^/]+\z}
    _rb_dir = $1
  else
    abort("can't find RUBY_INSTALL_PREFIX via ENV or via RB_EXE")
  end
end

RB_INSTALL_DIR = _rb_dir
(RbConfig::CONFIG['sitelibdir'] =~ %r{\A.*(/lib/ruby/site_ruby/.*)\z}) or abort("can't parse sitelibdir")
RB_SITELIBDIR = File.join(RB_INSTALL_DIR, $1)
(RbConfig::CONFIG['sitearchdir'] =~ %r{\A.*(/lib/ruby/site_ruby/.*)\z}) or abort("can't parse sitelibdir")
RB_SITEARCHDIR = File.join(RB_INSTALL_DIR, $1)

MAKE_EXE = (RUBY_PLATFORM =~ /mswin/) ? "nmake" : "make"

DLEXT = RbConfig::CONFIG['DLEXT']

##############################################################################

def log_info(msg)
  puts msg
end

def sys(cmd, *args)
  ok = system([cmd, cmd], *args)
  if !ok
    status = $?
    raise("#{cmd} #{Shellwords.join(args)} => #{status.exitstatus}")
  end
end

def cp(src_fpath, destdir)
  dest_fpath = File.join(destdir, File.basename(src_fpath))
  log_info "[CP] #{src_fpath} -> #{dest_fpath}"
  FileUtils.cp(src_fpath, dest_fpath, preserve:true)
end

def cp_r(src_paths, destdir)
  log_info "[CP_R] #{Shellwords.join(src_paths)} -> #{destdir}"
  FileUtils.cp_r(src_paths, destdir, preserve:true)
end

##############################################################################

Dir.chdir("./ext/amalgalite")

sys(RB_EXE, "extconf.rb", "--enable-icu")
sys(MAKE_EXE, "clean")
sys(MAKE_EXE)

destdir = File.join(RB_SITEARCHDIR, "amalgalite/1.9")
FileUtils.mkdir_p(destdir)
cp("amalgalite3.#{DLEXT}", destdir)

Dir.chdir("../..")

destdir = RB_SITELIBDIR
cp_r(["lib/amalgalite.rb", "lib/amalgalite"], destdir)


