=begin

= auto-reload.rb

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

autoreload reloads modified libraries automatically.
This library is useful to develop mod_ruby scripts.

=end

module AutoReload
  LIBRARY_MTIMES = {}
  LIBRARY_DEPS = {}
  PROCESSING_FILE = []

  def AutoReload.find_library(lib)
    if lib !~ /\.rb$/
      lib += ".rb"
    end
    for dir in $:
      file = File.expand_path(lib, dir)
      file.untaint
      return file if File.file?(file)
    end
    return nil
  end

  def require(lib)
    unless file = AutoReload.find_library(lib)
      return super(lib)
    end
    deps = LIBRARY_DEPS[file] ||= {}
    PROCESSING_FILE.each do |f|
      return false if f == file
      LIBRARY_DEPS[f][file] = 1
    end
    PROCESSING_FILE.push file
    begin
      do_load = false
      deps.each do |f,|
        begin
          if LIBRARY_MTIMES[f] != File.mtime(f)
            do_load = true
            break
          end
        rescue Errno::ENOENT
          deps.delete f
        end
      end
      mtime = File.mtime(file)
      if do_load or LIBRARY_MTIMES[file] != mtime
        result = load(file)
        LIBRARY_MTIMES[file] = mtime
        result
      else
        false
      end
    ensure
      PROCESSING_FILE.pop
    end
  end
end

include AutoReload		# override Kernel#require
