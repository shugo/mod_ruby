=begin

= mod_ruby Install Guide

[((<Index|URL:index.en.html>))
|((<RD format|URL:install.en.rd>))]

* ((<Install>))
* ((<Configure Apache>))
* ((<Use eRuby>))

== Install

Install mod_ruby like this:

  $ tar zxvf mod_ruby-x.y.z.tar.gz
  $ cd mod_ruby-x.y.z/
  $ ./configure.rb --with-apxs=/usr/local/apache/bin/apxs
  $ make
  # make install

== Configure Apache

Add these lines to httpd.conf.

  LoadModule ruby_module /usr/local/apache/libexec/mod_ruby.so
  
  # ClearModuleList
  # AddModule mod_ruby.c

  <IfModule mod_ruby.c>
    RubyRequire apache/ruby-run
    
    # Excucute files under /ruby as Ruby scripts
    <Location /ruby>
    SetHandler ruby-object
    RubyHandler Apache::RubyRun.instance
    </Location>
    
    # Execute *.rbx files as Ruby scripts
    <Files *.rbx>
    SetHandler ruby-object
    RubyHandler Apache::RubyRun.instance
    </Files>
  </IfModule>

== Use eRuby

Install eruby, and add these lines to httpd.conf.

  <IfModule mod_ruby.c>
    RubyRequire apache/eruby-run
    
    # Handle files under /eruby as eRuby files
    <Location /eruby>
    SetHandler ruby-object
    RubyHandler Apache::ERubyRun.instance
    </Location>
    
    # Handle *.rhtml files as eRuby files
    <Files *.rhtml>
    SetHandler ruby-object
    RubyHandler Apache::ERubyRun.instance
    </Files>
  </IfModule>

=end
