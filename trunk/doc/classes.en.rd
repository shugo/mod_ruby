=begin

= mod_ruby Class Reference Manual

[((<Index|URL:index.en.html>))
|((<RD format|URL:classes.en.rd>))]

* ((<Apache>))
* ((<Apache::Request>))
* ((<Apache::Cookie>))
* ((<Apache::MultiVal>))
* ((<Apache::Upload>))
* ((<Apache::Table>))
* ((<Apache::ParamTable>))
* ((<Apache::Server>))
* ((<Apache::Connection>))

== Apache

A module to provide Apache functions.

=== Module Functions

--- add_version_component
      Add a token to Apache's version string.

--- chdir_file(str)
      Change the server's current working directory to the directory part of the
      specified filename.

--- request
      Returns the current ((<Apache::Request>)) object.

--- server_root
      Returns the server's root directory (ie., the one set by the
      ((|ServerRoot|)) directive).

--- server_built
      Returns the server built date string.

--- server_version
      Returns the server version string.

--- unescape_url(str)
      Decodes a URL-encoded string.


=== Constants
--- Handler status return codes
      OK,
      DECLINED,
      DONE

--- HTTP response codes
      AUTH_REQUIRED, 
      BAD_GATEWAY, 
      BAD_REQUEST, 
      DOCUMENT_FOLLOWS, 
      FORBIDDEN, 
      HTTP_ACCEPTED, 
      HTTP_BAD_GATEWAY, 
      HTTP_BAD_REQUEST, 
      HTTP_CONFLICT, 
      HTTP_CONTINUE, 
      HTTP_CREATED, 
      HTTP_EXPECTATION_FAILED, 
      HTTP_FAILED_DEPENDENCY((-if implemented-)), 
      HTTP_FORBIDDEN, 
      HTTP_GATEWAY_TIME_OUT, 
      HTTP_GONE, 
      HTTP_INSUFFICIENT_STORAGE((-if implemented-)),
      HTTP_INTERNAL_SERVER_ERROR, 
      HTTP_LENGTH_REQUIRED, 
      HTTP_LOCKED, 
      HTTP_METHOD_NOT_ALLOWED, 
      HTTP_MOVED_PERMANENTLY, 
      HTTP_MOVED_TEMPORARILY, 
      HTTP_MULTIPLE_CHOICES, 
      HTTP_MULTI_STATUS, 
      HTTP_NON_AUTHORITATIVE, 
      HTTP_NOT_ACCEPTABLE, 
      HTTP_NOT_EXTENDED, 
      HTTP_NOT_FOUND, 
      HTTP_NOT_IMPLEMENTED, 
      HTTP_NOT_MODIFIED, 
      HTTP_NO_CONTENT, 
      HTTP_OK, 
      HTTP_PARTIAL_CONTENT, 
      HTTP_PAYMENT_REQUIRED, 
      HTTP_PRECONDITION_FAILED, 
      HTTP_PROCESSING, 
      HTTP_PROXY_AUTHENTICATION_REQUIRED, 
      HTTP_RANGE_NOT_SATISFIABLE, 
      HTTP_REQUEST_ENTITY_TOO_LARGE, 
      HTTP_REQUEST_TIME_OUT, 
      HTTP_REQUEST_URI_TOO_LARGE, 
      HTTP_RESET_CONTENT, 
      HTTP_SEE_OTHER, 
      HTTP_SERVICE_UNAVAILABLE, 
      HTTP_SWITCHING_PROTOCOLS, 
      HTTP_TEMPORARY_REDIRECT, 
      HTTP_UNAUTHORIZED, 
      HTTP_UNPROCESSABLE_ENTITY, 
      HTTP_UNSUPPORTED_MEDIA_TYPE, 
      HTTP_USE_PROXY, 
      HTTP_VARIANT_ALSO_VARIES, 
      HTTP_VERSION_NOT_SUPPORTED, 
      LENGTH_REQUIRED, 
      METHOD_NOT_ALLOWED, 
      MOVED, 
      MULTIPLE_CHOICES, 
      NOT_ACCEPTABLE, 
      NOT_FOUND, 
      NOT_IMPLEMENTED, 
      PARTIAL_CONTENT, 
      PRECONDITION_FAILED, 
      REDIRECT, 
      SERVER_ERROR, 
      USE_LOCAL_COPY, 
      VARIANT_ALSO_VARIES

