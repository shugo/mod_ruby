=begin

= mod_ruby Apache Directives

[((<Index|URL:index.en.html>))
|((<RD format|URL:directives.en.rd>))]

* ((<RubyAddPath|RubyAddPath directory...>))
* ((<RubyRequire|RubyRequire library...>))
* ((<RubyHandler|RubyHandler expr>))
* ((<RubyTransHandler|RubyTransHandler expr>))
* ((<RubyAuthenHandler|RubyAuthenHandler expr>))
* ((<RubyAuthzHandler|RubyAuthzHandler expr>))
* ((<RubyAccessHandler|RubyAccessHandler expr>))
* ((<RubyTypeHandler|RubyTypeHandler expr>))
* ((<RubyFixupHandler|RubyFixupHandler expr>))
* ((<RubyLogHandler|RubyLogHandler expr>))
* ((<RubyHeaderParserHandler|RubyHeaderParserHandler expr>))
* ((<RubyPostReadRequestHandler|RubyPostReadRequestHandler expr>))
* ((<RubyInitHandler|RubyInitHandler expr>))
* ((<RubyCleanupHandler|RubyCleanupHandler expr>))
* ((<RubyPassEnv|RubyPassEnv name...>))
* ((<RubySetEnv|RubySetEnv name val>))
* ((<RubyTimeOut|RubyTimeOut sec>))
* ((<RubySafeLevel|RubySafeLevel level>))
* ((<RubyOutputMode|RubyOutputMode mode>))
* ((<RubyRestrictDirectives|RubyRestrictDirectives flag>))
* ((<RubyKanjiCode|RubyKanjiCode kcode>))

--- RubyAddPath directory...
      Adds a directory to the library search path.

      example:

        RubyAddPath /home/shugo/ruby

--- RubyRequire library...
      Specifies a library or libraries to use with Ruby code.

      example:

        RubyRequire apache/ruby-run
        RubyRequire cgi

--- RubyHandler expr
      Specifies an expression that returns an object for content handler. It
      will call the ((|handler|)) method with the request object.

      example:

        <Location /ruby>
          SetHandler ruby-object
          RubyHandler Apache::RubyRun.instance
        </Location>

--- RubyTransHandler expr
      Specifies an expression that returns an object for URI translation
      handler. It will call the ((|translate_uri|)) method with the request
      object.

      example:

        <Location /ruby>
          RubyTransHandler Apache::Foo.instance
        </Location>

--- RubyAuthenHandler expr
      Specifies an expression that returns an object for the authentication
      handler. It will call the ((|authenticate|)) method with the request
      object.

      example:

        <Location /ruby>
          RubyAuthenHandler Apache::Foo.instance
        </Location>

--- RubyAuthzHandler expr
      Specifies an expression that returns an object for the authorization
      handler. It will call the ((|authorize|)) method with the request object.

      example:

        <Location /ruby>
          RubyAuthzHandler Apache::Foo.instance
        </Location>

--- RubyAccessHandler expr
      Specifies an expression that returns an object for the access handler. It
      will call the ((|check_access|)) method with the request object.

      example:

        <Location /ruby>
          RubyAccessHandler Apache::Foo.instance
        </Location>

--- RubyTypeHandler expr
      Specifies an expression that returns an object for the MIME-type checking
      handler. It will call the ((|find_types|)) method with the request object.

      example:

        <Location /ruby>
          RubyTypeHandler Apache::Foo.instance
        </Location>

--- RubyFixupHandler expr
      Specifies an expression that returns an object for the fixup handler. It
      will call the ((|fixup|)) method with the request object.

      example:

        <Location /ruby>
          RubyFixupHandler Apache::Foo.instance
        </Location>

--- RubyLogHandler expr
      Specifies an expression that returns an object for the logging handler. It
      will call the ((|log_transaction|)) method with the request object.

      example:

        <Location /ruby>
          RubyLogHandler Apache::Foo.instance
        </Location>

--- RubyHeaderParserHandler expr
      Specifies an expression that returns an object for the header parser
      handler. It will call the ((|header_parse|)) method with the request
      object. This handler is only available under Apache 1.x.

      example:

        <Location /ruby>
          RubyHeaderParserHandler Apache::Foo.instance
        </Location>


--- RubyPostReadRequestHandler expr
      Specifies an expression that returns an object for the post-read-request
      handler. It will call the ((|post_read_request|)) method with the request
      object.

      example:

        <Location /ruby>
          RubyPostReadRequestHandler Apache::Foo.instance
        </Location>

