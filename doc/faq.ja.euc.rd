=begin

= mod_ruby FAQ

[((<目次|URL:index.ja.jis.html>))
|((<RD形式|URL:faq.ja.euc.rd>))]

* ((<mod_rubyとは何ですか?>))
* ((<どこでmod_rubyを入手できますか?>))
* ((<バイナリパッケージはありますか?>))
* ((<mod_rubyのメーリングリストはありますか?>))
* ((<mod_rubyは安全ですか?>))
* ((<重い処理をするスクリプトにも効果がありますか?>))
* ((<Windowsで動きますか?>))
* ((<LoadModuleが反映されません。>))
* ((<なぜContent-Typeがブラウザに表示されるのですか?>))
* ((<ライブラリの変更が反映されないのですが?>))
* ((<SecurityErrorが発生します。>))
* ((<Locationへッダを出力しても上手く動きません。>))
* ((<CGI::Sessionが上手く動きません。>))
* ((<なぜかエラーが出ます。>))
* ((<Apacheが落ちます。>))
* ((<(({apachectl restart}))でメモリリークします。>))
* ((<libruby.soやliberuby.soが見つかりません。>))
* ((<既存のクラスを再定義できません。>))

== mod_rubyとは何ですか?

mod_rubyはRubyインタプリタをApache(WWWサーバ)に組み込み、Apacheが直接
RubyのCGIスクリプトを実行できるようにします。
mod_rubyを使うと、通常のCGIよりもスクリプトは高速に起動されます。

[((<目次に戻る|mod_ruby FAQ>))]

== どこでmod_rubyを入手できますか?

