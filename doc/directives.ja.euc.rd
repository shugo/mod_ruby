=begin

= mod_ruby Apacheディレクティブ

[((<目次|URL:index.ja.jis.html>))
|((<RD形式|URL:directives.ja.euc.rd>))]

* ((<RubyAddPath|RubyAddPath directory...>))
* ((<RubyRequire|RubyRequire library...>))
* ((<RubyHandler|RubyHandler expr>))
* ((<RubyPassEnv|RubyPassEnv name...>))
* ((<RubySetEnv|RubySetEnv name val>))
* ((<RubyTimeOut|RubyTimeOut sec>))
* ((<RubySafeLevel|RubySafeLevel level>))
* ((<RubyOutputMode|RubyOutputMode mode>))
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

      例:

        <Location /ruby>
          SetHandler ruby-object
          RubyHandler Apache::RubyRun.instance
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

--- RubyKanjiCode kcode
      Rubyの((|$KCODE|))を指定する。デフォルト値は(({"NONE"}))。

      例:

        RubyKanjiCode euc

=end