--- RubyInitHandler expr
      Specifies an expression that returns an object for the init handler. It
      will call the ((|init|)) method with the request object. If this directive
      is used at the server level (ie., outside of any ((|<Location>|)),
      ((|<Directory>|)), or ((|<Files>|)) directive), it will be run immediately
      before any ((<RubyPostReadRequestHandler>))s. Otherwise, it will be run
      immediately before any ((<RubyHeaderParserHandler>))s.

      example:

        RubyFixupHandler Apache::Foo.instance

        <Location /ruby>
          RubyInitHandler Apache::Foo.instance
        </Location>

--- RubyCleanupHandler expr
      Specifies an expression that returns an object for the cleanup handler. It
      will call the ((|cleanup|)) method with the request object. If this
      directive is inside of a ((|<Location>|)), ((|<Directory>|)), or
      ((|<Files>|)) directive, it will be run after the request is complete. If
      it occurs outside of a ((|<Directory>|)), it will be run at server
      shutdown.

      example:

        RubyCleanupHandler Apache::Foo.instance

        <Location /ruby>
          RubyCleanupHandler Apache::Foo.instance
        </Location>

--- RubyPassEnv name...
      Specifies environment variable names to pass to scripts.
      If this directive is not used, only CGI environment variables
      (such as QUERY_STRING) are passed to Ruby scripts.
      If it is used, all CGI environment variables and the other environment
      variables listed will be available within Ruby scripts.
      Only available in server config.

      example:

        RubyPassEnv HOSTNAME OSTYPE MACHTYPE

--- RubySetEnv name val
      Sets the value of environment variable ((|name|)) to pass to scripts.
      
      example:

        RubySetEnv LANG "ja_JP.eucJP"

--- RubyTimeOut sec
      Specifies the timeout (in seconds) for Ruby scripts.
      Scripts which are still running after the timeout expires will be
      terminated.
      Only available in server config. 

      example:

        RubyTimeOut 60

--- RubySafeLevel level
      Specifies the default value of ((|$SAFE|)).

      $SAFE is the security level. The value of $SAFE should be one of
      the integers from 0 to 4. The default value of $SAFE is 1 on mod_ruby.

      If $SAFE >= 1, Ruby disallows the use of tainted data by potentially
      dangerous operations.

      If $SAFE >= 2, Ruby prohibits the loading of program files from globally
      writable locations.

      If $SAFE >= 3, All newly created objects are considered tainted.

      If $SAFE >= 4, Ruby prohibits the modification of global states such as
      global variables.

      A ((|RubySafeLevel|)) directive in a ((|<Directory>|)), ((|<Location>|)),
      or ((|<Files>|)) section cannot set $SAFE to a value lower than that of
      the server.

      example:

        RubySafeLevel 2

--- RubyOutputMode mode
      Specifies the output mode of scripts. ((|mode|)) should be one of
      (({nosync})), (({sync})), (({syncheader})). If ((|mode|)) is
      (({nosync})), all output of scripts will be buffered, then flushed on
      the end of scripts execution. If ((|mode|)) is (({sync})), all
      output of scripts will be sent to clients immediately.
      If ((|mode|)) is ((|syncheader|)), only header output will be sent
      immediately, then other output will be buffered.
      The default value is ((|nosync|)).

      example:

        RubyOutputMode syncheader

--- RubyRestrictDirectives flag
      Specifies whether all the other Ruby* directives (like
      RubyHandler, RubySetEnv, etc.) are restricted from being
      specified in .htaccess files. Default is (({off})). Setting this
      to (({on})) can be useful in some multi-user situations
      (e.g. shared webhosting), in which the server admin wants to use
      mod_ruby but does not want to allow normal users to get access
      to it. Only available in server config.

      example:

        RubyRestrictDirectives on

--- RubyKanjiCode kcode
      Specifies the value of ((|$KCODE|)).

      $KCODE is the character coding system Ruby handles. If the first
      character of $KCODE is `e' or `E', Ruby handles EUC. If it is `s'
      or `S', Ruby handles Shift_JIS. If it is `u' or `U', Ruby handles
      UTF-8. If it is `n' or `N', Ruby doesn't handle multi-byte
      characters. The default value is "NONE".

      example:

        RubyKanjiCode euc

=end
