/*
 * $Id$
 * Copyright (C) 2000  ZetaBITS, Inc.
 * Copyright (C) 2000  Information-technology Promotion Agency, Japan
 * Copyright (C) 2000  Shugo Maeda <shugo@modruby.net>
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

#ifndef MOD_RUBY_H
#define MOD_RUBY_H

#include "ruby.h"
#include "rubyio.h"
#include "version.h"
#include "util.h"
#include "intern.h"

#if RUBY_VERSION_CODE < 190 || RUBY_RELEASE_CODE <= 20050304
#define rb_frame_this_func() rb_frame_last_func()
#endif

#if defined(_WIN32)
#undef pid_t
#undef uid_t
#undef gid_t
#endif

#include "httpd.h"
#include "http_config.h"
#include "http_core.h"
#include "http_log.h"
#include "http_main.h"
#include "http_protocol.h"
#include "http_request.h"
#include "util_script.h"

#ifndef RUBY_EXTERN
#define RUBY_EXTERN EXTERN
#endif

#ifndef StringValue
#define StringValue(v) (v = rb_obj_as_string(v))
#endif
#ifndef StringValuePtr
#define StringValuePtr(v) STR2CSTR(v = rb_obj_as_string(v))
#endif
#ifndef SafeStringValue
#define SafeStringValue(v) Check_SafeStr(v)
#endif

#ifdef STANDARD20_MODULE_STUFF /* Apache 2.x */

#define APACHE2

#include "ap_compat.h"
#include "apr_lib.h"
#include "apr_pools.h"
#include "apr_strings.h"
#include "apr_tables.h"

#define ap_pool apr_pool_t

typedef apr_pool_t pool;
typedef apr_array_header_t array_header;
typedef apr_table_t table;
typedef apr_table_entry_t table_entry;
#define APR_CLEANUP_RETURN_TYPE apr_status_t
#define APR_CLEANUP_RETURN_SUCCESS() { \
    return APR_SUCCESS; \
}
#define ap_soft_timeout(s, r) ;
#define ap_hard_timeout(s, r) ;
#define ap_kill_timeout(r) ;

#else /* Apache 1.x */

#define APR_CLEANUP_RETURN_TYPE void
#define APR_CLEANUP_RETURN_SUCCESS() return

#include "http_conf_globals.h"

#endif

/* libapreq */
#define ApacheCookie_new mod_ruby_ApacheCookie_new
#define ApacheCookie_parse mod_ruby_ApacheCookie_parse
#define ApacheCookie_as_string mod_ruby_ApacheCookie_as_string
#define ApacheCookie_attr mod_ruby_ApacheCookie_attr
#define ApacheCookie_expires mod_ruby_ApacheCookie_expires
#define ApacheCookie_bake mod_ruby_ApacheCookie_bake
#define fill_buffer mod_ruby_fill_buffer
#define multipart_buffer_new mod_ruby_multipart_buffer_new
#define multipart_buffer_headers mod_ruby_multipart_buffer_headers
#define multipart_buffer_read mod_ruby_multipart_buffer_read
#define multipart_buffer_read_body mod_ruby_multipart_buffer_read_body
#define multipart_buffer_eof mod_ruby_multipart_buffer_eof
#define ApacheRequest_new mod_ruby_ApacheRequest_new
#define ApacheRequest_parse_multipart mod_ruby_ApacheRequest_parse_multipart
#define ApacheRequest_parse_urlencoded mod_ruby_ApacheRequest_parse_urlencoded
#define ApacheRequest_script_name mod_ruby_ApacheRequest_script_name
#define ApacheRequest_script_path mod_ruby_ApacheRequest_script_path
#define ApacheRequest_param mod_ruby_ApacheRequest_param
#define ApacheRequest_params mod_ruby_ApacheRequest_params
#define ApacheRequest_params_as_string mod_ruby_ApacheRequest_params_as_string
#define ApacheRequest___parse mod_ruby_ApacheRequest___parse
#define ApacheRequest_query_params mod_ruby_ApacheRequest_query_params
#define ApacheRequest_post_params mod_ruby_ApacheRequest_post_params
#define ApacheRequest_tmpfile mod_ruby_ApacheRequest_tmpfile
#define ApacheUpload_new mod_ruby_ApacheUpload_new
#define ApacheUpload_find mod_ruby_ApacheUpload_find
#define ApacheUtil_expires mod_ruby_ApacheUtil_expires
#define ApacheRequest_expires mod_ruby_ApacheRequest_expires
#include "apache_request.h"
#include "apache_multipart_buffer.h"
#include "apache_cookie.h"

