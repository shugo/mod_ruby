=begin

= mod_ruby FAQ

[((<Index|URL:index.en.html>))
|((<RD format|URL:faq.en.rd>))]

* ((<What is mod_ruby?>))
* ((<Where is it available?>))
* ((<Are there binary packages?>))
* ((<Is there a mailing list?>))
* ((<Is it secure?>))
* ((<Is it effective for heavy scripts?>))
* ((<Does It work on Windows?>))
* ((<Can I use eRuby on mod_ruby?>))
* ((<LoadModule does not work.>))
* ((<Why is "Content-Type" displayed on my browser?>))
* ((<Why are changes to my library not reflected in the server?>))
* ((<SecurityError is raised.>))
* ((<Location header does not work.>))
* ((<CGI::Session does not work.>))
* ((<I don't know why error has occurred!>))
* ((<Apache dies.>))
* ((<(({apachectl restart})) causes memory leaks.>))
* ((<Can't find libruby.so or liberuby.so.>))
* ((<Can't override existing classes>))

== What is mod_ruby?

mod_ruby embeds the Ruby interpreter into the Apache web server,
allowing Ruby CGI scripts to be executed natively. These scripts will
start up much faster than without mod_ruby.

[((<Back to Index|mod_ruby FAQ>))]

== Where is it available?

It is available at the mod_ruby official site ((<URL:http://www.modruby.net/>)).

[((<Back to Index|mod_ruby FAQ>))]

== Are there binary packages?

mod_ruby is included in Debian GNU/Linux and FreeBSD.

RPM is provided by
((<Vine Linux|URL:http://www.vinelinux.org/index-en.html>)).

[((<Back to Index|mod_ruby FAQ>))]

== Is there a mailing list?

((<URL:mailto:modruby@modruby.net>)) is set up for a purpose
to talk about mod_ruby (and eruby).

To subscribe this list, please send the following phrase

  subscribe Your-First-Name Your-Last-Name 

in the mail body (not subject) to the address
((<URL:mailto:modruby-ctl@modruby.net>)).

[((<Back to Index|mod_ruby FAQ>))]

== Is it secure?

Yes, and No.

The default value of $SAFE is 1 on mod_ruby, so it's secure for careless
programmers. For example eval(cgi["foo"][0]) causes SecurityError.

But on the other side, different scripts run using the same Ruby interpreter,
so a malicious script can change the behavior of the other scripts.

== Is it effective for heavy scripts?

Yes.

Someone may think fork() has little cost, relatively, for heavy scripts,
so mod_ruby is not an effective way to start such scripts.
But in fact it is effective, because decreasing the number of processes
helps the server especially when many clients make requests simultaneously.

[((<Back to Index|mod_ruby FAQ>))]

== Does It work on Windows?

Not yet.
Because I have no Windows machine (fortunately).

[((<Back to Index|mod_ruby FAQ>))]

== Can I use eRuby on mod_ruby?

Yes.

Please see ((<Install Guide|URL:install.en.html>))
to install mod_ruby with eRuby support.

[((<Back to Index|mod_ruby FAQ>))]

== LoadModule does not work.

If ClearModuleList is in httpd.conf, Please write
the following line after it.

  AddModule mod_ruby.c

[((<Back to Index|mod_ruby FAQ>))]

== Why is "Content-Type" displayed on my browser?

This script does not work correctly on mod_ruby.

  print "Content-Type: text/plain\r\n\r\n"
  print "hello world"

Because mod_ruby is compatible with NPH-CGI, so does not output
the HTTP status-line.
You have to output it yourself.

  print "HTTP/1.1 200 OK\r\n"
  print "Content-Type: text/plain\r\n\r\n"
  print "hello world"

Or you can use cgi.rb.

  require "cgi"
  
  cgi = CGI.new
  print cgi.header("type"=>"text/plain")
  print "hello world"

This script is better than the previous one.

[((<Back to Index|mod_ruby FAQ>))]

== Why are changes to my library not reflected in the server?

mod_ruby scripts share the Ruby interpreter.
So a library is loaded once, (({require})) does not load the library later.

Please restart Apache like this:

  # apachectl restart

Or use (({load})) instead of (({require})) for debugging.

[((<Back to Index|mod_ruby FAQ>))]

== SecurityError is raised.

On mod_ruby the default value of (({$SAFE})) is (({1})),
so dangerous operations with tainted string cause a SecurityError.

If it is certain that the operation is secure, use (({untaint})).

  query = CGI.new
  filename = query.params["filename"][0].dup
  filename.untaint
  file = open(filename)

Be careful not to make security holes!

[((<Back to Index|mod_ruby FAQ>))]

== Location header does not work.

Please specify the status line like this:

  r = Apache.request
  r.status_line = "302 Found"
  r.headers_out["Location"] = "http://www.modruby.net/"
  r.content_type = "text/html; charset=iso-8859-1"
  r.send_http_header
  print "<html><body><h1>302 Found</h1></body></html>"

Or specify the exit status like this.

  r = Apache.request
  r.headers_out["Location"] = "http://www.modruby.net/"
  exit Apache::REDIRECT

[((<Back to Index|mod_ruby FAQ>))]

== CGI::Session does not work.

On mod_ruby, CGI::Session is not closed automatically, so you should
close it explicitly.

  session = CGI::Session.new(...)
  begin
    ...
  ensure
    session.close
  end

This problem is not CGI::Session specific. You should close files etc too.

[((<Back to Index|mod_ruby FAQ>))]

== I don't know why error has occurred!

The Ruby interpreter may be in a panic because of bugs in other scripts.
Please restart Apache.

[((<Back to Index|mod_ruby FAQ>))]

== Apache dies.

Ruby built by egcs-1.1.2 may cause Segmentation Fault.
Please try the latest Ruby (1.6.2 or later).

Otherwise please send a bug report to the author
((<Shugo Maeda|URL:mailto:shugo@modruby.net>)).

[((<Back to Index|mod_ruby FAQ>))]

== (({apachectl restart})) causes memory leaks.

Currently, Ruby API does not provide a way to free memory that has
been allocated by the interpreter. So mod_ruby can't free memory on
(({apachectl graceful})).

Please stop and start Apache for the time being.

  # apachectl restart

[((<Back to Index|mod_ruby FAQ>))]

== Can't find libruby.so or liberuby.so.

libruby.so may not be in the runtime library search path.

On many Linux distributions, /usr/local/lib is not included in the runtime
library search path.
If you are installing mod_ruby under /usr/local, Please add the fllowing line
to /etc/ld.so.conf, and run ldconfig.

  /usr/local/lib

[((<Back to Index|mod_ruby FAQ>))]

== Can't override existing classes

You can't override classes in your mod_ruby scripts directly.
(Instead, a new class will be defined.)
Because mod_ruby scripts are loaded by Kernel#load(filename,
true).

If you have to override existing classes, please do it in a
library, then require it from your mod_ruby scripts.

[((<Back to Index|mod_ruby FAQ>))]

=end
