=begin

= apache/query.rb

Copyright (C) 2003  Shugo Maeda <shugo@modruby.net>
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

=end

module Apache
  class Query
    attr_reader :params, :cookies

    def initialize(req)
      @req = req
      @files = []
      @closed = false

      if @req.method_number == Apache::M_POST &&
	  %r|\Amultipart/form-data.*boundary=\"?([^\";,]+)\"?|n =~
	  @req.headers_in["Content-Type"]
	@multipart = true
	boundary = $1
	@params = get_multipart(boundary)
      else
	@multipart = false
	str = get_query_string
	@params = parse_query_string(str)
      end

      @cookies = get_cookies
    end

    def multipart?
      return @multipart
    end

    def param(name)
      return @params[name][0]
    end
    alias [] param

    def close
      return if @closed
      for file in @files
	file.close!
      end
      @closed = true
    end

    private

    def get_query_string
      if @req.method_number == Apache::M_POST
	return @req.read(@req.headers_in["Content-Length"].to_i) || ""
      else
	return @req.args || ""
      end
    end

    def parse_query_string(str)
      params = Hash.new([])
      str.split(/[&;]/n).each do |pairs|
	key, value = pairs.split("=",2).collect{ |v| urldecode(v) }
	if params.key?(key)
	  params[key].push(value)
	else
	  params[key] = [value]
	end
      end
      return params
    end
        
    def get_multipart(boundary)
      params = Hash.new([])
      reader = MultipartReader.new(@req, boundary)
      while !reader.eof?
	header = reader.read_header
                
	if / name="?([^\";]*)"?/n =~ header["Content-Disposition"]
	  name = $1
	else
	  name = ""
	end
                
	if / filename="?([^\";]*)"?/n =~ header["Content-Disposition"]
	  filename = $1
	else
	  value = reader.read_body
	  if params.key?(name)
	    params[name].push(value)
	  else
	    params[name] = [value]
	  end
	  next
	end
                
	if /Mac/ni =~ @req.headers_in["User-Agent"] &&
	    /Mozilla/ni =~ @req.headers_in["User-Agent"] &&
	    /MSIE/ni !~ @req.headers_in["User-Agent"]
	  filename = urldecode(filename)
	end
                
	file = MultipartFile.new("query.rb")
	@files.push(file)
	file.binmode
	while data = reader.read
	  file.print(data)
	end
	file.rewind

	file.original_filename = filename
	file.content_type = header["Content-Type"]

	if params.key?(name)
	  params[name].push(file)
	else
	  params[name] = [file]
	end
      end
      return params
    end
        
    def get_cookies
      cookies = {}
      str = @req.headers_in["Cookie"]
      return cookies unless str

      str.split(/; /n).each do |pair|
	name, value = pair.split("=", 2)
	name = urldecode(name)
	value = urldecode(value || "")
	cookies[name] = value
      end
      return cookies
    end

    def urldecode(str)
      return str.tr("+", " ").gsub(/((?:%[0-9a-fA-F]{2})+)/n) {
	[$1.delete("%")].pack("H*")
      }
    end
  end

  class MultipartReader
    INITIAL_SIZE = 4 * 1024
    EOL = "\r\n"

    HEADER_VALID_TOKEN = '[-\w!\#$%&\'*+.^_`|{}~]'
    HEADER_REGEXP = /(#{HEADER_VALID_TOKEN}+):\s+([^#{EOL}]*)/n
    
    def initialize(req, boundary)
      @req = req
      @boundary = "--" + boundary
      @length = @req.headers_in["Content-Length"].to_i
      
      @buffer = ""
      @chunk_size = @boundary.size > INITIAL_SIZE ?
      @boundary.size : INITIAL_SIZE
    end
    
    def read_header
      while true
	fill_buffer(@chunk_size)
	endi = @buffer.index(EOL + EOL)
	break if endi || @buffer.empty?
	return {} unless @length > 0
      end
      header = @buffer[0 ... endi + 2]
      @buffer[0 ... endi + 4] = ""
      
      result = {}
      header.gsub!(/EOL\s+/n, "")
      header.scan(HEADER_REGEXP) do |name, value|
	name.gsub!(/\b\w/n) { |m| m.upcase }
	result[name] = value
      end
      
      return result
    end
    
    def read_body
      result = ""
      while data = read
	result.concat(data)
      end
      result
    end

    def read(bytes = INITIAL_SIZE)
      fill_buffer(bytes)
      start = @buffer.index(@boundary)
      case start
      when 0
	if @buffer.index(@boundary + "--") == 0
	  @buffer = ""
	  @length = 0
	  return nil
	end
	@buffer[0 ... @boundary.size + 2] = ""
	return nil
      when nil
	bytes_to_return = bytes - @boundary.size - 1
      else
	bytes_to_return = start > bytes ? bytes : start
      end
      
      chunk = @buffer[0 ... bytes_to_return]
      @buffer[0 ... bytes_to_return] = ""
      return start ? chunk[0 ... -2] : chunk
    end
    
    def fill_buffer(bytes)
      return if @length == 0
      
      bytes_to_read = bytes - @buffer.size + @boundary.size + 2
      bytes_to_read = @length if bytes_to_read > @length
      chunk = @req.read(bytes_to_read) || ""
      @buffer.concat(chunk)
      @length -= chunk.size
    end
    
    def eof?
      @buffer.empty? && @length <= 0
    end
  end

  class MultipartFile < Tempfile
    attr_accessor :original_filename, :content_type
  end
end
