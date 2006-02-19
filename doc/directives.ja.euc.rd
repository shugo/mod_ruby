=begin

= mod_ruby Apacheディレクティブ

[((<目次|URL:index.ja.jis.html>))
|((<RD形式|URL:directives.ja.euc.rd>))]

* ((<RubyAddPath|RubyAddPath directory...>))
* ((<RubyRequire|RubyRequire library...>))
* ((<RubyHandler|RubyHandler expr>))
* ((<RubyPassEnv|RubyPassEnv name...>))
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
* ((<RubySetEnv|RubySetEnv name val>))
* ((<RubyTimeOut|RubyTimeOut sec>))
* ((<RubySafeLevel|RubySafeLevel level>))
* ((<RubyOutputMode|RubyOutputMode mode>))
* ((<RubyRestrictDirectives|RubyRestrictDirectives flag>))
* ((<RubyGcPerRequest|RubyGcPerRequest flag>))
* ((<RubyKanjiCode|RubyKanjiCode kcode>))

--- RubyAddPath directory...
      ライブラリの検索パスにディレクトリを追加する。

      例:

        RubyAddPath /home/shugo/ruby

--- RubyRequire library...
      ((|library|))を(({require}))する。

      例:

        RubyRequire apache/ruby-run
        RubyRequire cgi

--- RubyHandler expr
      ruby-objectハンドラで実際に処理を行うオブジェクトを返す式を指定する。
      requestオブジェクトを引数に((|handler|))メソッドが呼ばれる。

      例:

        <Location /ruby>
          SetHandler ruby-object
          RubyHandler Apache::RubyRun.instance
        </Location>

--- RubyTransHandler expr
      URIの書き換えを行うオブジェクトを返す式を指定する。
      requestオブジェクトを引数に((|translate_uri|))メソッドが呼ばれる。

      例:

        <Location /ruby>
          RubyTransHandler Apache::Foo.instance
        </Location>

--- RubyAuthenHandler expr
      ユーザIDとパスワードの確認を行うオブジェクトを返す式を指定する。
      requestオブジェクトを引数に((|authenticate|))メソッドが呼ばれる。

      例:

        <Location /ruby>
          RubyAuthenHandler Apache::Foo.instance
        </Location>

--- RubyAuthzHandler expr
      リソースが権限を必要とするかどうかの確認を行うオブジェクトを返す式
      を指定する。
      requestオブジェクトを引数に((|authorize|))メソッドが呼ばれる。

      例:

        <Location /ruby>
          RubyAuthzHandler Apache::Foo.instance
        </Location>

--- RubyAccessHandler expr
      アクセス制約を行うオブジェクトを返す式を指定する。
      requestオブジェクトを引数に((|check_access|))メソッドが呼ばれる。

      例:

        <Location /ruby>
          RubyAccessHandler Apache::Foo.instance
        </Location>

--- RubyTypeHandler expr
      MIME-typeのチェックを行うオブジェクトを返す式を指定する。
      requestオブジェクトを引数に((|find_types|))メソッドが呼ばれる。

      例:

        <Location /ruby>
          RubyTypeHandler Apache::Foo.instance
        </Location>

--- RubyFixupHandler expr
      応答の内容を最後に変更するオブジェクトを返す式を指定する。
      requestオブジェクトを引数に((|fixup|))メソッドが呼ばれる。

      例:

        <Location /ruby>
          RubyFixupHandler Apache::Foo.instance
        </Location>

--- RubyLogHandler expr
      リクエストについての情報を記録するオブジェクトを返す式を指定する。
      requestオブジェクトを引数に((|log_transaction|))メソッドが呼ばれる。

      例:

        <Location /ruby>
          RubyLogHandler Apache::Foo.instance
        </Location>

--- RubyHeaderParserHandler expr
      ヘッダのパースをするオブジェクトを返す式を指定する。
      requestオブジェクトを引数に((|log_transaction|))メソッドが呼ばれる。
      Apache 1.x でのみ使用できる。

      例:

        <Location /ruby>
          RubyHeaderParserHandler Apache::Foo.instance
        </Location>


--- RubyPostReadRequestHandler expr
      リクエストを読みこんだ後、他のフェーズの前に呼ばれるオブジェクトを
      返す式を指定する。
      requestオブジェクトを引数に((|post_read_request|))メソッドが呼ばれる。

      例:

        <Location /ruby>
          RubyPostReadRequestHandler Apache::Foo.instance
        </Location>

