=begin

= apache/registry.rb

Copyright (C) 2001  Shugo Maeda <shugo@modruby.net>
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

Apache::Registry executes unaltered Ruby scripts persistently.

== Example of httpd.conf

  RubyRequire apache/registry
  <Location /ruby>
  SetHandler ruby-object
  RubyHandler Apache::Registry.instance
  </Location>

=end

require "apache/ruby-run"

module Apache
  class Registry < RubyRun
    def handler(r)
      status = check_request(r)
      return status if status != OK
      filename = setup(r)
      code = @db[filename]
      code.call
      return OK
    end

    private

    def initialize
      @db = RegistryDB.new
    end
  end

  class RegistryDB
    def [](filename)
      if @codes.key?(filename)
	return @codes[filename]
      else
	code = compile(filename)
	@codes[filename] = code
	return code
      end
    end

    private

    def initialize
      @codes = {}
    end

    def compile(filename)
      open(filename) do |f|
	s = f.read
	s.untaint
	binding = eval_string_wrap("binding")
	return eval(format("Proc.new {\n%s\n}", s), binding, filename, 0)
      end
    end
  end
end