--- Request method constants
      For testing against the return value of
      ((<Apache::Request#method_number|method_number>)).

      M_CONNECT, 
      M_COPY, 
      M_DELETE, 
      M_GET, 
      M_INVALID, 
      M_LOCK, 
      M_MKCOL, 
      M_MOVE, 
      M_OPTIONS, 
      M_PATCH, 
      M_POST, 
      M_PROPFIND, 
      M_PROPPATCH, 
      M_PUT, 
      M_TRACE, 
      M_UNLOCK, 
      METHODS

--- Options bitmask constants
      Constants for testing for enabled options via
      ((<Apache::Request#allow_options|allow_options>)).

      OPT_ALL,
      OPT_EXECCGI, 
      OPT_INCLUDES, 
      OPT_INCNOEXEC, 
      OPT_INDEXES, 
      OPT_MULTI, 
      OPT_NONE, 
      OPT_SYM_LINKS, 
      OPT_SYM_OWNER, 
      OPT_UNSET

--- Satisfy constants
      Constants for testing the return value of the
      ((<Apache::Request#satisfies|satisfies>)) method.

      SATISFY_ALL,
      SATISFY_ANY, 
      SATISFY_NOSPEC

--- Remotehost constants
      Constants which can be (optionally) passed to
      ((<Apache::Request#remote_host|remote_host>)) to affect what type of
      lookup is performed.

      REMOTE_DOUBLE_REV,
      REMOTE_HOST, 
      REMOTE_NAME, 
      REMOTE_NOLOOKUP

--- Blocking Policy constants
      Constants which are used for setting the blocking policy via
      ((<Apache::Request#setup_client_block|setup_client_block>)).

      REQUEST_NO_BODY,
      REQUEST_CHUNKED_ERROR,
      REQUEST_CHUNKED_DECHUNK,
      REQUEST_CHUNKED_PASS

[((<Back to Index|mod_ruby Class Reference Manual>))]

== Apache::Request

A class to wrap (({request_rec})) data type.

=== Superclass

Object

=== Included Modules

Enumerable

=== Methods

--- << obj
      String Output---Writes ((|obj|)) to the client output buffer. ((|obj|))
      will be converted to a string using (({to_s})).

--- []=
      Sets the value of the specified response header. ((*Deprecated*)): Use
      ((<#headers_out|headers_out>)) instead.

--- [str]
      Returns the value of the specified request header. ((*Deprecated*)): Use
      ((<#headers_in|headers_in>)) instead.

--- add_cgi_vars
      Add the variables required by the CGI/1.1 protocol to the
      ((<subprocess_env>)) table.

--- add_common_vars
      Add other Apache CGI variables to the ((<subprocess_env>)) table.

--- allow_options
      Returns the bitmap with specifies which options are enabled for the
      directory to which the request has been mapped. You can use the Apache
      module's ((<options bitmask constants|Options bitmask constants>)) to test
      for desired values.

      For example:
        include Apache

        # Make sure that ExecCGI and Indexes are turned on for the Location
        # being served:
        unless req.allow_options & (OPT_EXECCGI|OPT_INDEXES)
            req.log_reason( "ExecCGI and/or Indexes are off in this directory",
                            req.filename )
            return FORBIDDEN
        end

--- allow_overrides
      Returns an Integer (?).

--- allowed
--- allowed= int
      Returns/sets the bitvector (an (({Integer}))) of the request methods that
      the handler can accommodate. You can set bits in this field using one or
      more ((<request method constants|Request method constants>)).

      Example:

        include Apache
        Apache::request.allowed |= (1 << M_GET)
        Apache::request.allowed |= (1 << M_POST)

--- args
      Returns the quest string for CGI GET requests, and corresponds to the
      portion of the URI following the (({?})).

--- auth_name
--- auth_name= str
      Returns/sets the authentication realm for the receiving request.

--- auth_type
--- auth_type= str
      Returns/sets the authentication type for the receiving request. Usually
      one of (({"Basic"})) or (({"Digest"})).

--- binmode
      Puts the client input data stream into binary mode. This is useful only in
      MS-DOS/Windows environments. Once a stream is in binary mode, it cannot be
      reset to nonbinary mode.

--- bytes_sent
      Returns the number of bytes sent by the server to the client, excluding
      the HTTP headers. It is only useful after ((<send_http_header>)) has been
      called.

--- cache_resp
--- cache_resp= val
      Returns/sets the flag that controls whether the response will have
      cache-control headers put into its response. If (({cache_resp})) is set to
      to (({true})), the response will have the following headers added:

        Pragma: no-cache
        Cache-control: no-cache

      If set to (({false})), the (({Pragma})) and (({Cache-control})) headers
      will be removed completely from the response headers, regardless of their
      content.

--- cancel
      Clears the output buffer.

--- connection
      Returns the ((<Apache::Connection>)) object associated with the request.

--- construct_url(uri)
      Returns a fully-qualified URI (({String})) from the path specified by
      ((|uri|)) using the request object's server name and port.

--- content_encoding
      Returns the MIME encoding type of the response, as set by the
      MIME-checking phase of the transaction.

--- content_encoding= str
      Set the MIME (({Content-Encoding})) header of the response.

--- content_languages
      Returns the value of the (({Content-Languages})) of the response. This is
      typically set by the MIME-checking phase of the transaction.

--- content_languages= str
      Specifies Content-Languages of the response header.

--- content_length
      Returns the length of the incoming content as specified by the
      (({Content-Length})) header. ((*Deprecated*)): Use
      (({req.headers_in['Content-Length']})) instead.

--- content_type
      Returns the MIME content type of the response, as set by the MIME-checking
      phase of the transaction.

--- content_type= str
      Set the (({Content-Type})) header of the response.

--- custom_response(status,uri)
      Set the error document for the given ((|status|)) to the given
      ((|uri|)). The status is a (({Fixnum})) status code like those in the
      Apache module's ((<HTTP response codes>)).

      Example:
        include Apache

        unless req.notes['username']
          req.custom_response( HTTP_UNAUTHORIZED, "/noauth.html" )
          return HTTP_UNAUTHORIZED
        end

--- default_charset
      Returns the name of the default character set, as defined by the
      (({AddDefaultCharset})) directive.

--- default_type
      Returns the value of the (({DefaultType})) directive, or
      (({"text/plain"})) if not configured.

--- dispatch_handler
--- dispatch_handler= str
      Allows one to get/set the Ruby code which returns the dispatch handler for
      requests. This makes it possible to write your own dispatch handler.

--- each([rs]) {|line|...}
      Executes the block for every ((|line|)), where lines are separated by the
      separator string ((|rs|)) ((({$/})) by default).

--- each_byte {|ch|...}
      Calls the given block once for each byte (0..255) in the input from the
      client, passing the byte as an argument.

--- each_header {|hdr,val|...}
      Iterates over the headers in the request, calling the specified
      ((|block|)) with each header name and value. ((*Deprecated*)): Use
      ((<#headers_in|headers_in>)) instead.

--- each_key {|hdr|...}
      Iterates over the names of each header in the request, calling the
      specified ((|block|)) once with each one. ((*Deprecated*)): Use
      ((<#headers_in|headers_in>)) instead.

--- each_line([rs]) {|line|...}
      Synonym for ((<Apache::Request#each|each>)).

--- each_value {|val|...}
      Iterates over the values of each header in the request, calling the
      specified ((|block|)) once with each one. ((*Deprecated*)): Use
      ((<#headers_in|headers_in>)) instead.

--- eof
      Returns (({true})) if the client input data stream is at end of file.

--- eof?
      Synonym for ((<Apache::Request#eof|eof>)).

--- err_headers_out
      Returns the ((<Apache::Table>)) object for the headers which will be sent
      even when an error occurs, and which persist across internal redirects.

--- error_message
      Returns the error message set by mod_ruby's internal exception-handler, if
      any.

--- escape_html(str)
      Returns the specified string with any '(({&}))', '(({"}))', '(({<}))', or
      '(({>}))' characters escaped to their HTML entity equivalents.

--- exception
      Returns the (({Exception})) object set by mod_ruby's internal
      exception-handler, if any.

--- filename
--- filename=
      Returns/sets the translated physical pathname of the document as
      determined during the URI translation phase.

--- finfo
      Returns the (({File::Stat})) object associated with the translated
      filename of the request, if any. If no physical file is associated with
      the transaction, the File::Stat object will be the same as that returned
      from testing a non-existant file.

--- get_basic_auth_pw
      Returns the plaintext password entered by the user as a String. If there
      was any error fetching the password, a (({SystemExit})) exception is
      raised with its status code set to the status code returned by the call.

--- getc
      Returns the next 8-bit byte (0..255) from the data from the
      client. Returns nil if called at end of file.

--- gets([rs])
      Reads the next "line" from the I/O stream; lines are separated by the
      separator string ((|rs|)), which is (({$/})) by default. A separator of
      nil reads the entire contents, and a zero-length separator reads the
      input a paragraph at a time (two successive newlines in the input
      separate paragraphs). The line read in will be returned and also assigned
      to $_. Returns nil if called at end of file.

--- hard_timeout(msg)
--- kill_timeout
--- reset_timeout
--- soft_timeout(msg)
      Apache timeout interface methods. These methods are only available under
      Apache 1.x.

      (({#hard_timeout})) initiates a "hard" timeout. If an IO operation takes
      longer than the time specified by the (({Timeout})) directive, the current
      handler will be aborted and Apache will immediately enter the logging
      phase.

      (({#soft_timeout})) does not abort the current handler, but returns
      control to it when the timer expires after no-oping all input and output
      methods. After this occurs, ((<Apache::Connection#aborted?|aborted?>))
      will return (({true})).

      (({#reset_timeout})) is used to reset the timer back to zero between
      reads or writes.

      (({#kill_timeout})) cancels the timeout currently in effect when the IO
      operations it governs are finished.

      Example:
        input = ''
        req.hard_timeout( "#{caller(0)[0]}: Reading request." )
        req.each_line {|line|
          input << line
          req.reset_timeout
        }
        req.kill_timeout

        req.sync = true
        req.soft_timeout( "#{caller(0)[0]: Sending response headers." )
        req.send_http_header
        req.kill_timeout

        req.soft_timeout( "#{caller(0)[0]: Sending response data." )
        until output_data.empty?
          bytes = req.write( output_data )
          if bytes.nonzero?
            req.reset_timeout
            output_data.slice!(0,bytes)
          end
        end
        req.kill_timeout
          
--- header_only?
      Returns (({true})) if the request is a head-only request (ie.,
      (({req.request_method == 'HEAD'})).

--- headers_in
      Returns the ((<Apache::Table>)) object for the request header.

--- headers_out
      Returns the ((<Apache::Table>)) object for the response header.

--- hostname
      Returns the hostname, as set by full URI or Host:.

--- initial?
      Returns (({true})) if the request is the initial request (ie., not an
      internal redirect or a subrequest).

--- internal_redirect(uri)
      Redirect the current request internally to the specified (absolute)
      ((|uri|)).

      Example:
        if req.headers_in['user-agent'] !~ /mozilla/i
          req.internal_redirect( "/unsupported-browser.html" )
        end

--- last
      Return the final ((<Apache::Request>)) object for the current chain or
      internal redirects or subrequests.

--- log_reason(msg,file)
      Output a file-processing log message that looks like:
        access to #{file} failed for #{req.get_remote_host},
          reason: #{msg}

--- lookup_file(file)
--- lookup_uri(uri)
      Will perform a sub-request to lookup a given ((|uri|)) or ((|file|)),
      respectively.  The data will not be strongly verified (won't go through
      most of the request cycle), but it will return a new request object that
      you can use to play with and perform operations on.

      For example:
        subr = r.lookup_uri('/non/existent/file.html?asdf=asdf&asdf=asdf')
        subr.status   # ((* => 200 *))
        subr.filename # ((* => '/usr/local/www/data/non' *))
        subr = r.lookup_file('/etc/foo/bar/baz/non/existent/file.html?asdf=asdf&asdf=asdf')
        subr.status   # ((* => 200 *))
        subr.filename # ((* => '/etc/foo/bar/baz/non/existent/file.html' *))

      The moral of the story is that you have to be careful and perform your own
      data verification with a lookup.  If you use (({lookup_file()})), then
      Apache assumes that the filename specified is authoritative.

--- main
      Returns the main ((<Apache::Request>)) object, or (({nil})) if the
      receiver is the main request.

--- main?
      Returns (({true})) if the receiver is the initial request object or an
      internal redirect (ie., not a subrequest).

--- method_number
      Returns the request method as a (({Integer})). You can compare them to the
      ((<request method constants|Request method constants>)) above. 

--- next
      Returns the ((<Apache::Request>)) object for the next (newer) subrequest
      or internal redirect, if any. Returns (({nil})) if no such request exists.

--- note_auth_failure
      Set up the current request's response to indicate a failure to
      authenticate (ie., will send an "Authentication Required" message to the
      browser). It will call either ((<note_basic_auth_failure>)) or
      ((<note_digest_auth_failure>)), depending on which kind of authentication
      is configured for the current directory.

--- note_basic_auth_failure
      Set up the current request's response to indicate a failure to
      authenticate via HTTP Basic Authentication.

--- note_digest_auth_failure
      Set up the current request's response to indicate a failure to
      authenticate via HTTP Digest Authentication.

--- notes
      Returns the ((<Apache::Table>)) object which can be used to pass "notes"
      from one handler module to another.

--- output_buffer
      Returns the output buffer (({String})) currently associated with the request.

--- path_info
--- path_info= str
      Returns/sets the additional path information that remains after the URI
      has been translated into a file path.

--- pos
      Returns the current offset (in bytes) of the client input data stream.

--- pos= n
      Seeks to the given position ((|n|)) (in bytes) in the client input data
      stream.

--- prev
      Returns the ((<Apache::Request>)) object for the previous (older)
      subrequest or internal redirect.

--- print(arg...)
      Writes the given ((|arg|)) object(s) to the output buffer. If the output
      record separator ($\) is not nil, it will be appended to the output. If no
      arguments are given, prints $_. Objects that aren't strings will be
      converted by calling their (({to_s})) method. Returns (({nil})).

--- printf(fmt, arg...)
       Formats and writes to the output buffer, converting parameters under
       control of the ((|fmt|)) string.

--- protocol
      Returns the name and version number of the protocol requested by the
      browser (eg., (({"HTTP/1.1"}))).

--- proxy?
      Returns (({true})) if the request is for a proxy URI.

--- proxy_pass?
      Returns (({true})) if the request is for a pass-through-proxied URL.

--- putc(ch)
       Writes the given character ((|ch|)) (taken from a String or a Fixnum) to
       the output buffer.

--- puts(arg...)
      Writes the given ((|arg|)) objects to the output buffer as with
      ((<Apache::Request#print|print>)) . Writes a record separator (typically a
      newline) after any that do not already end with a newline sequence. If
      called with an array argument, writes each element on a new line. If
      called without arguments, outputs a single record separator.

--- read([len])
      Read ((|len|)) bytes from the client.

--- readchar
      Reads a character as with ((<Apache::Request#getc|getc>)) , but raises an
      (({EOFError})) on end of file.

--- readline([rs])
      Reads a line as with ((<Apache::Request#gets|gets>)) , but raises an
      (({EOFError})) on end of file.

--- readlines([rs])
      Reads all of the lines from the client, and returns them in an
      (({Array})). Lines are separated by the optional separator string
      ((|rs|)).

--- register_cleanup {...}
--- register_cleanup( plain, [child] )
      Register a cleanup handler for the request (((|plain|))) and/or any child
      processes forked by the current child (((|child|))). Either handler may be
      any object which responds to the (({#call})) method (eg., a (({Proc})), a
      (({Method})), etc.). The ((|plain|)) cleanup handler may also be given in
      the form of a block.

--- remote_host([type])
      Returns the remote client's DNS hostname, or its IP address if the
      hostname cannot be looked up. The optional argument specifies what type of
      lookup should be performed. The ((<remotehost constants|Remotehost
      constants>)) can be used for the ((|type|)) argument.

--- remote_logname
      Returns the login name of the remote user if the host is running the
      ((*identd*)) service (RFC 1413), or (({nil})) if the name could not be
      looked up. This method also depends on the server having the
      (({IdentityCheck})) configuration directive turned on, which it is not by
      default.

--- replace(str)
      Replaces the output buffer with ((|str|)).

--- request_method
      Returns the request method as a string (eg., "GET", "HEAD", "POST").

--- request_time
      Returns the time when the request started.

--- requires
      Returns an associative (({Array})) of the (({require})) directives that
      apply to the current request. Each entry is of the form:

        [ method_mask, requirement ]

      where ((|method_mask|)) is a bitmap of the HTTP request methods that the
      requirement applies to, and ((|requirement|)) is the contents of the
      (({require})) directive (ie., everything after the space).

      For example, given a config section like this:

        <Limit GET POST>
          require valid-user
        </Limit>
        <Limit PUT DELETE>
          require group Admin
        </Limit>

      the (({requires})) method would return something like:

        [
          [ 5, "valid-user" ],
          [ 10, "group Admin" ]
        ]

      The bitmask can be tested by left-shifting the mask by the method number,
      eg.,

        get_mask  = 1 << Apache::M_GET
        post_mask = 1 << Apache::M_POST

--- rewind
      Positions the client data stream to the beginning of input, resetting
      (({lineno})) to zero.

--- satisfies
      Returns an (({Integer})) that can be compared with one of the Apache
      module's ((<satisfy constants|Satisfy constants>)) to test the type of
      access control that applies to the request.

--- seek(offset, [whence])
      Seeks to a given offset ((|offset|)) in the stream according to the value
      of ((|whence|)):

      :(({IO::SEEK_CUR}))
        Seeks to ((|offset|)) plus current position.
      :(({IO::SEEK_END}))
        Seeks to anInteger plus end of stream (you probably want a negative
        value for ((|offset|))).
      :(({IO::SEEK_SET})) (the default)
        Seeks to the absolute location given by ((|offset|)).

--- send_fd(io)
      Send the contents of the specified (({IO})) object to the client. Eg.,

        BannerFile = "/www/htdocs/banner.html"

        begin
          File::open( BannerFile, File::O_RDONLY ) {|ofh|
            req.send_fd(ofh)
          }
        rescue IOError => err
          req.log_reason( err.message, BannerFile )
          return Apache::NOT_FOUND
        end
      
--- send_http_header
      Sends the HTTP response header. If you call this method more than once,
      only the first call will actually send it.

--- sent_http_header?
      Returns (({true})) if the header has been sent already.

--- server
      Returns the ((<Apache::Server>)) object associated with the request.

--- server_name
      Returns the server's public name as a (({String})) suitable for inclusion
      in self-referential URLs.

--- server_port
      Returns the port the request was sent to as an (({Integer})) suitable for
      inclusion in self-referential URLs.

--- setup_cgi_env
      Clear the current environment and add CGI and common variables to the
      ((<subprocess_env>)) table.  Then export the ((<subprocess_env>)) table,
      the variables defined in the server and directory configurations for the
      current request, and the (({MOD_RUBY})) and (({GATEWAY_INTERFACE}))
      variables into the environment shared with subprocesses.

--- get_client_block(bufsiz)
--- setup_client_block([policy])
--- should_client_block
--- should_client_block?
      Interface to Apache's internal request-reading functions. The ((|policy|))
      argument accepts one of the
      ((<blocking policy constants|Blocking Policy constants>)).

--- signature
      Returns the server's signature footer line if the server's
      (({ServerSignature})) has been turned on.

--- status
--- status=
      Returns/sets the numeric status code of the transaction.

--- status_line
--- status_line= str
      Returns/sets the full text of the status line returned from Apache to the
      remote browser (eg., (({200 OK}))).

--- subprocess_env
      Returns the ((<Apache::Table>)) object containing environment variables
      which should be passed to subprocesses.

--- sync=
      Set the synchronization of both headers and response body IO.

--- sync_header
--- sync_header= 
      Returns/sets the status of header IO synchronization. If (({sync_header}))
      is (({true})), headers will be sent immediately as they are written, and
      remaining content will be buffered until the end of the request.

--- sync_output
--- sync_output=
      Returns/sets the status of the synchronization of IO for the response
      body. If (({sync_output})) is (({true})), all output will be sent
      immediately instead of buffering it until the end of the request.

--- tell
      Synonym for ((<Apache::Request#pos|pos>)).

--- the_request
      Returns the first line of the request as a (({String})), for logging
      purposes.

--- ungetc(ch)
      Pushes back one character onto the date stream from the client, such that
      a subsequent buffered read will return it. Only one character may be
      pushed back before a subsequent read operation (that is, you will be able
      to read only the last of several characters that have been pushed
      back).

--- unparsed_uri
      Returns the uri without any parsing performed.

--- uri
--- uri= str
      Returns/sets the path portion of the URI.

--- user
--- user= str
      Portably set the authenticated username for the current request. For
      Apache 1.x, calling either of these methods just calls the equivalent
      method of the connection object, but since Apache 2.x moved the username
      into the main request object, this way of setting the username will work
      for either version.

--- write(str)
      Writes the given string ((|str|)) to the output buffer. If the argument is
      not a string, it will be converted to a string using (({to_s})). Returns
      the number of bytes written.


==== Libapreq Support

If mod_ruby has been compiled with support for the Generic Apache Request
library (((<libapreq|URL: http://httpd.apache.org/apreq/>))), then the following
methods will also be available.

--- cookies
--- cookies=
      Get/set the HTTP cookies (RFC 2109) associated with the request as a hash
      of ((<Apache::Cookie>)) objects keyed by cookie name. Note that setting
      the cookies hash does not automatically add them to the response. You must
      call ((<Apache::Cookie#bake|bake>)) on each one to add it to the
      response. ((-This may change in the future-))

--- disable_uploads=
      Turns uploads on/off; If set to a (({true})) value,
      ((<Apache::Request#parse>)) will raise an ((<Apache::RequestError>)) if a
      file upload is attempted.

--- disable_uploads?
--- uploads_disabled?
      Returns (({true})) if uploads are disabled.

--- param(name)
      Returns a single parameter by String.

--- params(name)
      Returns multiple parameters by Array.

--- paramtable
      Returns an ((<Apache::ParamTable>)) object which contains the request's
      parsed parameters. Each parameter will be contained in an
      ((<Apache::MultiVal>)) object, which allows it to be treated either like a
      (({String})) or an (({Array})).

--- parse( [options] )
      If the request method is (({GET})) or (({POST})), the query string
      arguments and the client form data will be read, parsed and saved. In
      addition, if the request method is (({POST})) and the (({Content-type}))
      is (({multipart/form-data})), any uploaded files will be written to
      temporary files which can be accessed with the corresponding
      parameters. The return value is (({OK})) on success; on an error, an error
      code is returned. The optional ((|options|)) hash sets options for the
      parsed request:

      :(({:post_max}))
          Specifies the limit for the size of POST data (in bytes). An
          (({Apache::RequestError})) is raised if the specified size is
          exceeded.

      :(({:disable_uploads}))
          If set to a (({true})) value, an (({Apache::RequestError})) will be
          raised if a file upload is attempted.

      :(({:temp_dir}))
          Specifies the directory where upload files are spooled. See
          ((<Apache::Request#temp_dir|temp_dir>)).

      :(({:upload_hook}))
          Specifies a (({Proc})) or (({Method})) to use as a callback that is
          run whenever file upload data is read. See
          ((<Apache::Request#upload_hook|upload_hook>)).
 
      :(({:hook_data}))
          Set the third argument passed to every call to the (({:upload_hook})),
          if any.

--- post_max
--- post_max=( bytes )
      Get/set the limit for the size of (({POST})) data (in
      bytes). ((<Apache::Request#parse>)) will raise an
      ((<Apache::RequestError>)) if the size is exceeded.


--- temp_dir
--- temp_dir=
      Get/set the directory where upload files are spooled. On a system that
      supports (({link(2)})), the specified direction should be located on the
      same file system as the final destination file.

--- upload_hook
--- upload_hook=
      Specifies a (({Proc})) or (({Method})) to use as a callback that is run
      whenever file upload data is read. This can be used to
      write data to database instead of file, or to provide an upload
      progress meter during file uploads. Apache doesn't write
      the original data to the upload filehandle, so you have to
      write it yourself if needed.
      The ((|buffer|)) argument contains a copy of the input buffer
      read for this chunk of the upload, the ((|upload|)) argument is the
      ((<Apache::Upload>)) object associated with the file being uploaded, and
      ((|arg|)) is whatever was set as the upload hook user argument via
      ((<Apache::Request#upload_hook_data|upload_hook_data>)) or the
      (({:hook_data})) attribute of the configuration hash passed to
      ((<Apache::Request#parse|parse>)).

      Example:
          hook = Proc::new {|buffer,upload,arg|
              request.server.log_debug( "Read %d bytes from upload '%s'",
                                        buffer.length, upload.filename )
              upload.io.write(buffer)
          }
          request.parse( :upload_hook => hook )

--- upload_hook_data
--- upload_hook_data=
      Get/set the object that is passed as the third argument every time the
      ((<Apache::Request#upload_hook|upload_hook>)) is called.

--- uploads
      Returns a hash of any uploaded files as ((<Apache::Upload>)) objects. The
      hash will only be filled if the request method was (({POST})) and the
      request's '(({Content-type}))' was (({multipart/form-data})).

[((<Back to Index|mod_ruby Class Reference Manual>))]
          
== Apache::Cookie

A class for manipulating a request's HTTP cookies. This functionality is only
available if mod_ruby is compiled with Generic Apache Request library (libapreq)
support. A hash of the cookies associated with a request can be fetched by
calling ((<Apache::Request#cookies|cookies>)).

=== Superclass

Object

=== Constants

:DateFormat
  The (({strftime}))-compatible date format used in the ((<#expires|expires>))
  attribute for absolute expirations.

=== Class Methods

--- new( request, [options] )
      Returns a new (({Apache::Cookie})) for the specified ((|request|)) (an
      ((<Apache::Request>)) object). The optional ((|options|)) Hash may be used
      to initialize the cookie's attributes. The following keys are supported:

      :(({:name}))
          Sets the (({name})) field to the given value. 
      :(({:value}))
          Adds the value to the (({values})) field. 
      :(({:expires}))
          Sets the (({expires})) field to the calculated date (({String})) or
          (({Time})) object. See ((<Apache::Cookie#expires|expires>)) for a
          listing of format options. The default is (({nil})).
      :(({:domain}))
          Sets the (({domain})) field to the given value. The default is
          (({nil})).
      :(({:path}))
          Sets the (({path})) field to the given value. The default path is
          derived from the requested uri.
      :(({:secure}))
          Sets the (({secure})) field to (({true})) or (({false})).

=== Methods

--- bake
      Add the cookie to the output headers of the request to which it belongs.

--- domain
--- domain=
      Get/set the (({domain})) attribute of the cookie. From the Netscape spec:
        When searching the cookie list for valid cookies, a comparison of the
        (({domain})) attributes of the cookie is made with the Internet domain
        name of the host from which the URL will be fetched. If there is a tail
        match, then the cookie will go through path matching to see if it should
        be sent. "Tail matching" means that (({domain})) attribute is matched
        against the tail of the fully qualified domain name of the host. A
        (({domain})) attribute of "acme.com" would match host names
        "anvil.acme.com" as well as "shipping.crate.acme.com".

        Only hosts within the specified domain can set a cookie for a domain and
        domains must have at least two (2) or three (3) periods in them to
        prevent domains of the form: ".com", ".edu", and "va.us". Any domain
        that fails within one of the seven special top level domains listed
        below only require two periods. Any other domain requires at least
        three. The seven special top level domains are: "COM", "EDU", "NET",
        "ORG", "GOV", "MIL", and "INT".

        The default value of (({domain})) is the host name of the server which
        generated the cookie response.

--- expires
--- expires=
      Sets the (({expires})) field. The value can be either a (({Time})) object or a
      (({String})) in any of the following formats:

        :(({+30s}))
          30 seconds from now 
        :(({+10m})) 
          ten minutes from now 
        :(({+1h}))
          one hour from now 
        :(({-1d}))
          yesterday (i.e. "ASAP!") 
        :(({now}))
          immediately 
        :(({+3M}))
          in three months 
        :(({+10y}))
          in ten years time 
        :(({Thursday, 25-Apr-1999 00:40:33 GMT}))
          at the indicated time & date

--- name
--- name=
      Get/set the name associated with the cookie.

--- path
--- path=
      Get/set the cookie's (({path})) attribute. From the Netscape spec:
        The (({path})) attribute is used to specify the subset of URLs in a
        domain for which the cookie is valid. If a cookie has already passed
        (({domain})) matching, then the pathname component of the URL is
        compared with the path attribute, and if there is a match, the cookie is
        considered valid and is sent along with the URL request. The path "/foo"
        would match "/foobar" and "/foo/bar.html". The path "/" is the most
        general path.

        If the (({path})) is not specified, it as assumed to be the same path as
        the document being described by the header which contains the cookie.

--- secure=
      Set the cookie's (({secure})) flag to the given value.

--- secure?
      Returns (({true})) if the cookie's (({secure})) flag is set.

--- to_s
      Returns the cookie as a (({String})).

--- value
      Get the first value stored in the cookie as a (({String})).

--- values
      Get all the values stored in the cookie as an (({Array})).

--- value=
      Set the value of the cookie.  If the new value responds to (({#each})),
      (({#each})) will be called, and the result of calling (({#to_s})) on each
      iterated value will be added to the cookie's value. If the new value
      doesn't respond to (({#each})), the result of calling (({#to_s})) on the
      value itself is added.

      For example:
        svarcookie = Apache::Cookie::new( req, :name => 'sessionvars' )
        svarcookie.value = [ Time::now, req.headers_in['host'] ]
        svarcookie.bake

[((<Back to Index|mod_ruby Class Reference Manual>))]


== Apache::MultiVal

(({Apache::MultiVal})) is a multi-valued datatype for Apache request
parameters. Instances of it are used to represent request parameters in a
parameter table in an Apache mod_ruby application. Apache::MultiVal makes each
parameter in the table capable of being treated like both a String and an Array
by delegating String and Array instance methods to either the first value or the
Array of values, respectively. The instance methods which String and Array share
in common are delegated to the String, except #each, #[], and #[]=, which are
sent to the Array.  This allows the code to be kept simple: if you only ever
expect a parameter to have a single value, you can treat it as if it is a
String:

  foo = request.paramtable['foo'].downcase

and treat parameters which can have multiple values (mostly) as an Array:

  bars = request.paramtable['bar'].collect {|val| val.downcase}

For the methods that Array and String share in common, you can cast the
parameter to the object you wish with the normal #to_a and #to_s methods:

  foo = request.paramtable['foo']
  if foo.to_a.length > 1
    request.log_warn( "Request had more than one 'foo' parameter: %s",
                         foo.to_a.inspect )

Of course, the Array's length could be obtained with foo.nitems, too, since
Array#nitems isn't obscured by String's instance methods.


=== Obscured Array methods

As indicated above, some of Array's methods are obscured by those of String,
so you should take special note when using them to be sure you know what
you'll be getting. For the version of Ruby that was most recent as of this
writing (ruby 1.8.0 (2003-06-27)), these are:

"*", "+", "<<", "<=>", "==", "concat", "delete", "empty?", "eql?", "hash",
"include?", "index", "insert", "inspect", "length", "replace", "reverse",
"reverse!", "rindex", "size", "slice", "slice!", "to_s"


== Apache::Upload
      
A class that provides an interface for accessing files uploaded by the
client. This class is only available when mod_ruby is compiled with the Generic
Apache Request Library (libapreq).

=== Superclass

Object

=== Methods

--- filename
      Returns the name of the uploaded file as reported by the client.

--- info
      Returns the header information for the uploaded file as an
      ((<Apache::Table>)) object.

--- io
      Returns a new IO object opened (readonly) to the temporary file associated
      with the upload. Alias: (({fp})).

      Example:
        upload = req.uploads['the-file']
        tempfile = upload.io
        destfile = File::open( "/some/where/thefile.txt", File::WRONLY ) {|safefile|
          tempfile.each {|line| safefile.print(line) }
        }

--- name
      Returns the name of the upload field.

--- size
      Returns the size of the uploaded file in bytes.

--- tempname
      Returns the name of the spool file containing the uploaded data on the
      server.

--- type
      Returns the file's MIME content type. This is a shortcut for accessing the
      uploaded file's 'Content-Type' header:
        upload = req.uploads['the-file']
        upload.info['content-type'] == upload.type


== Apache::Table

A class to wrap (({table})) data type.

=== Superclass

Object

=== Included Classes

Enumerable

=== Methods

--- clear
      Clears contents of the table.

--- self[name]
--- get(name)
      Returns the value of ((|name|)).

--- self[name]= val
--- set(name, val)
--- setn(name, val)
--- merge(name, val)
--- mergen(name, val)
--- add(name, val)
--- addn(name, val)
      Sets the value of ((|name|)) to ((|val|)).

--- unset(name)
      Unsets the value of ((|name|)).

--- each {|key,val|...}
--- each_key {|key|...}
--- each_value {|val|...}
      Iterates over each element.

[((<Back to Index|mod_ruby Class Reference Manual>))]


== Apache::ParamTable

A derivative of ((<Apache::Table>)) that returns ((<Apache::MultiVal>)) objects
for values instead of Strings.

=== Superclass

((<Apache::Table>))

[((<Back to Index|mod_ruby Class Reference Manual>))]


== Apache::Server

A class to wrap global and virtual server configuration and utility methods. Can
be fetched via ((<Apache::Request#server|server>)).

=== Superclass

Object

=== Methods

--- access_confname
      Returns the full location of the access.conf configuration file (if
      any). ((*Not implemented*))

--- admin
      Returns the email address of the server's administrator as set by the
      ServerAdmin directive.

--- defn_line_number
      Returns the line number of the file that the configuration came from.

--- defn_name
      Returns a description of where the configuration came from.

--- document_root
      Returns the server's document root, as configured with the
      (({DocumentRoot})) directive.

--- error_fname
      Return the name of the server's error log, either absolute or server-root
      relative.

--- gid
      Returns the effective server gid.

--- hostname
      Returns the (virtual) name of the server host.

--- is_virtual
--- virtual?
      Returns (({true})) if the server is a virtual host.

--- limit_req_fields
      Returns the limit on the number of request header fields.

--- limit_req_fieldsize
      Returns the limit on the size of any request header field.

--- limit_req_line
      Returns the limit on the number of characters that may be in an HTTP
      request line.

--- log_alert(fmt,*args)
--- log_crit(fmt,*args)
--- log_debug(fmt,*args)
--- log_emerg(fmt,*args)
--- log_error(fmt,*args)
--- log_info(fmt,*args)
--- log_notice(fmt,*args)
--- log_warn(fmt,*args)
      Write a message to the server's log if the server's (({LogLevel})) is the
      specified level or above. The ((|fmt|)) and ((|args|)) are used the same
      way as the arguments to (({printf})).

--- loglevel
      Returns the log level of the server as an (({Integer})) between 1 and 8; 1
      being the least verbose (emerg) and 8 being the most verbose (debug).

--- names
      Returns an Array of server names for the host, starting with the canonical
      name, plus any aliases set with the (({ServerAlias})) directive.

--- path
      Returns the legacy URL pathname for a host, for use with name-based
      virtual hosts. Set with the (({ServerPath})) directive.

--- port
      Return the port number that the (virtual) server is listening on.

--- send_buffer_size
      Returns the size of the TCP send buffer in bytes.

--- srm_confname
      Returns the full location of the srm.conf configuration file (if
      any). ((*Not implemented*))

--- keep_alive
--- keep_alive?
--- keep_alive_max
--- keep_alive_timeout
--- timeout
      Returns the values corresponding to the (({Timeout})),
      (({KeepAliveTimeout})), (({MaxKeepAliveRequests})), and the
      (({KeepAlive})) directives.

--- uid
      Returns the effective server uid.

--- wild_names
      Returns an Array of server names for the host that contain wildcards.


== Apache::Connection

A class to wrap client socket connection records; may be fetched via
((<Apache::Request#connection|connection>)).

=== Superclass

Object

=== Methods

--- aborted?
      Returns (({true})) if a timeout set by
      ((<Apache::Request#soft_timeout|soft_timeout>)) occurs while reading or
      writing to the client.

--- auth_type
--- auth_type= str
      Returns/sets the type of authentication used, if any, as a (({String})).
      These methods are only implemented when running under Apache 1.x.

--- local_host
      Returns the DNS name of the IP address of the local side of the socket
      connection.

--- local_ip
      Returns the dotted Internet address of the local side of the socket
      connection.

--- local_port
      Returns the port number of the local side of the socket connection.


--- remote_host
      Returns the DNS name of the client, if it is set, else the IP address.

--- remote_ip
      Returns the dotted Internet address of the client as a (({String})).

--- remote_logname
      Returns the username obtained via RFC 1413 lookup if the server is doing
      them.

--- remote_port
      Returns the port number of the socket on the client side of the
      connection.

--- user
--- user= str
      Returns/sets the name of the authenticated user, if any. These methods are
      only implemented when running under Apache 1.x, so if you wish to write
      code that works under either version, you should use the
      ((<equivalent methods|user>)) in the ((<Apache::Request>)) object.

=end


