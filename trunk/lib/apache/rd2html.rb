=begin

= apache/rd2html.rb

Copyright (C) 2001, 2003  Shugo Maeda <shugo@modruby.net>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.

== Overview

Apache::RD2HTML converts RD to HTML.

== Requirements

* RDtool 0.6.7 or later.

== Example of httpd.conf

  RubyRequire apache/rd2html
  Alias /ruby-lib-doc/ /usr/lib/ruby/1.6/
  <Location /ruby-lib-doc>
  Options Indexes
  SetHandler ruby-object
  RubyHandler Apache::RD2HTML.instance
  </Location>

You can see the HTML version of ruby library documents at
<URL:http://your.host.name/ruby-lib-doc/>.
If there are no RD documents in a script, mod_rd2html returns
"415 Unsupported Media Type".

=end

require "singleton"
require "rd/rdfmt"
require "rd/rd2html-lib"

module Apache
  class RD2HTML
    include Singleton

		NoRdPage = <<-"EOF".gsub(/^\t{3}/, '')
			<html>
			  <head><title>No RD</title></head>
			  <body>
				<h1>No RD</h1>
				<p>The resource you&rsquo;ve specified does not contain any RD
				documentation -- sorry!</p>
				<p>Here&rsquo;s the source, in case that helps:</p>
				<pre>%s</pre>
			  </body>
			</html>
		EOF
		ErrorPage = <<-"EOF".gsub(/^\t{3}/, '')
			<html>
			  <head><title>Runtime Error</title></head>
			  <body>
				<h1>Runtime Error</h1>
				<p>The system encountered a runtime error: %s</p>
				<pre>%s</pre>
			  </body>
			</html>
		EOF

    def handler(r)
      if r.method_number == M_OPTIONS
	r.allowed |= (1 << M_GET)
	r.allowed |= (1 << M_POST)
	return DECLINED
      end

      filename = r.filename.dup
      filename.untaint

      begin
	open(filename) do |f|
	  tree = RD::RDTree.new(f)
	  visitor = RD::RD2HTMLVisitor.new
	  r.content_type = "text/html"
	  r.send_http_header
					content = visitor.visit(tree)

					if content
						r.print(content)
					else
						r.print NoRdPage % File::readlines( filename ).join("")
					end

	  return Apache::OK
	end
      rescue Errno::ENOENT
	return Apache::NOT_FOUND
      rescue Errno::EACCES
	return Apache::FORBIDDEN
			rescue NameError, Racc::ParseError # no =begin ... =end ?
				r.content_type = "text/html"
				r.send_http_header
				r.print NoRdPage %
					r.escape_html( File::readlines(filename).join("") )
				return Apache::OK
			rescue Exception => err
				r.content_type = "text/html"
				r.send_http_header
				r.print ErrorPage %
					[ r.escape_html(err.message), r.escape_html(err.backtrace.join("\n")) ]
				return Apache::OK
      end
    end
  end
end
