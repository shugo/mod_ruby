/*
 * $Id$
 * Copyright (C) 2001  Shugo Maeda <shugo@modruby.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* for core_module */
#define CORE_PRIVATE

#include "mod_ruby.h"
#include "apachelib.h"

VALUE rb_cApacheServer;

VALUE rb_apache_server_new(server_rec *server)
{
    return Data_Wrap_Struct(rb_cApacheServer, NULL, NULL, server);
}

DEFINE_STRING_ATTR_READER(server_defn_name, server_rec, defn_name);
DEFINE_UINT_ATTR_READER(server_defn_line_number, server_rec, defn_line_number);
#ifdef APACHE2
static VALUE server_srm_confname(VALUE self)
{
    rb_notimplement();
    return Qnil;
}

static VALUE server_access_confname(VALUE self)
{
    rb_notimplement();
    return Qnil;
}
#else /* Apache 1.x */
DEFINE_STRING_ATTR_READER(server_srm_confname, server_rec, srm_confname);
DEFINE_STRING_ATTR_READER(server_access_confname, server_rec, access_confname);
#endif
DEFINE_STRING_ATTR_READER(server_admin, server_rec, server_admin);
DEFINE_STRING_ATTR_READER(server_hostname, server_rec, server_hostname);
DEFINE_UINT_ATTR_READER(server_port, server_rec, port);
DEFINE_STRING_ATTR_READER(server_error_fname, server_rec, error_fname);
DEFINE_INT_ATTR_READER(server_loglevel, server_rec, loglevel);
DEFINE_BOOL_ATTR_READER(server_is_virtual, server_rec, is_virtual);
DEFINE_INT_ATTR_READER(server_timeout, server_rec, timeout);
DEFINE_INT_ATTR_READER(server_keep_alive_timeout, server_rec,
		       keep_alive_timeout);
DEFINE_INT_ATTR_READER(server_keep_alive_max, server_rec, keep_alive_max);
DEFINE_BOOL_ATTR_READER(server_keep_alive, server_rec, keep_alive);
#ifdef APACHE2
static VALUE server_send_buffer_size(VALUE self)
{
    rb_notimplement();
    return Qnil;
}
#else /* Apache 1.x */
DEFINE_INT_ATTR_READER(server_send_buffer_size, server_rec, send_buffer_size);
#endif
DEFINE_STRING_ATTR_READER(server_path, server_rec, path);
#ifdef APACHE2
static VALUE server_uid(VALUE self)
{
    rb_notimplement();
    return Qnil;
}

static VALUE server_gid(VALUE self)
{
    rb_notimplement();
    return Qnil;
}
#else /* Apache 1.x */
DEFINE_INT_ATTR_READER(server_uid, server_rec, server_uid);
DEFINE_INT_ATTR_READER(server_gid, server_rec, server_gid);
#endif
DEFINE_INT_ATTR_READER(server_limit_req_line, server_rec, limit_req_line);
DEFINE_INT_ATTR_READER(server_limit_req_fieldsize, server_rec,
		       limit_req_fieldsize);
DEFINE_INT_ATTR_READER(server_limit_req_fields, server_rec, limit_req_fields);

static VALUE server_names(VALUE self)
{
    server_rec *server;

    Data_Get_Struct(self, server_rec, server);
    if (server->names) {
	return rb_apache_array_new(server->names);
    }
    else {
	return Qnil;
    }
}

static VALUE server_wild_names(VALUE self)
{
    server_rec *server;

    Data_Get_Struct(self, server_rec, server);
    if (server->wild_names) {
	return rb_apache_array_new(server->wild_names);
    }
    else {
	return Qnil;
    }
}

static VALUE server_log(int type, int argc, VALUE *argv, VALUE self)
{
    server_rec *server;
    VALUE s;

    Data_Get_Struct(self, server_rec, server);
    s = rb_f_sprintf(argc, argv);
#ifdef APACHE2
    ap_log_error(APLOG_MARK, type | APLOG_NOERRNO, 0, server,
		 "%s", StringValuePtr(s));
#else
    ap_log_error(APLOG_MARK, type | APLOG_NOERRNO, server,
		 "%s", StringValuePtr(s));
#endif
    return Qnil;
}

static VALUE server_log_emerg(int argc, VALUE *argv, VALUE self)
{
    return server_log(APLOG_EMERG, argc, argv, self);
}

static VALUE server_log_alert(int argc, VALUE *argv, VALUE self)
{
    return server_log(APLOG_ALERT, argc, argv, self);
}

static VALUE server_log_crit(int argc, VALUE *argv, VALUE self)
{
    return server_log(APLOG_CRIT, argc, argv, self);
}

