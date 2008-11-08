#!/usr/bin/env ruby

AC_ACVERSION = "0.2.4"

$ac_helps = []

open("configure.rb.in") do |cin|
  $input = cin.read
  $input.gsub!(/AC_ARG_ENABLE\(\s*(.*?)\s*,\s*(.*?)\s*\)/m) do
    $ac_helps.push($2)
    "AC_ENABLE(#{$1})"
  end
  $input.gsub!(/AC_ARG_WITH\(\s*(.*?)\s*,\s*(.*?)\s*\)/m) do
    $ac_helps.push($2)
    "AC_WITH(#{$1})"
  end
end

open("configure.rb", "w") do |cout|
  cout.print(<<EOS)
#!/usr/bin/env ruby

# Generated automatically using autoconf.rb version #{AC_ACVERSION}

require "mkmf"

$ac_help = ""
EOS

  for help in $ac_helps
    cout.printf("$ac_help += %s \"\\n\"\n", help)
  end

  cout.print(<<'EOS')

$ac_sed = {}
$ac_confdefs = {}
$ac_features = {}
$ac_packages = {}
  
def AC_SUBST(variable)
  $ac_sed[variable] = true
end

def AC_DEFINE(variable, value = 1)
  case value
  when true
    value = "1"
  when false
    value = "0"
  when String
    value = value.dump
  else
    value = value.inspect
  end
  $ac_confdefs[variable] = value
end

AC_GIVEN = Object.new
def AC_GIVEN.if_not_given
  # do nothing
end

AC_NOT_GIVEN = Object.new
def AC_NOT_GIVEN.if_not_given
  yield
end

def AC_ENABLE(feature, action = Proc.new)
  if $ac_features[feature]
    yield($ac_features[feature])
    return AC_GIVEN
  else
    return AC_NOT_GIVEN
  end
end

def AC_WITH(package, action = Proc.new)
  if $ac_packages[package]
    yield($ac_packages[package])
    return AC_GIVEN
  else
    return AC_NOT_GIVEN
  end
end

require 'fileutils'

def AC_OUTPUT(*files)
  $DEFS ||= ""
  if $AC_LIST_HEADER
    $DEFS << " -DHAVE_CONFIG_H"
    AC_OUTPUT_HEADER($AC_LIST_HEADER)
  else
    $DEFS << " " + $ac_confdefs.collect {|k, v| "-D#{k}=#{v}" }.join(" ")
  end
  for file in files
    print "creating ", file, "\n"
    open(File.join($srcdir, file + ".in")) do |fin|
      FileUtils.mkdir_p(File.dirname(file))
      open(file, "w") do |fout|
	depend = false
	while line = fin.gets
          depend = true if /^\#\#\# depend/ =~ line
	  line.gsub!(/@([A-Za-z_]+)@/) do |s|
	    name = $1
	    if $ac_sed.key?(name)
	      eval("$" + name) #"
	    else
	      s
	    end
	  end
          line.gsub!(/(\s)([^\s\/]+\.[ch])/, '\1{$(srcdir)}\2') if depend && $nmake
	  fout.print(line)
	end
      end
    end
  end
end