mod_rubyの公式ウェブサイト((<URL:http://www.modruby.net/>))
で入手できます。

[((<目次に戻る|mod_ruby FAQ>))]

== バイナリパッケージはありますか?

Debian GNU/LinuxやFreeBSDでは標準パッケージとして提供されています。

RPMは((<Vine Linux|URL:http://www.vinelinux.org/>))で提供されています。

[((<目次に戻る|mod_ruby FAQ>))]

== mod_rubyのメーリングリストはありますか?

英語のメーリングリスト((<URL:mailto:modruby@modruby.net>))と
日本語のメーリングリスト((<URL:mailto:modruby-ja@modruby.net>)))があります。

購読するには((<URL:mailto:modruby-ctl@modruby.net>))
あるいは((<URL:mailto:modruby-ja-ctl@modruby.net>))に、

  subscribe Your-First-Name Your-Last-Name

のような本文のメールを送ってください。

[((<目次に戻る|mod_ruby FAQ>))]

== mod_rubyは安全ですか?

その答は「はい」であり「いいえ」です。

mod_rubyのデフォルトの$SAFEの値は1なので、CGIプログラマのミスによって
セキュリティホールを作ってしまう可能性は低くなります。たとえば、
eval(cgi["foo"][0])はSecurityErrorになります。

一方、mod_rubyでは複数のスクリプトが一つのRubyインタプリタを利用するため、
あるスクリプトがグローバルな状態を変更すると、他のスクリプトにも影響が
及びます。
したがって、ISPなどで信頼できない不特定多数のユーザにmod_rubyの使用を
許すようなことは避けてください。

[((<目次に戻る|mod_ruby FAQ>))]

== 重い処理をするスクリプトにも効果がありますか?

重い処理をする(実行時間が長い)スクリプトでは起動にかかるコストは
相対的に小さくなるため、mod_rubyによるプロセス生成コストの削減の
効果はあまりないのではないか、と考える人もいるかもしれません。

しかし、テスト結果によると、実際にはそのようなスクリプトでも、特に
高負荷時に大きな効果があることが確認されています。
これは、mod_rubyでは実行されるプロセス数が大幅に減るため、サーバ
マシンへの負荷が軽減されるためだと考えられます。

[((<目次に戻る|mod_ruby FAQ>))]

== Windowsで動きますか?

私は(幸運にも)Windowsマシンを持っていないので、まだ動きません。

Apache 2.0をサポートするようになったら動くかもしれません。

[((<目次に戻る|mod_ruby FAQ>))]

== LoadModuleが反映されません。

httpd.confでClearModuleListが記述されている場合は、それ以降に
以下のように記述する必要があります。

  AddModule mod_ruby.c

[((<目次に戻る|mod_ruby FAQ>))]

== なぜContent-Typeがブラウザに表示されるのですか?

次のスクリプトはmod_ruby上では正しく動作しません。

  print "Content-Type: text/plain\r\n\r\n"
  print "hello world"

これはmod_rubyはNPH-CGIであり、HTTPステータスラインを
出力しないためです。
次のように自分でHTTPステータスラインを出力する必要が
あります。

  print "HTTP/1.1 200 OK\r\n"
  print "Content-Type: text/plain\r\n\r\n"
  print "hello world"

あるいはcgi.rbを利用することもできます。

  require "cgi"
  
  cgi = CGI.new
  print cgi.header("type"=>"text/plain")
  print "hello world"

このスクリプトの方が前のものよりよいでしょう。

[((<目次に戻る|mod_ruby FAQ>))]

== ライブラリの変更が反映されないのですが?

mod_rubyでは複数のスクリプトで一つのRubyインタプリタを共有します。
(({require 'foo'}))を実行した場合、ライブラリがロードされるのは
最初の一回だけなので、その後(({require 'foo'}))を実行しても
ライブラリが再びロードされることはありません。

  # apachectl stop
  # apachectl start

として、Apacheを再起動するか、デバッグ中は(({require}))の代りに
(({load}))を使ってください。

[((<目次に戻る|mod_ruby FAQ>))]

== SecurityErrorが発生します。

mod_rubyではデフォルトの(({$SAFE}))の値は(({1}))になるため、
汚染された文字列で危険な操作を行うとSecurityErrorが発生します。
その操作が危険でないと分かっている場合は、(({untaint}))で
汚染された文字列を浄化してやることでSecurityErrorを回避できます。

  query = CGI.new
  filename = query.params["filename"][0].dup
  filename.untaint
  file = open(filename)

不用意な(({untaint}))はセキュリティホールを作ってしまう可能性が
あるので、十分注意してください。

[((<目次に戻る|mod_ruby FAQ>))]

== Locationへッダを出力しても上手く動きません。

HTTPステータスラインで200 OKを返す(デフォルトの動作)と、
Locationへッダを与えてもそのURLにジャンプしてくれません。

  r = Apache.request
  r.status_line = "302 Found"
  r.headers_out["Location"] = "http://www.modruby.net/"
  r.content_type = "text/html; charset=iso-8859-1"
  r.send_http_header
  print "<html><body><h1>302 Found</h1></body></html>"

のようにステータスラインを指定するか、

  r = Apache.request
  r.headers_out["Location"] = "http://www.modruby.net/"
  exit Apache::REDIRECT

のように終了ステータスを設定してください。

[((<目次に戻る|mod_ruby FAQ>))]

== CGI::Sessionが上手く動きません。

mod_rubyではCGI::Sessionは自動的にクローズされません。
したがって明示的にクローズしてやる必要があります。

  session = CGI::Session.new(...)
  begin
    ...
  ensure
    session.close
  end

この問題はCGI::Session特有のものではありません。
ファイルなども同様にクローズする必要があります。

[((<目次に戻る|mod_ruby FAQ>))]

== なぜかエラーが出ます。

スクリプトのバグなどでインタプリタの状態がおかしくなると、その後の
スクリプトの実行に問題が出る可能性があります。
そのような場合はApacheを再起動してください。

[((<目次に戻る|mod_ruby FAQ>))]

== Apacheが落ちます。

Rubyのコンパイルにegcs-1.1.2を使っている場合、ApacheがSegmentation Faultで
落ちる時があります。
その場合、Rubyの最新版(1.6.2以降)で試してみてください。

それ以外の場合で落ちる時は作者の((<前田修吾|URL:mailto:shugo@modruby.net>))
に報告していただけるとありがたいです。

[((<目次に戻る|mod_ruby FAQ>))]

== (({apachectl restart}))でメモリリークします。

現状のRubyのAPIではインタプリタがmalloc()したメモリをfree()する方法がないため、
apachectl restartを実行するたびに、Apacheが使用するメモリが増えていってしまいます。

当面、

  # apachectl stop
  # apachectl start

のように、Apacheをいったん停止させてから、再び起動するようにしてください。

[((<目次に戻る|mod_ruby FAQ>))]

== libruby.soやliberuby.soが見つかりません。

おそらく、libruby.soがインストールされた場所が、実行時のライブラリの
検索パスに、含まれていません。

多くのLinuxディストリビューションでは、/usr/local/libは実行時のライブラリの
検索パスに含まれません。
mod_rubyを/usr/local以下にインストールする場合は、次のような行を
/etc/ld.so.confに追加し、ldconfigを実行してください。

  /usr/local/lib

[((<目次に戻る|mod_ruby FAQ>))]

== 既存のクラスを再定義できません。

mod_rubyスクリプト中で直接既存のクラスを再定義することはできません。
(代りに、新しいクラスが定義されます。)
なぜなら、mod_rubyスクリプトはKernel#load(filename, true)によって
ロードされるからです。

既存のクラスを再定義する必要がある場合は、ライブラリの中で再定義を行い、
mod_rubyスクリプトからそのライブラリをrequireするようにしてください。

[((<目次に戻る|mod_ruby FAQ>))]

=end