static VALUE server_log_error(int argc, VALUE *argv, VALUE self)
{
    return server_log(APLOG_ERR, argc, argv, self);
}

static VALUE server_log_warn(int argc, VALUE *argv, VALUE self)
{
    return server_log(APLOG_WARNING, argc, argv, self);
}

static VALUE server_log_notice(int argc, VALUE *argv, VALUE self)
{
    return server_log(APLOG_NOTICE, argc, argv, self);
}

static VALUE server_log_info(int argc, VALUE *argv, VALUE self)
{
    return server_log(APLOG_INFO, argc, argv, self);
}

static VALUE server_log_debug(int argc, VALUE *argv, VALUE self)
{
    return server_log(APLOG_DEBUG, argc, argv, self);
}

static VALUE server_document_root(VALUE self)
{
    server_rec *server;
    core_server_config *conf;

    Data_Get_Struct(self, server_rec, server);
    conf = (core_server_config *)
	ap_get_module_config(server->module_config, &core_module);
    if (conf->ap_document_root) {
	return rb_tainted_str_new2(conf->ap_document_root);
    }
    else {
	return Qnil;
    }
}

void rb_init_apache_server()
{
    rb_cApacheServer = rb_define_class_under(rb_mApache, "Server", rb_cObject);
    rb_undef_method(CLASS_OF(rb_cApacheServer), "new");
    rb_define_method(rb_cApacheServer, "defn_name", server_defn_name, 0);
    rb_define_method(rb_cApacheServer, "defn_line_number",
		     server_defn_line_number, 0);
    rb_define_method(rb_cApacheServer, "srm_confname", server_srm_confname, 0);
    rb_define_method(rb_cApacheServer, "access_confname", server_access_confname, 0);
    rb_define_method(rb_cApacheServer, "admin", server_admin, 0);
    rb_define_method(rb_cApacheServer, "hostname", server_hostname, 0);
    rb_define_method(rb_cApacheServer, "port", server_port, 0);
    rb_define_method(rb_cApacheServer, "error_fname", server_error_fname, 0);
    rb_define_method(rb_cApacheServer, "loglevel", server_loglevel, 0);
    rb_define_method(rb_cApacheServer, "is_virtual", server_is_virtual, 0);
    rb_define_method(rb_cApacheServer, "virtual?", server_is_virtual, 0);
    rb_define_method(rb_cApacheServer, "timeout", server_timeout, 0);
    rb_define_method(rb_cApacheServer, "keep_alive_timeout",
		     server_keep_alive_timeout, 0);
    rb_define_method(rb_cApacheServer, "keep_alive_max",
		     server_keep_alive_max, 0);
    rb_define_method(rb_cApacheServer, "keep_alive", server_keep_alive, 0);
    rb_define_method(rb_cApacheServer, "keep_alive?", server_keep_alive, 0);
    rb_define_method(rb_cApacheServer, "send_buffer_size",
		     server_send_buffer_size, 0);
    rb_define_method(rb_cApacheServer, "path", server_path, 0);
    rb_define_method(rb_cApacheServer, "names", server_names, 0);
    rb_define_method(rb_cApacheServer, "wild_names", server_wild_names, 0);
    rb_define_method(rb_cApacheServer, "uid", server_uid, 0);
    rb_define_method(rb_cApacheServer, "gid", server_gid, 0);
    rb_define_method(rb_cApacheServer, "limit_req_line",
		     server_limit_req_line, 0);
    rb_define_method(rb_cApacheServer, "limit_req_fieldsize",
		     server_limit_req_fieldsize, 0);
    rb_define_method(rb_cApacheServer, "limit_req_fields",
		     server_limit_req_fields, 0);
    rb_define_method(rb_cApacheServer, "log_emerg",
		     server_log_emerg, -1);
    rb_define_method(rb_cApacheServer, "log_alert",
		     server_log_alert, -1);
    rb_define_method(rb_cApacheServer, "log_crit",
		     server_log_crit, -1);
    rb_define_method(rb_cApacheServer, "log_error",
		     server_log_error, -1);
    rb_define_method(rb_cApacheServer, "log_warn",
		     server_log_warn, -1);
    rb_define_method(rb_cApacheServer, "log_notice",
		     server_log_notice, -1);
    rb_define_method(rb_cApacheServer, "log_info",
		     server_log_info, -1);
    rb_define_method(rb_cApacheServer, "log_debug",
		     server_log_debug, -1);
    rb_define_method(rb_cApacheServer, "document_root",
		     server_document_root, 0);
}

/*
 * Local variables:
 * mode: C
 * tab-width: 8
 * End:
 */

/* vim: set filetype=c ts=8 sw=4 : */
