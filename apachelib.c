/*
 * $Id: apachelib.c,v 1.42 2004/01/27 13:53:33 shugo Exp $
 * Copyright (C) 2000  ZetaBITS, Inc.
 * Copyright (C) 2000  Information-technology Promotion Agency, Japan
 * Copyright (C) 2001-2003  Shugo Maeda <shugo@modruby.net>
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

#ifndef WIN32
#include <unistd.h>
#endif


#include "mod_ruby.h"
#include "apachelib.h"

VALUE rb_mApache;
VALUE rb_eApacheTimeoutError;

#ifdef HAVE_LIBAPREQ
VALUE rb_eApacheRequestError;
#endif

VALUE rb_request;
VALUE rb_apache_objrefs;

void rb_apache_exit(int status)
{
    VALUE exit;

    exit = rb_exc_new(rb_eSystemExit, 0, 0);
    rb_iv_set(exit, "status", INT2NUM(status));
    rb_exc_raise(exit);
}

VALUE rb_apache_register_object(VALUE obj)
{
    rb_hash_aset(rb_apache_objrefs, rb_obj_id(obj), obj);
    return obj;
}

VALUE rb_apache_unregister_object(VALUE obj)
{
    rb_protect_funcall(rb_apache_objrefs,
		       rb_intern("delete"), NULL, 1, rb_obj_id(obj));
    return obj;
}

static VALUE f_exit(int argc, VALUE *argv, VALUE obj)
{
    VALUE status;
    int status_code;

    rb_secure(4);
    if (rb_scan_args(argc, argv, "01", &status) == 1) {
	status_code = NUM2INT(status);
	if (status_code < 0)
	    rb_raise(rb_eArgError, "negative status code %d", status_code);
    }
    else {
	status_code = OK;
    }
    rb_apache_exit(status_code);
    return Qnil;		/* not reached */
}

static VALUE f_eval_string_wrap(VALUE self, VALUE str)
{
    if (rb_safe_level() >= 4) {
	Check_Type(str, T_STRING);
    }
    else {
	Check_SafeStr(str);
    }
    return rb_eval_string_wrap(StringValuePtr(str), NULL);
}

static VALUE apache_server_version(VALUE self)
{
    return rb_str_new2(ap_get_server_version());
}

#ifdef APACHE2 /* Apache 2.x */
static VALUE apache_add_version_component(VALUE self, VALUE component)
{
    rb_notimplement();
    return Qnil;
}
#else /* Apache 1.x */
static VALUE apache_add_version_component(VALUE self, VALUE component)
{
    ap_add_version_component(StringValuePtr(component));
    return Qnil;
}
#endif

static VALUE apache_server_built(VALUE self)
{
    return rb_str_new2(ap_get_server_built());
}

static VALUE apache_request(VALUE self)
{
    return rb_request;
}

static VALUE apache_unescape_url(VALUE self, VALUE url)
{
    char *buf;

    Check_Type(url, T_STRING);
    buf = ALLOCA_N(char, RSTRING(url)->len + 1);
    memcpy(buf, RSTRING(url)->ptr, RSTRING(url)->len);
    buf[RSTRING(url)->len] = '\0';
    ap_unescape_url(buf);
    return rb_str_new2(buf);
}

#ifdef APACHE2 /* Apache 2.x */
static void ap_chdir_file(const char *file)
{
    const char *x;
    char buf[HUGE_STRING_LEN];

    x = strrchr(file, '/');
    if (x == NULL) {
	chdir(file);
    }
    else if (x - file < sizeof(buf) - 1) {
	memcpy(buf, file, x - file);
	buf[x - file] = '\0';
	chdir(buf);
    }
}
#endif

static VALUE apache_chdir_file(VALUE self, VALUE file)
{
    ap_chdir_file(StringValuePtr(file));
    return Qnil;
}

static VALUE apache_server_root(VALUE self)
{
    return rb_str_new2(ap_server_root);
}

