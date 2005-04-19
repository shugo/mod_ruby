=begin

= apache/ruby-profile.rb

Copyright (C) 2005  Shugo Maeda <shugo@modruby.net>
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

Apache::RubyProfile profiles Ruby scripts.

== Example of httpd.conf

  RubyRequire apache/ruby-profile
  <Location /ruby-profile>
  SetHandler ruby-object
  RubyHandler Apache::RubyProfile.instance
  </Location>

=end

require "prof"
require "apache/ruby-run"

begin
  open("/proc/cpuinfo") do |f|
    f.each_line do |line|
      s = line.slice(/cpu MHz\s*:\s*(.*)/, 1)
      if s
        Prof.cpu_frequency = s.to_f * 1000000
        break
      end
    end
  end
rescue Errno::ENOENT
end

module Apache
  class RubyProfile < RubyRun
    def handler(r)
      status = check_request(r)
      return status if status != OK
      filename = setup(r)
      case ENV["RUBY_PROF_CLOCK_MODE"]
      when "gettimeofday"
        Prof.clock_mode = Prof::GETTIMEOFDAY
      when "cpu"
        Prof.clock_mode = Prof::CPU
      else
        Prof.clock_mode = Prof::CLOCK
      end
      Prof.start
      load(filename, true)
      result = Prof.stop
      append_profile_result(r, result)
      return OK
    end

    def append_profile_result(r, result)
      total = result.detect { |i|
        i.method_class.nil? && i.method_id == :"#toplevel"
      }.total_time
      if total == 0.0
        total = 0.001
      end
      sum = 0
      buf = "<table>\n"
      buf.concat("<tr>")
      buf.concat("<th>% time</th>")
      buf.concat("<th>cumulative seconds</th>")
      buf.concat("<th>self seconds</th>")
      buf.concat("<th>calls</th>")
      buf.concat("<th>self ms/call</th>")
      buf.concat("<th>total ms/call</th>")
      buf.concat("<th>name</th>")
      buf.concat("</tr>\n")
      for i in result
        sum += i.self_time
        if i.method_class.nil?
          name = i.method_id.to_s
        elsif i.method_class.is_a?(Class)
          name = i.method_class.to_s + "#" + i.method_id.to_s
        else
          name = i.method_class.to_s + "." + i.method_id.to_s
        end
        buf.concat("<tr>")
        buf.concat(format("<td>%.4f</td>", i.self_time / total * 100))
        buf.concat(format("<td>%.4f</td>", sum))
        buf.concat(format("<td>%.4f</td>", i.self_time))
        buf.concat(format("<td>%d</td>", i.count))
        buf.concat(format("<td>%.4f</td>", i.self_time * 1000 / i.count))
        buf.concat(format("<td>%.4f</td>", i.total_time * 1000 / i.count))
        buf.concat(format("<td>%s</td>", r.escape_html(name)))
        buf.concat("</tr>\n")
      end
      buf.concat("</table>\n")
      if !r.output_buffer.gsub!(/<\/body>/i, buf)
        r.output_buffer.concat(buf)
      end
      r.headers_out["Content-Length"] = r.output_buffer.length.to_s
    end
  end
end
