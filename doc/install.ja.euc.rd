=begin

= mod_ruby インストールガイド

[((<目次|URL:index.ja.jis.html>))
|((<RD形式|URL:install.ja.euc.rd>))]

* ((<インストール>))
* ((<Apacheの設定>))
* ((<eRubyの利用>))

== インストール

以下の手順でmod_rubyをインストールします。

  $ tar zxvf mod_ruby-x.y.z.tar.gz
  $ cd mod_ruby-x.y.z/
  $ ./configure.rb --with-apxs=/usr/local/apache/bin/apxs
  $ make
  # make install

== Apacheの設定

以下のような設定をhttpd.confに記述します。

  LoadModule ruby_module /usr/local/apache/libexec/mod_ruby.so
  
  # ClearModuleList
  # AddModule mod_ruby.c

  <IfModule mod_ruby.c>
    RubyRequire apache/ruby-run
    
    # /ruby以下のファイルをRubyスクリプトとして実行する
    <Location /ruby>
    SetHandler ruby-object
    RubyHandler Apache::RubyRun.instance
    </Location>
    
    # *.rbxをRubyスクリプトとして実行する
    <Files *.rbx>
    SetHandler ruby-object
    RubyHandler Apache::RubyRun.instance
    </Files>
  </IfModule>

== eRubyの利用

erubyをインストールし、以下のような設定をhttpd.confに記述します。

  <IfModule mod_ruby.c>
    RubyRequire apache/eruby-run
    
    # /eruby以下のファイルをeRubyファイルとして扱う
    <Location /eruby>
    SetHandler ruby-object
    RubyHandler Apache::ERubyRun.instance
    </Location>
    
    # *.rhtmlをeRubyファイルとして扱う。
    <Files *.rhtml>
    SetHandler ruby-object
    RubyHandler Apache::ERubyRun.instance
    </Files>
  </IfModule>

=end
