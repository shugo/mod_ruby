=begin

= apache/rails-dispatcher.rb

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

Apache::RailsDispatcher dispatches requests to Rails applications.

== Example of httpd.conf

  RubySafeLevel 0
  RubyRequire apache/rails-dispatcher
  RubyTransHandler Apache::RailsDispatcher.instance
  <Location /appname>
    SetHandler ruby-object
    RubyHandler Apache::RailsDispatcher.instance
    RubyOption rails_uri_root /appname
    RubyOption rails_root /path/to/rails/root
    RubyOption rails_env development
  </Location>

=end

require "set"
require "singleton"
require "active_support"
require "active_record"
require "action_controller"
require "action_mailer"
require "action_web_service"
require "active_support/whiny_nil"

module Apache
  class RailsDispatcher
    include Singleton

    @@environments = {}
    @@current_environment = nil

    def self.current_environment
      return @@current_environment
    end

    def translate_uri(r)
      return DECLINED unless r.options.key?("rails_uri_root")
      re = Regexp.new("\\A" + Regexp.quote(r.options["rails_uri_root"]) + "/*",
                      nil, "n")
      filename = File.expand_path(r.uri.sub(re, "public/"),
                                  r.options["rails_root"])
      if File.file?(filename)
        r.filename = filename
        return OK
      end
      return DECLINED
    end

    def handler(r)
      if !/\Adispatch\.(cgi|fcgi|rb)\z/.match(r.filename) &&
        File.file?(r.filename)
        return DECLINED
      end
      r.setup_cgi_env
      Object.const_set(:RAILS_ROOT, r.options["rails_root"])
      ENV["RAILS_ENV"] = r.options["rails_env"] || "development"
      Object.const_set(:RAILS_ENV, ENV["RAILS_ENV"])
      Dependencies.mechanism = :load
      env = get_environment(r.options["rails_root"])
      # set classpath for Marshal
      Apache::RailsDispatcher.const_set(:CURRENT_MODULE, env.module)
      Dependencies.loaded = env.loaded_dependencies
      @@current_environment = env
      begin
        env.eval_string("Apache::RailsDispatcher.instance.dispatch")
      ensure
        @@current_environment = nil
        env.loaded_dependencies = Dependencies.loaded.reject { |filename|
          /\Aenvironments\//.match(filename)
        }
        Apache::RailsDispatcher.send(:remove_const, :CURRENT_MODULE)
        Object.remove_const(:RAILS_ROOT)
        Object.remove_const(:RAILS_ENV)
        reset_configurations
      end
      return OK
    end

    def dispatch
      r = Apache.request
      old_constants = @@current_environment.module.constants
      @@current_environment.load_environment
      conf_constants =
        (@@current_environment.module.constants - old_constants).select { |c|
        /\A(RAILS_ROOT|RAILS_ENV|ADDITIONAL_LOAD_PATHS|RAILS_DEFAULT_LOGGER|[A-Z_]+_(CONF|PATH|DIR)|Controllers)\z/.match(c)
      }
      begin
        if /\A\/*\z/.match(r.options["rails_uri_root"])
          ActionController::AbstractRequest.relative_url_root = nil
        else
          ActionController::AbstractRequest.relative_url_root =
            r.options["rails_uri_root"]
        end
        cgi = CGI.new
        session_options = ActionController::CgiRequest::DEFAULT_SESSION_OPTIONS
        session_options = session_options.merge({:session_path => r.options["rails_uri_root"]})
        request = ActionController::CgiRequest.new(cgi, session_options)
        response = ActionController::CgiResponse.new(cgi)
        prepare_application
        ActionController::Routing::Routes.recognize!(request).process(request, response).out(r)
      rescue Exception => exception
        ActionController::Base.process_with_exception(request, response, exception).out(r)
      ensure
        if r.options.key?("rails_cleanup")
          begin
            @@current_environment.eval_string(r.options["rails_cleanup"],
                                              "rails_cleanup")
          rescue Exception => e
            r.server.log_error("%s: %s", e.class, e.message)
            for line in e.backtrace
              r.server.log_error("  %s", line)
            end
          end
        end
        for c in conf_constants
          @@current_environment.remove_const(c)
        end
        reset_after_dispatch
      end
    end

    private

    def get_environment(rails_root)
      env = @@environments[rails_root]
      unless env
        env = @@environments[rails_root] = Environment.new(rails_root)
      end
      return env
    end
    
    def prepare_application
      ActionController::Routing::Routes.reload if Dependencies.load?
      unless Controllers.const_defined?(:ApplicationController)
        Controllers.const_load!(:ApplicationController, "application")
      end
    end
    
    def reset_after_dispatch
      reset_application! if Dependencies.load?
    end

    def reset_application!
      @@environments.delete(@@current_environment.rails_root)
      Dependencies.clear
    end

    def reset_configurations
      for target_class, defaults in DEFAULT_CONFIGURATIONS
        for setter, val in defaults
          target_class.send(setter, val)
        end
      end
      ActionController::CgiRequest::DEFAULT_SESSION_OPTIONS.replace(DEFAULT_SESSION_OPTIONS)
    end

    def self.get_configuration_methods(target_class)
      return target_class.singleton_methods.collect { |m|
        m.slice(/(\w+)=\z/n, 1)
      }.reject { |m|
        m.nil? || /\A(connection|table_name|timestamps_gmt)\z/.match(m)
      }
    end

    DEFAULT_CONFIGURATIONS = [
      ActiveRecord::Base,
      ActiveRecord::Errors,
      ActionController::Base,
      ActionView::Base,
      ActionMailer::Base,
    ].inject({}) { |defaults, target_class|
      getters = get_configuration_methods(target_class)
      defaults[target_class] = getters.inject({}) { |defs, getter|
        defs[getter + "="] = target_class.send(getter)
        defs
      }
      defaults
    }
    DEFAULT_SESSION_OPTIONS =
      ActionController::CgiRequest::DEFAULT_SESSION_OPTIONS.dup


    class Environment
      attr_reader :rails_root, :binding, :module, :loaded_files, :controllers
      attr_accessor :loaded_dependencies

      def initialize(rails_root)
        @rails_root = rails_root
        @binding = eval_string_wrap("binding")
        @module = setup_module
        @loaded_files = Set.new
        @loaded_dependencies = []
        @controllers = nil
      end

      def eval_string(s, filename = "(eval)", lineno = 1)
        eval(s, @binding, filename, lineno)
      end

      def load_environment
        environment_path = File.expand_path("config/environment.rb",
                                            @rails_root)
        load_file(environment_path)
        if @controllers
          remove_const(:Controllers)
          @module.const_set(:Controllers, @controllers)
        else
          @controllers = @module.const_get(:Controllers)
        end
      end

      def load_file(filename)
        file = $:.collect { |dir|
          File.expand_path(filename, dir)
        }.detect { |f| File.file?(f) } || filename
        begin
          eval_string(File.read(file), file, 1)
          return true
        rescue Errno::ENOENT => e
          raise LoadError.new("no such file to load -- #{filename}")
        end
      end

      def require_file(filename)
        if @loaded_files.include?(filename) || $:.include?(filename) ||
          $".include?(filename + ".rb") || $".include?(filename + ".so")
          return false
        end
        file = filename
        file += ".rb" unless /\.rb\z/.match(filename)
        begin
          load_file(file)
        rescue LoadError
          Kernel.require(filename)
        end
        @loaded_files.add(filename)
        return true
      end

      def remove_const(name)
        begin
          @module.send(:remove_const, name)
        rescue NameError
        end
      end

      private

      def setup_module
        mod = eval_string("class << self; self; end.ancestors.first")
        env = self
        mod.send(:define_method, :load) do |filename|
          env.load_file(filename)
        end
#        mod.send(:define_method, :require) do |filename|
#          env.require_file(filename)
#        end
        return mod
      end
    end
  end
end

module Dependencies
  def require_or_load(file_name)
    file_name = "#{file_name}.rb" unless ! load? || file_name[-3..-1] == '.rb'
    env = Apache::RailsDispatcher.current_environment
    if env
      load? ? env.load_file(file_name) : env.require_file(file_name)
    else
      load? ? load(file_name) : require(file_name)
    end
  end
end

def Object.const_get(name)
  return super(name) if self != Object
  if Apache::RailsDispatcher.current_environment
    mod = Apache::RailsDispatcher.current_environment.module
    if mod.const_defined?(name)
      return mod.const_get(name)
    end
    return super(name)
  end
end

def Object.const_defined?(name)
  return super(name) if self != Object
  if Apache::RailsDispatcher.current_environment
    return super(name) ||
      Apache::RailsDispatcher.current_environment.module.const_defined?(name)
  else
    return super(name)
  end
end

def Object.const_set(name, val)
  return super(name, val) if self != Object
  if Apache::RailsDispatcher.current_environment
    Apache::RailsDispatcher.current_environment.module.const_set(name, val)
  else
    super(name, val)
  end
end

def Object.remove_const(name)
  return super(name) if self != Object
  if Apache::RailsDispatcher.current_environment
    begin
      mod = Apache::RailsDispatcher.current_environment
      mod.send(:remove_const, name)
    rescue NameError
      super(name)
    end
  else
    super(name)
  end
end

class Module
  alias name__ name

  def name
    return name__.sub(/\AApache::RailsDispatcher::CURRENT_MODULE::/, "")
  end

  def const_missing(class_id)
    env = Apache::RailsDispatcher.current_environment
    unless env
      super(class_id)
    end
    if env.module.const_defined?(class_id)
      return env.module.const_get(class_id)
    end
    if class_id != :Controllers && Object.const_defined?(:Controllers) &&
      self != Controllers && Controllers.const_available?(class_id)
      return Controllers.const_get(class_id)
    end
    begin
      require_dependency(class_id.to_s.demodulize.underscore)
      if env.module.const_defined?(class_id)
        return env.module.const_get(class_id)
      else
        raise LoadError
      end
    rescue LoadError => e
      raise NameError.new("uninitialized constant #{class_id}").copy_blame!(e)
    end
  end
end

class << Marshal
  alias load__ load

  def load(*args)
    begin
      return load__(*args)
    rescue ArgumentError => e
      name = e.message.slice(/undefined class\/module Apache::RailsDispatcher::CURRENT_MODULE::(\w+)/u, 1)
      if name
        raise ArgumentError.new("undefined class/module #{name}")
      else
        raise
      end
    end
  end
end