--- RubyInitHandler expr
      子プロセル起動直後に呼ばれるオブジェクトを返す式を指定する。
      requestオブジェクトを引数に((|init|))メソッドが呼ばれる。
      サーバレベルの設定(つまり((|<Location>|))や((|<Directory>|))や
      ((|<Files>|))の外)で指定すると、((<RubyPostReadRequestHandler>))よ
      りも前に実行され、そうでなければ((<RubyHeaderParserHandler>))の前に
      実行される。

      例:

        RubyInitHandler Apache::Foo.instance

        <Location /ruby>
          RubyInitHandler Apache::Foo.instance
        </Location>

--- RubyCleanupHandler expr
      クリーンアップ時に呼ばれるオブジェクトを返す式を指定する。
      リクエスト完了時に、requestオブジェクトを引数に((|cleanup|))メソッドが
      呼ばれる。

      例:

        RubyCleanupHandler Apache::Foo.instance

        <Location /ruby>
          RubyCleanupHandler Apache::Foo.instance
        </Location>

--- RubyPassEnv name...
      スクリプトに受け渡す環境変数の名前を指定する。
      このディレクティブが指定されなかった場合、CGI環境変数(QUERY_STRINGなど)
      のみがRubyスクリプトに渡される。
      このティレクティブが指定された場合、すべてのCGI環境変数と、リストされた
      他の環境変数が、Rubyスクリプトから利用可能となる。
      サーバ設定のみに指定可。

      例:

        RubyPassEnv HOSTNAME OSTYPE MACHTYPE

--- RubySetEnv name val
      スクリプトに受け渡す環境変数の値を指定する。

      例:

        RubySetEnv LANG "ja_JP.eucJP"

--- RubyTimeOut sec
      スクリプトのタイムアウト時間(単位は秒)を指定する。
      タイムアウト時間が過ぎても実行されているスクリプトは強制的に
      終了される。
      サーバ設定のみに指定可。

      例:

        RubyTimeOut 60

--- RubySafeLevel level
      ((|$SAFE|))のデフォルト値を指定する。
      サーバ設定のみに指定可。

      $SAFEはセキュリティレベルである。
      $SAFEの値は0から4までの整数でなければならない。
      mod_rubyでは$SAFEのデフォルト値は1である。

      $SAFEが1以上の場合、Rubyはユーザが汚染されたデータを潜在的危険性のある
      操作に利用するのを禁止する。

      $SAFEが2以上の場合、Rubyはだれでも書き込むことのできる場所からプログラム
      ファイルをロードすることを禁止する。
      writable locations.

      $SAFEが3以上の場合、新しく生成されたすべてのオブジェクトは汚染される。

      $SAFEが4以上の場合、Rubyはグローバル変数などのグローバルな状態の変更を
      禁止する。

      例:

        RubySafeLevel 2

--- RubyOutputMode mode
      スクリプトの出力モードを指定する。((|mode|))は(({nosync})), (({sync})),
      (({syncheader}))のうちのどれか一つでなければならない。((|mode|))が
      (({nosync}))の場合、スクリプトのすべての出力はバッファリングされ、スクリプ
      トの実行の終了時にフラッシュされる。((|mode|))が(({sync}))の場合、スクリプ
      トのすべての出力はただちにクライントに送信される。((|mode|))が
      (({syncheader}))の場合、へッダの出力のみがただちに送信され、他の出力はバッ
      ファリングされる。デフォルト値は(({nosync}))。
      
      例:

        RubyOutputMode syncheader

--- RubyRestrictDirectives flag
      他の「Rubyで始まるディレクティブ(RubyHandlerやRubySetEnvなど)」
      を.htaccessで制限するかどうかを指定する。デフォルトは(({off}))。
      共有サーバなど複数のユーザで利用する場合などで(({on}))にすると、
      サーバ管理者だけがmod_rubyを使うことができる。サーバ設定でのみ指定
      することができる。

      例:

        RubyRestrictDirectives on

--- RubyGcPerRequest flag
      各リクエストの処理後にGCを実行するかどうかを指定する。

      例:

        RubyGcPerRequest on

--- RubyKanjiCode kcode
      Rubyの((|$KCODE|))を指定する。デフォルト値は(({"NONE"}))。

      例:

        RubyKanjiCode euc

=end
