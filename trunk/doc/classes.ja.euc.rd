=begin

= mod_ruby クラスリファレンス

[((<目次|URL:index.ja.jis.html>))
|((<RD形式|URL:classes.ja.euc.rd>))]

* ((<Apache>))
* ((<Apache::Request>))
* ((<Apache::Table>))

== Apache

Apacheの機能を提供するモジュール。

=== モジュール関数

--- server_version
      サーバのバージョン情報を表す文字列を返す。

--- server_built
      サーバがビルドされた日付を表す文字列を返す。

--- request
      現在のリクエストを表す((<Apache::Request>))オブジェクトを返す。

--- unescape_url(str)
      URLエンコードされた文字列のデコードを行う。

[((<目次に戻る|mod_ruby クラスリファレンス>))]

== Apache::Request

request_rec(リクエスト情報を表現するApacheのデータ型)をラップするクラス。

=== スーパークラス

Object

=== インクルードされているモジュール

Enumerable

=== メソッド

--- hostname
      フルURIやHost:で与えるられたホスト名を返す。

--- unparsed_uri
      パースされていないURIを返す。

--- uri
      URIのパス部を返す。

--- filename
      スクリプトのファイル名を返す。

--- path_info
      PATH_INFOを返す。

--- request_time
      リクエストの開始時刻を表すTimeオブジェクトを返す。

--- request_method
      リクエストメソッド(GET, HEAD, POST)を返す。

--- header_only?
      HEADリクエストの場合、真を返す。

--- args
      リクエストの引数を返す。

--- headers_in
      リクエストへッダを表す((<Apache::Table>))オブジェクトを返す。

--- read([len])
--- gets([rs])
--- readline([rs])
--- readlines([rs])
--- each([rs]) {|line|...}
--- each_line([rs]) {|line|...}
--- each_byte {|ch|...}
--- getc
--- readchar
--- ungetc(ch)
--- tell
--- seek(offset, [whence])
--- rewind
--- pos
--- pos= n
--- eof
--- eof?
--- binmode
      クライアントからのデータを受け取る。
      各メソッドはIOの同名のメソッドと同様に動作する。

--- status_line= str
      ステータスラインに((|str|))を指定する。

--- status_line
      指定されたステータスラインを返す。

--- headers_out
      レスポンスへッダを表す((<Apache::Table>))オブジェクトを返す。
      レスポンスへッダは((<send_http_header>))により出力される。

--- content_type= str
      レスポンスへッダのContent-Typeを指定する。

--- content_type
      指定されたContent-Typeを返す。

--- content_encoding= str
      レスポンスへッダのContent-Encodingを指定する。

--- content_encoding
      指定されたContent-Encodingを返す。

--- content_languages= str
      レスポンスへッダのContent-Languagesを指定する。

--- content_languages
      指定されたContent-Languagesを返す。

--- send_http_header
      レスポンスへッダをクライアントに送信する。
      二回以上呼び出しても一度しか出力されない。

--- write(str)
--- putc(ch)
--- print(arg...)
--- printf(fmt, arg...)
--- puts(arg...)
--- << obj
      クライアントに出力する。
      各メソッドはIOの同名のメソッドと同様に動作する。

--- replace(str)
      クライアントへの出力を保持しているバッファの内容を((|str|))で
      置き換える。

--- cancel
      クライアントへの出力を保持しているバッファの内容を破棄する。

--- escape_html(str)
      &"<>などの文字をエスケープする。

[((<目次に戻る|mod_ruby クラスリファレンス>))]

== Apache::Table

table(Apacheのデータ型)をラップするクラス。

=== スーパークラス

Object

=== インクルードされたクラス

Enumerable

=== メソッド

--- clear
      テーブルの内容を破棄する。

--- self[name]
--- get(name)
      ((|name|))に対応するデータを取得する。

--- self[name]= val
--- set(name, val)
--- setn(name, val)
--- merge(name, val)
--- mergen(name, val)
--- add(name, val)
--- addn(name, val)
      ((|name|))に対応するデータを設定する。
      各メソッドの違いはよくわからない。

--- unset(name)
      ((|name|))に対応するデータを取り除く。

--- each {|key,val|...}
--- each_key {|key|...}
--- each_value {|val|...}
      各要素に対してブロックを実行する。

[((<目次に戻る|mod_ruby クラスリファレンス>))]

=end