def AC_OUTPUT_HEADER(header)
  print "creating ", header, "\n"
  open(File.join($srcdir, header + ".in")) do |fin|
    open(header, "w") do |fout|
      while line = fin.gets
	line.sub!(/^(#define|#undef)\s+([A-Za-z_]+).*$/) do |s|
	  name = $2
	  if $ac_confdefs.key?(name)
	    val = $ac_confdefs[name]
	    "#define #{name} #{val}"
	  else
	    s
	  end
	end
	fout.print(line)
      end
    end
  end
  $ac_confdefs.clear
end

def AC_CONFIG_HEADER(header)
  $AC_LIST_HEADER = header
end

def AC_CHECK(feature)
  AC_MSG_CHECKING(feature)
  AC_MSG_RESULT(yield)
end

def AC_MSG_CHECKING(feature)
  print "checking #{feature}... "
  $stdout.flush
end

def AC_MSG_RESULT(result)
  case result
  when true
    result = "yes"
  when false, nil
    result = "no"
  end
  puts(result)
end

def AC_MSG_WARN(msg)
  $stderr.print("configure.rb: warning: ", msg, "\n")
end

def AC_MSG_ERROR(msg)
  $stderr.print("configure.rb: error: ", msg, "\n")
  exit(1)
end

def AC_CONFIG_AUX_DIR_DEFAULT
  AC_CONFIG_AUX_DIRS($srcdir, "#{$srcdir}/..",  "#{$srcdir}/../..")
end

def AC_CONFIG_AUX_DIRS(*dirs)
  for dir in dirs
    for prog in [ "install-rb", "install.rb" ]
      file = File.join(dir, prog)
      if File.file?(file); then
	$ac_aux_dir = dir
	$ac_install_rb = "$(RUBY) #{file} -c"
	return
      end
    end
  end
end

begin
  require "continuation"
rescue LoadError
end

def AC_PROG_INSTALL
  AC_MSG_CHECKING("for a BSD compatible install")
  $ac_cv_path_install = callcc { |c|
    for dir in ENV["PATH"].split(/:/)
      if %r'^(/|\./|/etc/.*|/usr/sbin/.*|/usr/etc/.*|/sbin/.*|/usr/afsws/bin/.*|/usr/ucb/.*)$' =~ dir + "/" # '
	next
      end
      for prog in [ "ginstall", "scoinst", "install" ]
	file = File.join(dir, prog)
	if File.file?(file)
	  if prog == "install" &&
	      `#{file} 2>&1` =~ /dspmsg/
	    # AIX install.  It has an incompatible calling convention.
	  else
	    c.call("#{file} -c")
	  end
	end
      end
    end
    unless $ac_install_rb
      AC_CONFIG_AUX_DIR_DEFAULT()
    end
    $ac_install_rb
  }
  $INSTALL = $ac_cv_path_install
  AC_MSG_RESULT($INSTALL)
  $INSTALL_PROGRAM ||= "$(INSTALL)"
  $INSTALL_SCRIPT ||= "$(INSTALL)"
  $INSTALL_DATA ||= "$(INSTALL) -m 644"
  $INSTALL_DLLIB ||= "$(INSTALL) -m 555"
  $INSTALL_DIR ||= "$(INSTALL) -d"
  AC_SUBST("INSTALL")
  AC_SUBST("INSTALL_PROGRAM")
  AC_SUBST("INSTALL_SCRIPT")
  AC_SUBST("INSTALL_DATA")
  AC_SUBST("INSTALL_DLLIB")
  AC_SUBST("INSTALL_DIR")
end

$stdout.sync = true

drive = File::PATH_SEPARATOR == ';' ? /\A\w:/ : /\A/
prefix = Regexp.new("\\A" + Regexp.quote(CONFIG["prefix"]))
$drive = CONFIG["prefix"] =~ drive ? $& : ''
$prefix = CONFIG["prefix"].sub(drive, '')
$exec_prefix = "$(prefix)"
$bindir = CONFIG["bindir"].sub(prefix, "$(exec_prefix)").sub(drive, '')
$datadir = CONFIG["datadir"].sub(prefix, "$(prefix)").sub(drive, '')
$libdir = CONFIG["libdir"].sub(prefix, "$(exec_prefix)").sub(drive, '')
$archdir = $archdir.sub(prefix, "$(prefix)").sub(drive, '')
$sitelibdir = $sitelibdir.sub(prefix, "$(prefix)").sub(drive, '')
$sitearchdir = $sitearchdir.sub(prefix, "$(prefix)").sub(drive, '')
$includedir = CONFIG["includedir"].sub(prefix, "$(prefix)").sub(drive, '')
$mandir = CONFIG["mandir"].sub(prefix, "$(prefix)").sub(drive, '')

$rubylibdir ||=
  $libdir + "/ruby/" + CONFIG["MAJOR"] + "." + CONFIG["MINOR"]

for option in ARGV
  if option =~ /^-.*?=(.*)/
    optarg = $1
  else
    optarg = nil
  end
  case option
  when /^--prefix/
    $prefix = optarg
  when /^--exec-prefix/
    $exec_prefix = optarg
  when /^--bindir/
    $bindir = optarg
  when /^--datadir/
    $datadir = optarg
  when /^--libdir/
    $libdir = optarg
  when /^--includedir/
    $includedir = optarg
  when /^--mandir/
    $mandir = optarg
  when /^--enable-([^=]+)/
    feature = $1
    if optarg.nil?
      optarg = "yes"
    end
    $ac_features[feature] = optarg
  when /^--with-([^=]+)/
    package = $1
    if optarg.nil?
      optarg = "yes"
    end
    $ac_packages[package] = optarg
  when "--help"
    print <<EOF
Usage: configure.rb [options]
Options: [defaults in brackets after descriptions]
Configuration:
  --help                  print this message
Directory and file names:
  --prefix=PREFIX         install architecture-independent files in PREFIX
                          [#{$prefix}]
  --exec-prefix=EPREFIX   install architecture-dependent files in EPREFIX
                          [same as prefix]
  --bindir=DIR            user executables in DIR [EPREFIX/bin]
  --libdir=DIR            object code libraries in DIR [EPREFIX/lib]
  --includedir=DIR        C header files in DIR [PREFIX/include]
  --datadir=DIR           read-only architecture-independent data in DIR
                          [PREFIX/share]
  --mandir=DIR            manual pages in DIR [PREFIX/man]
EOF
    if $ac_help.length > 0
      print "--enable and --with options recognized:\n"
      print $ac_help
    end
    exit(0)
  when /^-.*/
    AC_MSG_ERROR("#{option}: invalid option; use --help to show usage")
  end
end

$srcdir = File.expand_path(File.dirname($0))
$VPATH = ""

$arch = CONFIG["arch"]
$sitearch = CONFIG["sitearch"]
$ruby_version = Config::CONFIG["ruby_version"] ||
  CONFIG["MAJOR"] + "." + CONFIG["MINOR"]

$CC = CONFIG["CC"]
$AR = CONFIG["AR"]
$LD = "$(CC)"
$RANLIB = CONFIG["RANLIB"]
$ruby = arg_config("--ruby", File.join(Config::CONFIG["bindir"], CONFIG["ruby_install_name"]))
$RUBY = ($nmake && !$configure_args.has_key?('--ruby')) ? $ruby.gsub(%r'/', '\\') : $ruby
if RUBY_VERSION < "1.8.0"
  $RM = 'rm -f'
else
  $RM = CONFIG["RM"] || '$(RUBY) -run -e rm -- -f'
end

if not defined? CFLAGS
  CFLAGS = CONFIG["CFLAGS"]
end

if CFLAGS.index(CONFIG["CCDLFLAGS"])
  $CFLAGS = CFLAGS
else
  $CFLAGS = CFLAGS + " " + CONFIG["CCDLFLAGS"]
end
$cflags = CONFIG["cflags"]
$optflags = CONFIG["optflags"]
$debugflags = CONFIG["debugflags"]
$LDFLAGS = CONFIG["LDFLAGS"]
if $LDFLAGS.to_s.empty? && /mswin32/ =~ RUBY_PLATFORM
  $LDFLAGS = "-link -incremental:no -pdb:none"
end
$LIBS = CONFIG["LIBS"]
$XLDFLAGS = CONFIG["XLDFLAGS"].to_s
$XLDFLAGS.gsub!(/-L\./, "")
if /mswin32/ !~ RUBY_PLATFORM
  $XLDFLAGS += " -L$(libdir)"
elsif RUBY_VERSION >= "1.8"
  $XLDFLAGS += " #{CONFIG['LIBPATHFLAG'] % '$(libdir)'}"
end
$DLDFLAGS = CONFIG["DLDFLAGS"]
$LDSHARED = CONFIG["LDSHARED"]

$EXEEXT = CONFIG["EXEEXT"]
$DLEXT = CONFIG["DLEXT"]

$ruby_install_name = CONFIG["ruby_install_name"]
$RUBY_INSTALL_NAME = CONFIG["RUBY_INSTALL_NAME"]
$RUBY_SHARED = (CONFIG["ENABLE_SHARED"] == "yes")
$LIBRUBYARG = CONFIG["LIBRUBYARG"]
if $RUBY_SHARED
  $LIBRUBYARG.gsub!(/-L\./, "")
else
  if RUBY_VERSION < "1.8" && RUBY_PLATFORM !~ /mswin32/
    $LIBRUBYARG = "$(hdrdir)/" + $LIBRUBYARG
  end
end
$LIBRUBY = CONFIG["LIBRUBY"]
$LIBRUBY_A = CONFIG["LIBRUBY_A"]
$RUBY_SO_NAME = CONFIG["RUBY_SO_NAME"]

case RUBY_PLATFORM
when /-aix/
  if $RUBY_SHARED
    $LIBRUBYARG = "-Wl,$(libdir)/" + CONFIG["LIBRUBY_SO"]
    $LIBRUBYARG.sub!(/\.so\.[.0-9]*$/, '.so')
    $XLDFLAGS = ""
  else
    $XLDFLAGS = "-Wl,-bE:$(topdir)/ruby.imp"
  end
  if $DLDFLAGS !~ /-Wl,/
    $LIBRUBYARG.gsub!(/-Wl,/, '')
    $XLDFLAGS.gsub!(/-Wl,/, '')
    $DLDFLAGS.gsub!(/-Wl,/, '')
  end
end

$COMPILE_RULES = ''
if defined?(COMPILE_RULES)
  COMPILE_RULES.each do |rule|
    $COMPILE_RULES << sprintf(rule, 'c', $OBJEXT)
    $COMPILE_RULES << sprintf("\n\t%s\n\n", COMPILE_C)
  end
end

AC_SUBST("srcdir")
AC_SUBST("topdir")
AC_SUBST("hdrdir")
AC_SUBST("VPATH")

AC_SUBST("arch")
AC_SUBST("sitearch")
AC_SUBST("ruby_version")
AC_SUBST("drive")
AC_SUBST("prefix")
AC_SUBST("exec_prefix")
AC_SUBST("bindir")
AC_SUBST("datadir")
AC_SUBST("libdir")
AC_SUBST("rubylibdir")
AC_SUBST("archdir")
AC_SUBST("sitedir")
AC_SUBST("sitelibdir")
AC_SUBST("sitearchdir")
AC_SUBST("includedir")
AC_SUBST("mandir")

AC_SUBST("CC")
AC_SUBST("AR")
AC_SUBST("LD")
AC_SUBST("RANLIB")
AC_SUBST("RUBY")
AC_SUBST("RM")

AC_SUBST("CFLAGS")
AC_SUBST("cflags")
AC_SUBST("optflags")
AC_SUBST("debugflags")
AC_SUBST("DEFS")
AC_SUBST("LDFLAGS")
AC_SUBST("LIBS")
AC_SUBST("XLDFLAGS")
AC_SUBST("DLDFLAGS")
AC_SUBST("LDSHARED")

AC_SUBST("OBJEXT")
AC_SUBST("EXEEXT")
AC_SUBST("DLEXT")

AC_SUBST("COMPILE_RULES")

AC_SUBST("ruby_install_name")
AC_SUBST("RUBY_INSTALL_NAME")
AC_SUBST("LIBRUBYARG")
AC_SUBST("LIBRUBYARG_SHARED")
AC_SUBST("LIBRUBYARG_STATIC")
AC_SUBST("LIBRUBY")
AC_SUBST("LIBRUBY_A")
AC_SUBST("RUBY_SO_NAME")

EOS

  cout.print($input)
  cout.chmod(0755)
end