#define MOD_RUBY_STRING_VERSION "mod_ruby/1.2.4"
#define RUBY_GATEWAY_INTERFACE "CGI-Ruby/1.1"

typedef struct {
    array_header *load_path;
    table *env;
    int timeout;
    array_header *ruby_child_init_handler;
    int restrict_directives;
} ruby_server_config;

typedef struct {
    char *kcode;
    table *env;
    int safe_level;
    int output_mode;
    array_header *load_path;
    table *options;
    array_header *ruby_handler;
    array_header *ruby_trans_handler;
    array_header *ruby_authen_handler;
    array_header *ruby_authz_handler;
    array_header *ruby_access_handler;
    array_header *ruby_type_handler;
    array_header *ruby_fixup_handler;
    array_header *ruby_log_handler;
    array_header *ruby_header_parser_handler;
    array_header *ruby_post_read_request_handler;
    array_header *ruby_init_handler;
    array_header *ruby_cleanup_handler;
} ruby_dir_config;

typedef struct {
    char *filename;
    ruby_server_config *server_config;
    ruby_dir_config *dir_config;
} ruby_library_context;

typedef struct {
    table *saved_env;
    VALUE request_object;
} ruby_request_config;

#define MR_DEFAULT_TIMEOUT 270
#define MR_DEFAULT_SAFE_LEVEL 1
#define MR_DEFAULT_RESTRICT_DIRECTIVES 0

#define MR_OUTPUT_DEFAULT	0
#define MR_OUTPUT_NOSYNC	1
#define MR_OUTPUT_SYNC		2
#define MR_OUTPUT_SYNC_HEADER	3

/* copied from eval.c */
#define TAG_RETURN      0x1
#define TAG_BREAK       0x2
#define TAG_NEXT        0x3
#define TAG_RETRY       0x4
#define TAG_REDO        0x5
#define TAG_RAISE       0x6
#define TAG_THROW       0x7
#define TAG_FATAL       0x8
#define TAG_MASK        0xf

extern MODULE_VAR_EXPORT module ruby_module;
extern array_header *ruby_required_libraries;

VALUE rb_protect_funcall(VALUE recv, ID mid, int *state, int argc, ...);
void ruby_log_error(const char *file, int line, int level,
		    const server_rec *s, const char *fmt, ...);
void ruby_log_error_string(server_rec *s, VALUE errmsg);
VALUE ruby_get_error_info(int state);
int ruby_running();
void rb_setup_cgi_env(request_rec *r);
void mod_ruby_setup_loadpath(ruby_server_config *sconf,
			     ruby_dir_config *dconf);

#define get_server_config(s) \
	((ruby_server_config *) ap_get_module_config(s->module_config, \
						     &ruby_module))
#define get_dir_config(r) \
	(r->per_dir_config ? \
	   ((ruby_dir_config *) ap_get_module_config(r->per_dir_config, \
						    &ruby_module)) : \
	   NULL)
#define get_request_config(r) \
	((ruby_request_config *) ap_get_module_config(r->request_config, \
						     &ruby_module))

#if APR_HAS_THREADS
typedef void *(*ruby_interp_func_t)(void*);

extern int ruby_is_threaded_mpm;

apr_status_t ruby_call_interpreter(pool *p, ruby_interp_func_t func,
				   void *arg, void **result, int *state);
#endif

#endif /* !MOD_RUBY_H */

/*
 * Local variables:
 * mode: C
 * tab-width: 8
 * End:
 */

/* vim: set filetype=c ts=8 sw=4 : */