void rb_init_apache()
{
    rb_request = Qnil;
    rb_global_variable(&rb_request);
    rb_apache_objrefs = rb_hash_new();
    rb_global_variable(&rb_apache_objrefs);

    rb_define_global_function("exit", f_exit, -1);
    rb_define_global_function("eval_string_wrap", f_eval_string_wrap, 1);

    rb_mApache = rb_define_module("Apache");
    rb_define_module_function(rb_mApache, "server_version", apache_server_version, 0);
    rb_define_module_function(rb_mApache, "add_version_component",
			      apache_add_version_component, 1);
    rb_define_module_function(rb_mApache, "server_built", apache_server_built, 0);
    rb_define_module_function(rb_mApache, "request", apache_request, 0);
    rb_define_module_function(rb_mApache, "unescape_url", apache_unescape_url, 1);
    rb_define_module_function(rb_mApache, "chdir_file", apache_chdir_file, 1);
    rb_define_module_function(rb_mApache, "server_root", apache_server_root, 0);

    rb_eApacheTimeoutError =
	rb_define_class_under(rb_mApache, "TimeoutError", rb_eException);

    rb_define_const(rb_mApache, "DECLINED", INT2NUM(DECLINED));
    rb_define_const(rb_mApache, "DONE", INT2NUM(DONE));
    rb_define_const(rb_mApache, "OK", INT2NUM(OK));

    /* HTTP status codes */
    rb_define_const(rb_mApache, "HTTP_CONTINUE",
		    INT2NUM(HTTP_CONTINUE));
    rb_define_const(rb_mApache, "HTTP_SWITCHING_PROTOCOLS",
		    INT2NUM(HTTP_SWITCHING_PROTOCOLS));
    rb_define_const(rb_mApache, "HTTP_PROCESSING",
		    INT2NUM(HTTP_PROCESSING));
    rb_define_const(rb_mApache, "HTTP_OK",
		    INT2NUM(HTTP_OK));
    rb_define_const(rb_mApache, "HTTP_CREATED",
		    INT2NUM(HTTP_CREATED));
    rb_define_const(rb_mApache, "HTTP_ACCEPTED",
		    INT2NUM(HTTP_ACCEPTED));
    rb_define_const(rb_mApache, "HTTP_NON_AUTHORITATIVE",
		    INT2NUM(HTTP_NON_AUTHORITATIVE));
    rb_define_const(rb_mApache, "HTTP_NO_CONTENT",
		    INT2NUM(HTTP_NO_CONTENT));
    rb_define_const(rb_mApache, "HTTP_RESET_CONTENT",
		    INT2NUM(HTTP_RESET_CONTENT));
    rb_define_const(rb_mApache, "HTTP_PARTIAL_CONTENT",
		    INT2NUM(HTTP_PARTIAL_CONTENT));
    rb_define_const(rb_mApache, "HTTP_MULTI_STATUS",
		    INT2NUM(HTTP_MULTI_STATUS));
    rb_define_const(rb_mApache, "HTTP_MULTIPLE_CHOICES",
		    INT2NUM(HTTP_MULTIPLE_CHOICES));
    rb_define_const(rb_mApache, "HTTP_MOVED_PERMANENTLY",
		    INT2NUM(HTTP_MOVED_PERMANENTLY));
    rb_define_const(rb_mApache, "HTTP_MOVED_TEMPORARILY",
		    INT2NUM(HTTP_MOVED_TEMPORARILY));
    rb_define_const(rb_mApache, "HTTP_SEE_OTHER",
		    INT2NUM(HTTP_SEE_OTHER));
    rb_define_const(rb_mApache, "HTTP_NOT_MODIFIED",
		    INT2NUM(HTTP_NOT_MODIFIED));
    rb_define_const(rb_mApache, "HTTP_USE_PROXY",
		    INT2NUM(HTTP_USE_PROXY));
    rb_define_const(rb_mApache, "HTTP_TEMPORARY_REDIRECT",
		    INT2NUM(HTTP_TEMPORARY_REDIRECT));
    rb_define_const(rb_mApache, "HTTP_BAD_REQUEST",
		    INT2NUM(HTTP_BAD_REQUEST));
    rb_define_const(rb_mApache, "HTTP_UNAUTHORIZED",
		    INT2NUM(HTTP_UNAUTHORIZED));
    rb_define_const(rb_mApache, "HTTP_PAYMENT_REQUIRED",
		    INT2NUM(HTTP_PAYMENT_REQUIRED));
    rb_define_const(rb_mApache, "HTTP_FORBIDDEN",
		    INT2NUM(HTTP_FORBIDDEN));
    rb_define_const(rb_mApache, "HTTP_NOT_FOUND",
		    INT2NUM(HTTP_NOT_FOUND));
    rb_define_const(rb_mApache, "HTTP_METHOD_NOT_ALLOWED",
		    INT2NUM(HTTP_METHOD_NOT_ALLOWED));
    rb_define_const(rb_mApache, "HTTP_NOT_ACCEPTABLE",
		    INT2NUM(HTTP_NOT_ACCEPTABLE));
    rb_define_const(rb_mApache, "HTTP_PROXY_AUTHENTICATION_REQUIRED",
		    INT2NUM(HTTP_PROXY_AUTHENTICATION_REQUIRED));
    rb_define_const(rb_mApache, "HTTP_REQUEST_TIME_OUT",
		    INT2NUM(HTTP_REQUEST_TIME_OUT));
    rb_define_const(rb_mApache, "HTTP_CONFLICT",
		    INT2NUM(HTTP_CONFLICT));
    rb_define_const(rb_mApache, "HTTP_GONE",
		    INT2NUM(HTTP_GONE));
    rb_define_const(rb_mApache, "HTTP_LENGTH_REQUIRED",
		    INT2NUM(HTTP_LENGTH_REQUIRED));
    rb_define_const(rb_mApache, "HTTP_PRECONDITION_FAILED",
		    INT2NUM(HTTP_PRECONDITION_FAILED));
    rb_define_const(rb_mApache, "HTTP_REQUEST_ENTITY_TOO_LARGE",
		    INT2NUM(HTTP_REQUEST_ENTITY_TOO_LARGE));
    rb_define_const(rb_mApache, "HTTP_REQUEST_URI_TOO_LARGE",
		    INT2NUM(HTTP_REQUEST_URI_TOO_LARGE));
    rb_define_const(rb_mApache, "HTTP_UNSUPPORTED_MEDIA_TYPE",
		    INT2NUM(HTTP_UNSUPPORTED_MEDIA_TYPE));
    rb_define_const(rb_mApache, "HTTP_RANGE_NOT_SATISFIABLE",
		    INT2NUM(HTTP_RANGE_NOT_SATISFIABLE));
    rb_define_const(rb_mApache, "HTTP_EXPECTATION_FAILED",
		    INT2NUM(HTTP_EXPECTATION_FAILED));
    rb_define_const(rb_mApache, "HTTP_UNPROCESSABLE_ENTITY",
		    INT2NUM(HTTP_UNPROCESSABLE_ENTITY));
    rb_define_const(rb_mApache, "HTTP_LOCKED",
		    INT2NUM(HTTP_LOCKED));
#ifdef HTTP_FAILED_DEPENDENCY
    rb_define_const(rb_mApache, "HTTP_FAILED_DEPENDENCY",
			      INT2NUM(HTTP_FAILED_DEPENDENCY));
#endif
    rb_define_const(rb_mApache, "HTTP_INTERNAL_SERVER_ERROR",
		    INT2NUM(HTTP_INTERNAL_SERVER_ERROR));
    rb_define_const(rb_mApache, "HTTP_NOT_IMPLEMENTED",
		    INT2NUM(HTTP_NOT_IMPLEMENTED));
    rb_define_const(rb_mApache, "HTTP_BAD_GATEWAY",
		    INT2NUM(HTTP_BAD_GATEWAY));
    rb_define_const(rb_mApache, "HTTP_SERVICE_UNAVAILABLE",
		    INT2NUM(HTTP_SERVICE_UNAVAILABLE));
    rb_define_const(rb_mApache, "HTTP_GATEWAY_TIME_OUT",
		    INT2NUM(HTTP_GATEWAY_TIME_OUT));
    rb_define_const(rb_mApache, "HTTP_VERSION_NOT_SUPPORTED",
		    INT2NUM(HTTP_VERSION_NOT_SUPPORTED));
    rb_define_const(rb_mApache, "HTTP_VARIANT_ALSO_VARIES",
		    INT2NUM(HTTP_VARIANT_ALSO_VARIES));
#ifdef HTTP_INSUFFICIENT_STORAGE
    rb_define_const(rb_mApache, "HTTP_INSUFFICIENT_STORAGE",
			      INT2NUM(HTTP_INSUFFICIENT_STORAGE));
#endif
    rb_define_const(rb_mApache, "HTTP_NOT_EXTENDED",
		    INT2NUM(HTTP_NOT_EXTENDED));

    rb_define_const(rb_mApache, "DOCUMENT_FOLLOWS",
		    INT2NUM(HTTP_OK));
    rb_define_const(rb_mApache, "PARTIAL_CONTENT",
		    INT2NUM(HTTP_PARTIAL_CONTENT));
    rb_define_const(rb_mApache, "MULTIPLE_CHOICES",
		    INT2NUM(HTTP_MULTIPLE_CHOICES));
    rb_define_const(rb_mApache, "MOVED",
		    INT2NUM(HTTP_MOVED_PERMANENTLY));
    rb_define_const(rb_mApache, "REDIRECT",
		    INT2NUM(HTTP_MOVED_TEMPORARILY));
    rb_define_const(rb_mApache, "USE_LOCAL_COPY",
		    INT2NUM(HTTP_NOT_MODIFIED));
    rb_define_const(rb_mApache, "BAD_REQUEST",
		    INT2NUM(HTTP_BAD_REQUEST));
    rb_define_const(rb_mApache, "AUTH_REQUIRED",
		    INT2NUM(HTTP_UNAUTHORIZED));
    rb_define_const(rb_mApache, "FORBIDDEN",
		    INT2NUM(HTTP_FORBIDDEN));
    rb_define_const(rb_mApache, "NOT_FOUND",
		    INT2NUM(HTTP_NOT_FOUND));
    rb_define_const(rb_mApache, "METHOD_NOT_ALLOWED",
		    INT2NUM(HTTP_METHOD_NOT_ALLOWED));
    rb_define_const(rb_mApache, "NOT_ACCEPTABLE",
		    INT2NUM(HTTP_NOT_ACCEPTABLE));
    rb_define_const(rb_mApache, "LENGTH_REQUIRED",
		    INT2NUM(HTTP_LENGTH_REQUIRED));
    rb_define_const(rb_mApache, "PRECONDITION_FAILED",
		    INT2NUM(HTTP_PRECONDITION_FAILED));
    rb_define_const(rb_mApache, "SERVER_ERROR",
		    INT2NUM(HTTP_INTERNAL_SERVER_ERROR));
    rb_define_const(rb_mApache, "NOT_IMPLEMENTED",
		    INT2NUM(HTTP_NOT_IMPLEMENTED));
    rb_define_const(rb_mApache, "BAD_GATEWAY",
		    INT2NUM(HTTP_BAD_GATEWAY));
    rb_define_const(rb_mApache, "VARIANT_ALSO_VARIES",
		    INT2NUM(HTTP_VARIANT_ALSO_VARIES));

    rb_define_const(rb_mApache, "M_GET", INT2NUM(M_GET));
    rb_define_const(rb_mApache, "M_PUT", INT2NUM(M_PUT));
    rb_define_const(rb_mApache, "M_POST", INT2NUM(M_POST));
    rb_define_const(rb_mApache, "M_DELETE", INT2NUM(M_DELETE));
    rb_define_const(rb_mApache, "M_CONNECT", INT2NUM(M_CONNECT));
    rb_define_const(rb_mApache, "M_OPTIONS", INT2NUM(M_OPTIONS));
    rb_define_const(rb_mApache, "M_TRACE", INT2NUM(M_TRACE));
    rb_define_const(rb_mApache, "M_PATCH", INT2NUM(M_PATCH));
    rb_define_const(rb_mApache, "M_PROPFIND", INT2NUM(M_PROPFIND));
    rb_define_const(rb_mApache, "M_PROPPATCH", INT2NUM(M_PROPPATCH));
    rb_define_const(rb_mApache, "M_MKCOL", INT2NUM(M_MKCOL));
    rb_define_const(rb_mApache, "M_COPY", INT2NUM(M_COPY));
    rb_define_const(rb_mApache, "M_MOVE", INT2NUM(M_MOVE));
    rb_define_const(rb_mApache, "M_LOCK", INT2NUM(M_LOCK));
    rb_define_const(rb_mApache, "M_UNLOCK", INT2NUM(M_UNLOCK));
    rb_define_const(rb_mApache, "M_INVALID", INT2NUM(M_INVALID));
    rb_define_const(rb_mApache, "METHODS", INT2NUM(METHODS));

    rb_define_const(rb_mApache, "OPT_NONE", INT2NUM(OPT_NONE));
    rb_define_const(rb_mApache, "OPT_INDEXES", INT2NUM(OPT_INDEXES));
    rb_define_const(rb_mApache, "OPT_INCLUDES", INT2NUM(OPT_INCLUDES));
    rb_define_const(rb_mApache, "OPT_SYM_LINKS", INT2NUM(OPT_SYM_LINKS));
    rb_define_const(rb_mApache, "OPT_EXECCGI", INT2NUM(OPT_EXECCGI));
    rb_define_const(rb_mApache, "OPT_UNSET", INT2NUM(OPT_UNSET));
    rb_define_const(rb_mApache, "OPT_INCNOEXEC", INT2NUM(OPT_INCNOEXEC));
    rb_define_const(rb_mApache, "OPT_SYM_OWNER", INT2NUM(OPT_SYM_OWNER));
    rb_define_const(rb_mApache, "OPT_MULTI", INT2NUM(OPT_MULTI));
    rb_define_const(rb_mApache, "OPT_ALL", INT2NUM(OPT_ALL));

    rb_define_const(rb_mApache, "SATISFY_ALL", INT2NUM(SATISFY_ALL));
    rb_define_const(rb_mApache, "SATISFY_ANY", INT2NUM(SATISFY_ANY));
    rb_define_const(rb_mApache, "SATISFY_NOSPEC", INT2NUM(SATISFY_NOSPEC));

    rb_define_const(rb_mApache, "REMOTE_HOST",
		    INT2NUM(REMOTE_HOST));
    rb_define_const(rb_mApache, "REMOTE_NAME",
		    INT2NUM(REMOTE_NAME));
    rb_define_const(rb_mApache, "REMOTE_NOLOOKUP",
		    INT2NUM(REMOTE_NOLOOKUP));
    rb_define_const(rb_mApache, "REMOTE_DOUBLE_REV",
		    INT2NUM(REMOTE_DOUBLE_REV));

    /* Policy constants for setup_client_block() */
    rb_define_const(rb_mApache, "REQUEST_NO_BODY",
		    INT2NUM(REQUEST_NO_BODY));
    rb_define_const(rb_mApache, "REQUEST_CHUNKED_ERROR",
		    INT2NUM(REQUEST_CHUNKED_ERROR));
    rb_define_const(rb_mApache, "REQUEST_CHUNKED_DECHUNK",
		    INT2NUM(REQUEST_CHUNKED_DECHUNK));
#ifdef REQUEST_CHUNKED_PASS
    rb_define_const(rb_mApache, "REQUEST_CHUNKED_PASS",
		    INT2NUM(REQUEST_CHUNKED_PASS));
#endif

    rb_init_apache_array();
    rb_init_apache_table();
    rb_init_apache_connection();
    rb_init_apache_server();
    rb_init_apache_request();

#ifdef HAVE_LIBAPREQ
    rb_eApacheRequestError = 
	rb_define_class_under(rb_mApache, "RequestError", rb_eException);

    rb_init_apache_multival();
    rb_init_apache_paramtable();
    rb_init_apache_upload();
    rb_init_apache_cookie();
#endif

}

/*
 * Local variables:
 * mode: C
 * tab-width: 8
 * End:
 */
