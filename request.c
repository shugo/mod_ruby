/*
 * $Id$
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

#include <sys/types.h>
#include <sys/stat.h>

/* for core_module */
#define CORE_PRIVATE

#include "mod_ruby.h"
#include "apachelib.h"

#include "st.h"

static VALUE rb_eApachePrematureChunkEndError;
VALUE rb_cApacheRequest;

/* Parse options */
static ID id_post_max, id_disable_uploads,
    id_temp_dir, id_hook_data, id_upload_hook;

typedef struct request_data {
    request_rec *request;
    VALUE outbuf;
    VALUE connection;
    VALUE server;
    VALUE headers_in;
    VALUE headers_out;
    VALUE err_headers_out;
    VALUE subprocess_env;
    VALUE notes;
    VALUE finfo;
    VALUE parsed_uri;
    VALUE attributes;
    VALUE error_message;
    VALUE exception;
    ApacheRequest *apreq;
    VALUE upload_hook;
    VALUE upload_hook_arg;
    VALUE upload_table;
    VALUE cookies;
    VALUE param_table;
    VALUE options;
} request_data;

#define REQ_SYNC_HEADER     FL_USER1
#define REQ_SYNC_OUTPUT     FL_USER2
#define REQ_HEADER_PENDING  FL_USER3
#define REQ_SENT_HEADER     FL_USER4

#define REQUEST_STRING_ATTR_READER(fname, member) \
	DEFINE_STRING_ATTR_READER(fname, request_data, request->member)

#ifdef APACHE2
#define REQUEST_STRING_ATTR_WRITER(fname, member) \
static VALUE fname(VALUE self, VALUE val) \
{ \
    request_data *data; \
    Check_Type(val, T_STRING); \
    data = get_request_data(self); \
    data->request->member = \
	apr_pstrndup(data->request->pool, \
		    RSTRING(val)->ptr, RSTRING(val)->len); \
    return val; \
}
#else
#define REQUEST_STRING_ATTR_WRITER(fname, member) \
static VALUE fname(VALUE self, VALUE val) \
{ \
    request_data *data; \
    Check_Type(val, T_STRING); \
    data = get_request_data(self); \
    data->request->member = \
	ap_pstrndup(data->request->pool, \
		    RSTRING(val)->ptr, RSTRING(val)->len); \
    return val; \
}
#endif

#define REQUEST_INT_ATTR_READER(fname, member) \
	DEFINE_INT_ATTR_READER(fname, request_data, request->member)
#define REQUEST_INT_ATTR_WRITER(fname, member) \
	DEFINE_INT_ATTR_WRITER(fname, request_data, request->member)
#define REQUEST_BOOL_ATTR_READER(fname, member) \
	DEFINE_BOOL_ATTR_READER(fname, request_data, request->member)
#define REQUEST_BOOL_ATTR_WRITER(fname, member) \
	DEFINE_BOOL_ATTR_WRITER(fname, request_data, request->member)

static void request_mark(request_data *data)
{
    if (data == NULL) return;
    rb_gc_mark(data->outbuf);
    rb_gc_mark(data->connection);
    rb_gc_mark(data->server);
    rb_gc_mark(data->headers_in);
    rb_gc_mark(data->headers_out);
    rb_gc_mark(data->err_headers_out);
    rb_gc_mark(data->subprocess_env);
    rb_gc_mark(data->notes);
    rb_gc_mark(data->finfo);
    rb_gc_mark(data->parsed_uri);
    rb_gc_mark(data->attributes);
    rb_gc_mark(data->error_message);
    rb_gc_mark(data->exception);
    rb_gc_mark(data->upload_hook);
    rb_gc_mark(data->upload_hook_arg);
    rb_gc_mark(data->upload_table);
    rb_gc_mark(data->cookies);
    rb_gc_mark(data->param_table);
    rb_gc_mark(data->options);
}

static APR_CLEANUP_RETURN_TYPE cleanup_request_object(void *data)
{
    request_rec *r = (request_rec *) data;
    ruby_request_config *rconf;
    VALUE reqobj;

    if (r->request_config == NULL)
	APR_CLEANUP_RETURN_SUCCESS();
    rconf = get_request_config(r);
    if (rconf == NULL) APR_CLEANUP_RETURN_SUCCESS();
    reqobj = rconf->request_object;
    if (TYPE(reqobj) == T_DATA) {
	free(RDATA(reqobj)->data);
	RDATA(reqobj)->data = NULL;
    }
    ap_set_module_config(r->request_config, &ruby_module, NULL);
#if APR_HAS_THREADS
    if (ruby_is_threaded_mpm) {
	ruby_call_interpreter(r->pool,
			      (ruby_interp_func_t) rb_apache_unregister_object,
			      (void *) reqobj, NULL, 0);
    }
    else {
#endif
	rb_apache_unregister_object(reqobj);
#if APR_HAS_THREADS
    }
#endif
    APR_CLEANUP_RETURN_SUCCESS();
}

static VALUE apache_request_new(request_rec *r)
{
    ruby_dir_config *dconf = get_dir_config(r);
    request_data *data;
    VALUE obj;
    const array_header *opts_arr;
    table_entry *opts;
    int i;

    obj = Data_Make_Struct(rb_cApacheRequest, request_data,
			   request_mark, free, data);
    data->request = r;
    data->outbuf = rb_tainted_str_new("", 0);
    data->connection = Qnil;
    data->server = Qnil;
    data->headers_in = Qnil;
    data->headers_out = Qnil;
    data->err_headers_out = Qnil;
    data->subprocess_env = Qnil;
    data->notes = Qnil;
    data->finfo = Qnil;
    data->parsed_uri = Qnil;
    data->attributes = Qnil;
    data->error_message = Qnil;
    data->exception = Qnil;
    data->apreq = ApacheRequest_new( r );
    data->upload_hook = Qnil;
    data->upload_hook_arg = Qnil;
    data->upload_table = rb_hash_new();
    data->cookies = rb_hash_new();
    data->param_table = Qnil;
    data->options = rb_hash_new();
    if (dconf) {
        opts_arr = apr_table_elts(dconf->options);
        opts = (table_entry *) opts_arr->elts;
        for (i = 0; i < opts_arr->nelts; i++) {
            if (opts[i].key == NULL)
                continue;
            rb_hash_aset(data->options,
                         rb_tainted_str_new2(opts[i].key),
                         rb_tainted_str_new2(opts[i].val));
        }
    }
    
    rb_apache_register_object(obj);
    if (r->request_config) {
	ruby_request_config *rconf = get_request_config(r);
	if (rconf)
	    rconf->request_object = obj;
    }
    apr_pool_cleanup_register(r->pool, (void *) r,
			cleanup_request_object, apr_pool_cleanup_null);
    if (dconf) {
	switch (dconf->output_mode) {
	    case MR_OUTPUT_SYNC_HEADER:
		FL_SET(obj, REQ_SYNC_HEADER);
		break;
	    case MR_OUTPUT_SYNC:
		FL_SET(obj, REQ_SYNC_HEADER);
		FL_SET(obj, REQ_SYNC_OUTPUT);
		break;
	    case MR_OUTPUT_NOSYNC:
	    default:
		break;
	}
    }
    return obj;
}

VALUE rb_get_request_object(request_rec *r)
{
    if (r == NULL) return Qnil;
    if (r->request_config) {
	ruby_request_config *rconf = get_request_config(r);
	if (rconf && !NIL_P(rconf->request_object))
	    return rconf->request_object;
    }
    return apache_request_new(r);
}

static request_data *get_request_data(VALUE obj)
{
    request_data *data;

    Check_Type(obj, T_DATA);
    data = (request_data *) RDATA(obj)->data;
    if (data == NULL)
	rb_raise(rb_eArgError, "destroyed request");
    return data;
}

request_rec *rb_get_request_rec(VALUE obj)
{
    return get_request_data(obj)->request;
}

long rb_apache_request_length(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    return RSTRING(data->outbuf)->len;
}

static VALUE request_output_buffer(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    return data->outbuf;
}

static VALUE request_replace(int argc, VALUE *argv, VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    return rb_funcall2(data->outbuf, rb_intern("replace"), argc, argv);
}

static VALUE request_cancel(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    RSTRING(data->outbuf)->len = 0;
    RSTRING(data->outbuf)->ptr[0] = '\0';
    return Qnil;
}

static VALUE request_get_sync_header(VALUE self)
{
    return FL_TEST(self, REQ_SYNC_HEADER) ? Qtrue : Qfalse;
}

static VALUE request_set_sync_header(VALUE self, VALUE val)
{
    if (RTEST(val)) {
	FL_SET(self, REQ_SYNC_HEADER);
    }
    else {
	FL_UNSET(self, REQ_SYNC_HEADER);
    }
    return val;
}

static VALUE request_get_sync_output(VALUE self)
{
    return FL_TEST(self, REQ_SYNC_OUTPUT) ? Qtrue : Qfalse;
}

static VALUE request_set_sync_output(VALUE self, VALUE val)
{
    if (RTEST(val)) {
	FL_SET(self, REQ_SYNC_OUTPUT);
    }
    else {
	FL_UNSET(self, REQ_SYNC_OUTPUT);
    }
    return val;
}

static VALUE request_set_sync(VALUE self, VALUE val)
{
    if (RTEST(val)) {
	FL_SET(self, REQ_SYNC_HEADER);
	FL_SET(self, REQ_SYNC_OUTPUT);
    }
    else {
	FL_UNSET(self, REQ_SYNC_HEADER);
	FL_UNSET(self, REQ_SYNC_OUTPUT);
    }
    return val;
}

static VALUE request_write(VALUE self, VALUE str)
{
    request_data *data;
    int len;

    data = get_request_data(self);
    str = rb_obj_as_string(str);
    if (FL_TEST(self, REQ_SYNC_OUTPUT)) {
	if (data->request->header_only &&
	    FL_TEST(self, REQ_SENT_HEADER))
	    return INT2NUM(0);
	len = ap_rwrite(RSTRING(str)->ptr, RSTRING(str)->len, data->request);
	ap_rflush(data->request);
    }
    else {
	rb_str_cat(data->outbuf, RSTRING(str)->ptr, RSTRING(str)->len);
	len = RSTRING(str)->len;
    }
    return INT2NUM(len);
}

static VALUE request_putc(VALUE self, VALUE c)
{
    char ch = NUM2CHR(c);
    request_data *data;

    data = get_request_data(self);
    if (FL_TEST(self, REQ_SYNC_OUTPUT)) {
	int ret;

	if (data->request->header_only &&
	    FL_TEST(self, REQ_SENT_HEADER))
	    return INT2NUM(EOF);
	ret = ap_rputc(NUM2INT(c), data->request);
	return INT2NUM(ret);
    }
    else {
	rb_str_cat(data->outbuf, &ch, 1);
	return c;
    }
}

static VALUE request_print(int argc, VALUE *argv, VALUE out)
{
    int i;
    VALUE line;

    /* if no argument given, print `$_' */
    if (argc == 0) {
	argc = 1;
	line = rb_lastline_get();
	argv = &line;
    }
    for (i=0; i<argc; i++) {
	if (!NIL_P(rb_output_fs) && i>0) {
	    request_write(out, rb_output_fs);
	}
	switch (TYPE(argv[i])) {
	  case T_NIL:
	    request_write(out, STRING_LITERAL("nil"));
	    break;
	  default:
	    request_write(out, argv[i]);
	    break;
	}
    }
    if (!NIL_P(rb_output_rs)) {
	request_write(out, rb_output_rs);
    }

    return Qnil;
}

static VALUE request_printf(int argc, VALUE *argv, VALUE out)
{
    request_write(out, rb_f_sprintf(argc, argv));
    return Qnil;
}

static VALUE request_puts _((int, VALUE*, VALUE));

#if RUBY_VERSION_CODE >= 190 && RUBY_RELEASE_CODE > 20050304
static VALUE request_puts_ary(VALUE ary, VALUE out, int recur)
#else
static VALUE request_puts_ary(VALUE ary, VALUE out)
#endif
{
    VALUE tmp;
    int i;

    for (i=0; i<RARRAY(ary)->len; i++) {
	tmp = RARRAY(ary)->ptr[i];
#if RUBY_VERSION_CODE >= 190 && RUBY_RELEASE_CODE > 20050304
	if (recur) {
#else
	if (rb_inspecting_p(tmp)) {
#endif
	    tmp = STRING_LITERAL("[...]");
	}
	request_puts(1, &tmp, out);
    }
    return Qnil;
}

static VALUE request_puts(int argc, VALUE *argv, VALUE out)
{
    int i;
    VALUE line;

    /* if no argument given, print newline. */
    if (argc == 0) {
	request_write(out, rb_default_rs);
	return Qnil;
    }
    for (i=0; i<argc; i++) {
	switch (TYPE(argv[i])) {
	  case T_NIL:
	    line = STRING_LITERAL("nil");
	    break;
	  case T_ARRAY:
#if RUBY_VERSION_CODE >= 190 && RUBY_RELEASE_CODE > 20050304
	    rb_exec_recursive(request_puts_ary, argv[i], out);
#else
	    rb_protect_inspect(request_puts_ary, argv[i], out);
#endif
	    continue;
	  default:
	    line = argv[i];
	    break;
	}
	line = rb_obj_as_string(line);
	request_write(out, line);
	if (RSTRING(line)->ptr[RSTRING(line)->len-1] != '\n') {
	    request_write(out, rb_default_rs);
	}
    }

    return Qnil;
}

static VALUE request_addstr(VALUE out, VALUE str)
{
    request_write(out, str);
    return out;
}

VALUE rb_apache_request_send_http_header(VALUE self)
{
    request_data *data;

    if (FL_TEST(self, REQ_SYNC_HEADER)) {
	data = get_request_data(self);
	ap_send_http_header(data->request);
	FL_SET(self, REQ_SENT_HEADER);
	FL_UNSET(self, REQ_HEADER_PENDING);
    }
    else {
	FL_SET(self, REQ_HEADER_PENDING);
    }
    return Qnil;
}

static VALUE request_sent_http_header(VALUE self)
{
    if (FL_TEST(self, REQ_SENT_HEADER) ||
	FL_TEST(self, REQ_HEADER_PENDING)) {
	return Qtrue;
    }
    else {
	return Qfalse;
    }
}

void rb_apache_request_flush(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    if (FL_TEST(self, REQ_HEADER_PENDING)) {
	ap_send_http_header(data->request);
	FL_SET(self, REQ_SENT_HEADER);
	FL_UNSET(self, REQ_HEADER_PENDING);
    }
    if (data->request->header_only &&
	FL_TEST(self, REQ_SENT_HEADER)) {
	RSTRING(data->outbuf)->len = 0;
	return;
    }
    if (RSTRING(data->outbuf)->len > 0) {
	ap_rwrite(RSTRING(data->outbuf)->ptr, RSTRING(data->outbuf)->len,
		  data->request);
	ap_rflush(data->request);
	RSTRING(data->outbuf)->len = 0;
    }
}


static VALUE request_connection(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    if (NIL_P(data->connection)) {
	data->connection = rb_apache_connection_new(data->request->connection);
    }
    return data->connection;
}

static VALUE request_server(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    if (NIL_P(data->server)) {
	data->server = rb_apache_server_new(data->request->server);
    }
    return data->server;
}

static VALUE request_next(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    return rb_get_request_object(data->request->next);
}

static VALUE request_prev(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    return rb_get_request_object(data->request->prev);
}

static VALUE request_last(VALUE self)
{
    request_data *data;
    request_rec *last;

    data = get_request_data(self);

    for (last = data->request; last->next; last = last->next)
        continue;

    return rb_get_request_object(last);
}

static VALUE request_main(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    return rb_get_request_object(data->request->main);
}

REQUEST_STRING_ATTR_READER(request_protocol, protocol);
REQUEST_STRING_ATTR_READER(request_hostname, hostname);
REQUEST_STRING_ATTR_READER(request_unparsed_uri, unparsed_uri);
REQUEST_STRING_ATTR_READER(request_get_uri, uri);
REQUEST_STRING_ATTR_WRITER(request_set_uri, uri);
REQUEST_STRING_ATTR_READER(request_get_filename, filename);
REQUEST_STRING_ATTR_WRITER(request_set_filename, filename);
REQUEST_STRING_ATTR_READER(request_get_path_info, path_info);
REQUEST_STRING_ATTR_WRITER(request_set_path_info, path_info);
REQUEST_INT_ATTR_READER(request_get_status, status);
REQUEST_INT_ATTR_WRITER(request_set_status, status);
REQUEST_STRING_ATTR_READER(request_get_status_line, status_line);
REQUEST_STRING_ATTR_WRITER(request_set_status_line, status_line);
REQUEST_STRING_ATTR_READER(request_the_request, the_request);
REQUEST_BOOL_ATTR_READER(request_get_assbackwards, assbackwards);
REQUEST_BOOL_ATTR_WRITER(request_set_assbackwards, assbackwards);
REQUEST_STRING_ATTR_READER(request_request_method, method);
REQUEST_INT_ATTR_READER(request_method_number, method_number);
REQUEST_INT_ATTR_READER(request_get_allowed, allowed);
REQUEST_INT_ATTR_WRITER(request_set_allowed, allowed);
REQUEST_STRING_ATTR_READER(request_get_args, args);
REQUEST_STRING_ATTR_WRITER(request_set_args, args);
REQUEST_STRING_ATTR_READER(request_get_content_type, content_type);
REQUEST_STRING_ATTR_READER(request_get_content_encoding, content_encoding);
REQUEST_STRING_ATTR_READER(request_get_dispatch_handler, handler);
REQUEST_STRING_ATTR_WRITER(request_set_dispatch_handler, handler);

static VALUE request_request_time(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    return rb_time_new(apr_time_sec(data->request->request_time), 0);
}

static VALUE request_header_only(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    return data->request->header_only ? Qtrue : Qfalse;
}

static VALUE request_content_length(VALUE self)
{
    request_data *data;
    const char *s;

    rb_warn("content_length is obsolete; use headers_in[\"Content-Length\"] instead");
    data = get_request_data(self);
    s = apr_table_get(data->request->headers_in, "Content-Length");
    return s ? rb_cstr2inum(s, 10) : Qnil;
}

static VALUE request_set_content_type(VALUE self, VALUE str)
{
    request_data *data;

    data = get_request_data(self);
    if (NIL_P(str)) {
	data->request->content_type = NULL;
    }
    else {
	Check_Type(str, T_STRING);
	data->request->content_type =
	    apr_pstrndup(data->request->pool,
			RSTRING(str)->ptr, RSTRING(str)->len);
    }
    return str;
}

static VALUE request_set_content_encoding(VALUE self, VALUE str)
{
    request_data *data;

    data = get_request_data(self);
    if (NIL_P(str)) {
	data->request->content_encoding = NULL;
    }
    else {
	Check_Type(str, T_STRING);
	data->request->content_encoding =
	    apr_pstrndup(data->request->pool,
			RSTRING(str)->ptr,
			RSTRING(str)->len);
    }
    return str;
}
 
static VALUE request_get_content_languages(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    if (data->request->content_languages) {
	return rb_apache_array_new(data->request->content_languages);
    }
    else {
	return Qnil;
    }
}

static VALUE request_set_content_languages(VALUE self, VALUE ary)
{
    request_data *data;
    int i;

    data = get_request_data(self);
    if (NIL_P(ary)) {
	data->request->content_languages = NULL;
    }
    else {
	Check_Type(ary, T_ARRAY);
	for (i = 0; i < RARRAY(ary)->len; i++) {
	    Check_Type(RARRAY(ary)->ptr[i], T_STRING);
	}
	data->request->content_languages =
	    apr_array_make(data->request->pool, RARRAY(ary)->len, sizeof(char *));
	for (i = 0; i < RARRAY(ary)->len; i++) {
	    VALUE str = RARRAY(ary)->ptr[i];
	    *(char **) apr_array_push(data->request->content_languages) =
		apr_pstrndup(data->request->pool,
			    RSTRING(str)->ptr,
			    RSTRING(str)->len);
	}
    }
    return ary;
}

static VALUE request_headers_in(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    if (NIL_P(data->headers_in)) {
	data->headers_in = rb_apache_table_new(data->request->headers_in);
    }
    return data->headers_in;
}

static VALUE request_headers_out(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    if (NIL_P(data->headers_out)) {
	data->headers_out = rb_apache_table_new(data->request->headers_out);
    }
    return data->headers_out;
}

static VALUE request_err_headers_out(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    if (NIL_P(data->err_headers_out)) {
	data->err_headers_out =
	    rb_apache_table_new(data->request->err_headers_out);
    }
    return data->err_headers_out;
}

static VALUE request_subprocess_env(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    if (NIL_P(data->subprocess_env)) {
	data->subprocess_env =
	    rb_apache_table_new(data->request->subprocess_env);
    }
    return data->subprocess_env;
}

static VALUE request_notes(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    if (NIL_P(data->notes)) {
	data->notes =
	    rb_apache_table_new(data->request->notes);
    }
    return data->notes;
}

#ifdef APACHE2

#ifdef WIN32
typedef int mode_t;
#endif

static mode_t get_mode(apr_finfo_t *finfo)
{
    mode_t mode = 0;

    if (finfo->protection & APR_UREAD)
        mode |= S_IRUSR;
    if (finfo->protection & APR_UWRITE)
        mode |= S_IWUSR;
    if (finfo->protection & APR_UEXECUTE)
        mode |= S_IXUSR;

    if (finfo->protection & APR_GREAD)
        mode |= S_IRGRP;
    if (finfo->protection & APR_GWRITE)
        mode |= S_IWGRP;
    if (finfo->protection & APR_GEXECUTE)
        mode |= S_IXGRP;

    if (finfo->protection & APR_WREAD)
        mode |= S_IROTH;
    if (finfo->protection & APR_WWRITE)
        mode |= S_IWOTH;
    if (finfo->protection & APR_WEXECUTE)
        mode |= S_IXOTH;

    switch (finfo->filetype) {
    case APR_REG:
	mode |= S_IFREG;
	break;
    case APR_DIR:
	mode |= S_IFDIR;
	break;
    case APR_CHR:
	mode |= S_IFCHR;
	break;
    case APR_BLK:
#ifndef WIN32
	mode |= S_IFBLK;
#endif
	break;
    case APR_PIPE:
#ifndef WIN32    
	mode |= S_IFIFO;
#endif	
	break;
    case APR_LNK:
#ifndef WIN32   
	mode |= S_IFLNK;
#endif	
	break;
    case APR_SOCK:
#if !defined(BEOS) && defined(S_ISSOCK)
	mode |= S_IFSOCK;
#endif
	break;
    default:
	break;
    }

    return mode;
}
#endif /* APACHE2 */

static VALUE request_finfo(VALUE self)
{
    VALUE cStat;
    request_data *data;
    struct stat *st;

    data = get_request_data(self);
    if (NIL_P(data->finfo)) {
	cStat = rb_const_get(rb_cFile, rb_intern("Stat"));
	data->finfo = Data_Make_Struct(cStat, struct stat, NULL, free, st);
#ifdef APACHE2
	memset(st, 0, sizeof(struct stat));
	if (data->request->finfo.filetype != 0) {
	    st->st_dev = data->request->finfo.device;
	    st->st_ino = data->request->finfo.inode;
	    st->st_nlink = data->request->finfo.nlink;
	    st->st_mode = get_mode(&data->request->finfo);
	    st->st_uid = data->request->finfo.user;
	    st->st_gid = data->request->finfo.group;
	    st->st_size = data->request->finfo.size;
	    st->st_atime = apr_time_sec(data->request->finfo.atime);
	    st->st_mtime = apr_time_sec(data->request->finfo.mtime);
	    st->st_ctime = apr_time_sec(data->request->finfo.ctime);
	}
#else /* Apache 1.x */
	*st = data->request->finfo;
#endif
    }
    return data->finfo;
}

static VALUE request_parsed_uri(VALUE self)
{
    request_data *data;
    
    data = get_request_data(self);
    if (NIL_P(data->parsed_uri)) {
      data->parsed_uri = rb_apache_uri_new(&data->request->parsed_uri);
    }
    return data->parsed_uri;
}

static VALUE request_attributes(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    if (NIL_P(data->attributes))
	data->attributes = rb_hash_new();
    return data->attributes;
}

static VALUE request_setup_client_block(int argc, VALUE *argv, VALUE self)
{
    request_data *data;
    VALUE policy;
    int read_policy = REQUEST_CHUNKED_ERROR;

    data = get_request_data(self);
    if (rb_scan_args(argc, argv, "01", &policy) == 1) {
	read_policy = NUM2INT(policy);
    }
    return INT2NUM(ap_setup_client_block(data->request, read_policy));
}

static VALUE request_should_client_block(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    return INT2BOOL(ap_should_client_block(data->request));
}

static VALUE request_get_client_block(VALUE self, VALUE length)
{
    request_data *data;
    int len;
    VALUE result;

    data = get_request_data(self);
    len = NUM2INT(length);
#if RUBY_VERSION_CODE < 180
    {
	char *buf = (char *) ap_palloc(data->request->pool, len);
	len = ap_get_client_block(data->request, buf, len);
	result = rb_tainted_str_new(buf, len);
	return result;
    }
#else
    result = rb_str_buf_new(len);
    len = ap_get_client_block(data->request, RSTRING(result)->ptr, len);
    switch (len) {
    case -1:
	rb_raise(rb_eApachePrematureChunkEndError, "premature chunk end");
	break;
    case 0:
	return Qnil;
    default:
	RSTRING(result)->ptr[len] = '\0';
	RSTRING(result)->len = len;
	OBJ_TAINT(result);
	return result;
    }
#endif
}

static VALUE read_client_block(request_rec *r, int len)
{
    char *buf;
    int rc;
    int nrd = 0;
    int old_read_length;
    VALUE result;

    if (r->read_length == 0) {
        if ((rc = ap_setup_client_block(r, REQUEST_CHUNKED_ERROR)) != OK) {
	    rb_apache_exit(rc);
        }
    }
    old_read_length = r->read_length;
    r->read_length = 0;
    if (ap_should_client_block(r)) {
	if (len < 0)
	    len = r->remaining;
	buf = (char *) apr_palloc(r->pool, len);
	result = rb_tainted_str_new("", 0);
	while (len > 0) {
	    nrd = ap_get_client_block(r, buf, len);
	    if (nrd == 0) {
		break;
	    }
	    if (nrd == -1) {
		r->read_length += old_read_length;
		rb_raise(rb_eApachePrematureChunkEndError, "premature chunk end");
	    }
	    rb_str_cat(result, buf, nrd);
	    len -= nrd;
	}
    }
    else {
	result = Qnil;
    }
    r->read_length += old_read_length;
    return result;
}

static VALUE request_read(int argc, VALUE *argv, VALUE self)
{
    request_data *data;
    VALUE length;
    int len;

    data = get_request_data(self);
    rb_scan_args(argc, argv, "01", &length);
    if (NIL_P(length)) {
	return read_client_block(data->request, -1);
    }
    len = NUM2INT(length);
    if (len < 0) {
	rb_raise(rb_eArgError, "negative length %d given", len);
    }
    return read_client_block(data->request, len);
}

static VALUE request_getc(VALUE self)
{
    request_data *data;
    VALUE str;

    data = get_request_data(self);
    str = read_client_block(data->request, 1);
    if (NIL_P(str) || RSTRING(str)->len == 0)
	return Qnil;
    return INT2FIX(RSTRING(str)->ptr[0]);
}

static VALUE request_eof(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    return INT2BOOL(data->request->remaining == 0);
}

static VALUE request_binmode(VALUE self)
{
    return Qnil;
}

static VALUE request_allow_options(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    return INT2NUM(ap_allow_options(data->request));
}

static VALUE request_allow_overrides(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    return INT2NUM(ap_allow_overrides(data->request));
}

static VALUE request_default_type(VALUE self)
{
    request_data *data;
    const char *type;

    data = get_request_data(self);
    type = ap_default_type(data->request);
    return type ? rb_tainted_str_new2(type) : Qnil;
}

static VALUE request_default_port(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    return INT2NUM(ap_default_port(data->request));
}

static VALUE request_remote_host(int argc, VALUE *argv, VALUE self)
{
    request_data *data;
    const char *host;
    VALUE lookup_type;
    int lookup_val = REMOTE_HOST;

    if (argc == 1) {
      rb_scan_args(argc, argv, "01", &lookup_type);
      switch (NUM2INT(lookup_type)) {
      case REMOTE_HOST:
	lookup_val = REMOTE_HOST;
	break;
      case REMOTE_NAME:
	lookup_val = REMOTE_NAME;
	break;
      case REMOTE_NOLOOKUP:
	lookup_val = REMOTE_NOLOOKUP;
	break;
      case REMOTE_DOUBLE_REV:
	lookup_val = REMOTE_DOUBLE_REV;
	break;
      default:
	lookup_val = REMOTE_HOST;
      }
    }

    data = get_request_data(self);
#ifdef APACHE2
    host = ap_get_remote_host(data->request->connection,
			      data->request->per_dir_config,
			      lookup_val, NULL);
#else /* Apache 1.x */
    host = ap_get_remote_host(data->request->connection,
			      data->request->per_dir_config,
			      lookup_val);
#endif
    return host ? rb_tainted_str_new2(host) : Qnil;
}

static VALUE request_remote_logname(VALUE self)
{
    request_data *data;
    const char *logname;

    data = get_request_data(self);
    logname = ap_get_remote_logname(data->request);
    return logname ? rb_tainted_str_new2(logname) : Qnil;
}

static VALUE request_construct_url(VALUE self, VALUE uri)
{
    request_data *data;
    char *url;

    data = get_request_data(self);
    url = ap_construct_url(data->request->pool, StringValuePtr(uri), data->request);
    return rb_tainted_str_new2(url);
}

static VALUE request_server_name(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    return rb_tainted_str_new2(ap_get_server_name(data->request));
}

static VALUE request_server_port(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    return INT2NUM(ap_get_server_port(data->request));
}

static VALUE request_satisfies(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    return INT2NUM(ap_satisfies(data->request));
}

static VALUE request_requires(VALUE self)
{
    VALUE result, req;
    request_data *data;
    const array_header *reqs_arr;
    require_line *reqs;
    int i;

    data = get_request_data(self);
    reqs_arr = ap_requires(data->request);
    if (reqs_arr == NULL)
	return Qnil;
    reqs = (require_line *) reqs_arr->elts;
    result = rb_ary_new2(reqs_arr->nelts);
    for (i = 0; i < reqs_arr->nelts; i++) {
	req = rb_assoc_new(INT2NUM(reqs[i].method_mask),
			   rb_tainted_str_new2(reqs[i].requirement));
	rb_ary_push(result, req);
    }
    return result;
}

static VALUE request_escape_html(VALUE self, VALUE str)
{
    request_data *data;
    char *tmp;
    VALUE result;

    data = get_request_data(self);
    tmp = ap_escape_html(data->request->pool, StringValuePtr(str));
    result = rb_str_new2(tmp);
    OBJ_INFECT(result, str);
    return result;
}

static VALUE request_signature(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    return rb_tainted_str_new2(ap_psignature("", data->request));
}

static VALUE request_reset_timeout(VALUE self)
{
#ifdef APACHE2
    rb_notimplement();
#else /* Apache 1.x */
    request_data *data;

    data = get_request_data(self);
    ap_reset_timeout(data->request);
#endif
    return Qnil;
}

static VALUE request_hard_timeout(VALUE self, VALUE name)
{
#ifdef APACHE2
    rb_notimplement();
#else /* Apache 1.x */
    request_data *data;
    char *s;

    Check_Type(name, T_STRING);
    data = get_request_data(self);
    s = ap_pstrndup(data->request->pool, RSTRING(name)->ptr, RSTRING(name)->len);
    ap_hard_timeout(s, data->request);
#endif
    return Qnil;
}

static VALUE request_soft_timeout(VALUE self, VALUE name)
{
    request_data *data;
    char *s;

    Check_Type(name, T_STRING);
    data = get_request_data(self);
    s = apr_pstrndup(data->request->pool, RSTRING(name)->ptr, RSTRING(name)->len);
    ap_soft_timeout(s, data->request);
    return Qnil;
}

static VALUE request_kill_timeout(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    ap_kill_timeout(data->request);
    return Qnil;
}

static VALUE request_note_auth_failure(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    ap_note_auth_failure(data->request);
    return Qnil;
}

static VALUE request_note_basic_auth_failure(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    ap_note_basic_auth_failure(data->request);
    return Qnil;
}

static VALUE request_note_digest_auth_failure(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    ap_note_digest_auth_failure(data->request);
    return Qnil;
}

static VALUE request_get_basic_auth_pw(VALUE self)
{
    request_data *data;
    const char *pw;
    int res;

    data = get_request_data(self);
    if ((res = ap_get_basic_auth_pw(data->request, &pw)) != OK) {
	rb_apache_exit(res);
    }
    return rb_tainted_str_new2(pw);
}

static VALUE request_add_common_vars(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    ap_add_common_vars(data->request);
    return Qnil;
}

static VALUE request_add_cgi_vars(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    ap_add_cgi_vars(data->request);
    return Qnil;
}

static VALUE request_setup_cgi_env(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    rb_setup_cgi_env(data->request);
    return Qnil;
}

static VALUE request_log_reason(VALUE self, VALUE msg, VALUE file)
{
    request_data *data;
    const char *host;

    Check_Type(msg, T_STRING);
    Check_Type(file, T_STRING);
    data = get_request_data(self);
#ifdef APACHE2
    host = ap_get_remote_host(data->request->connection,
			      data->request->per_dir_config,
			      REMOTE_HOST, NULL);
    ap_log_error(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO,
		 0, data->request->server,
		 "access to %s failed for %s, reason: %s",
		 StringValuePtr(file),
		 host,
		 StringValuePtr(msg));
#else /* Apache 1.x */
    host = ap_get_remote_host(data->request->connection,
			      data->request->per_dir_config,
			      REMOTE_HOST);
    ap_log_error(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO,
		 data->request->server,
		 "access to %s failed for %s, reason: %s",
		 StringValuePtr(file),
		 host,
		 StringValuePtr(msg));
#endif
    return Qnil;
}

void rb_apache_request_set_error(VALUE request, VALUE errmsg, VALUE exception)
{
    request_data *data;

    Data_Get_Struct(request, request_data, data);
    data->error_message = errmsg;
    data->exception = exception;
}

static VALUE request_error_message(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    return data->error_message;
}

static VALUE request_exception(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    return data->exception;
}

#ifdef APACHE2
REQUEST_STRING_ATTR_READER(request_user, user);

static VALUE request_set_user(VALUE self, VALUE val)
{
    request_data *data;

    Check_Type(val, T_STRING);
    data = get_request_data(self);
    data->request->user = apr_pstrndup(data->request->pool,
				      RSTRING(val)->ptr,
				      RSTRING(val)->len);
    return val;
}
#else /* Apache 1.x */
static VALUE request_user(VALUE self)
{
    VALUE conn;

    conn = request_connection(self);
    return rb_funcall(conn, rb_intern("user"), 0);
}

static VALUE request_set_user(VALUE self, VALUE val)
{
    VALUE conn;

    conn = request_connection(self);
    return rb_funcall(conn, rb_intern("user="), 1, val);
}
#endif

/* Should only be called from inside of response handlers */
static VALUE request_internal_redirect(VALUE self, VALUE uri)
{
    request_data *data;

    Check_Type(uri, T_STRING);
    data = get_request_data(self);
    ap_internal_redirect(StringValuePtr(uri), data->request);
    return Qnil;
}

static VALUE request_custom_response(VALUE self, VALUE status, VALUE string)
{
    request_data *data;

    Check_Type(status, T_FIXNUM);
    Check_Type(string, T_STRING);
    data = get_request_data(self);
    ap_custom_response(data->request, NUM2INT(status), StringValuePtr(string));
    return Qnil;
}

static VALUE request_is_initial(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    if (ap_is_initial_req(data->request)) {
        return Qtrue;
    } else {
        return Qfalse;
    }
}

static VALUE request_is_main(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    if (data->request->main) {
        return Qfalse;
    } else {
        return Qtrue;
    }
}

static VALUE request_auth_type(VALUE self)
{
    request_data *data;
    core_dir_config *conf;

    data = get_request_data(self);
    conf = (core_dir_config *)
	ap_get_module_config(data->request->per_dir_config, &core_module);
    if (conf->ap_auth_type) {
	return rb_tainted_str_new2(conf->ap_auth_type);
    }
    else {
	return Qnil;
    }
}

static VALUE request_set_auth_type(VALUE self, VALUE val)
{
    request_data *data;
    core_dir_config *conf;

    Check_Type(val, T_STRING);
    data = get_request_data(self);
    conf = (core_dir_config *)
	ap_get_module_config(data->request->per_dir_config, &core_module);
    conf->ap_auth_type = apr_pstrndup(data->request->pool,
				     RSTRING(val)->ptr,
				     RSTRING(val)->len);
    ap_set_module_config(data->request->per_dir_config, &core_module, conf);
    return val;
}

static VALUE request_auth_name(VALUE self)
{
    request_data *data;
    core_dir_config *conf;

    data = get_request_data(self);
    conf = (core_dir_config *)
	ap_get_module_config(data->request->per_dir_config, &core_module);
    if (conf->ap_auth_name) {
	return rb_tainted_str_new2(conf->ap_auth_name);
    }
    else {
	return Qnil;
    }
}

static VALUE request_set_auth_name(VALUE self, VALUE val)
{
    request_data *data;
    core_dir_config *conf;

    Check_Type(val, T_STRING);
    data = get_request_data(self);
    conf = (core_dir_config *)
	ap_get_module_config(data->request->per_dir_config, &core_module);
    conf->ap_auth_name = apr_pstrndup(data->request->pool,
				     RSTRING(val)->ptr,
				     RSTRING(val)->len);
    ap_set_module_config(data->request->per_dir_config, &core_module, conf);
    return val;
}

static VALUE request_default_charset(VALUE self)
{
    request_data *data = get_request_data(self);
    core_dir_config *conf =
	ap_get_module_config(data->request->per_dir_config, &core_module);
    const char *charset_name =
	conf->add_default_charset_name;
    return charset_name ? rb_tainted_str_new2(charset_name) : Qnil;
}

static VALUE request_bytes_sent(VALUE self)
{
    request_data *data;
    request_rec *last;

    data = get_request_data(self);

    for (last = data->request; last->next; last = data->request->next)
        continue;

#ifndef APACHE2
    if (last->sent_bodyct && !last->bytes_sent)
        ap_bgetopt(last->connection->client, BO_BYTECT, &last->bytes_sent);
#endif

    return INT2NUM(last->bytes_sent);
}

static VALUE request_send_fd(VALUE self, VALUE io)
{
    OpenFile *fptr;
    request_data *data;
#ifdef APACHE2
    apr_size_t bytes_sent;
    apr_file_t *file;
    int fd;
    struct stat st;
#else
    FILE *f;
    long bytes_sent;
#endif

    request_set_sync(self, Qtrue);
    rb_apache_request_flush(self);    
    data = get_request_data(self);

    Check_Type(io, T_FILE);
    GetOpenFile(io, fptr);

#ifdef APACHE2
#if RUBY_VERSION_CODE >= 190
    fd = fptr->fd;
#else
    fd = fileno(fptr->f);
#endif
    if (apr_os_file_put(&file, &fd, 0, data->request->pool) != APR_SUCCESS) {
	rb_raise(rb_eIOError, "apr_os_file_put() failed");
    }
    if (fstat(fd, &st) == -1) {
	rb_sys_fail(fptr->path);
    }
    ap_send_fd(file, data->request, 0, st.st_size, &bytes_sent);
#else
#if RUBY_VERSION_CODE >= 190
    f = rb_io_stdio_file(fptr);
#else
    f = fptr->f;
#endif
    bytes_sent = ap_send_fd_length(f, data->request, -1);
#endif

    return INT2NUM(bytes_sent);
}

static VALUE request_proxy_q(VALUE self)
{
    request_data *data = get_request_data(self);

    switch (data->request->proxyreq) {
#ifdef APACHE2
    case PROXYREQ_NONE:
	return Qfalse;
    case PROXYREQ_PROXY:
    case PROXYREQ_REVERSE:
	return Qtrue;
#else
    case NOT_PROXY:
	return Qfalse;
    case STD_PROXY:
    case PROXY_PASS:
	return Qtrue;
#endif
    default:
      rb_raise(rb_eArgError, "Unknown Proxy Type");
    }
}

static VALUE request_proxy_pass_q(VALUE self)
{
    request_data *data = get_request_data(self);

#ifdef APACHE2
    if (data->request->proxyreq == PROXYREQ_REVERSE)
#else
    if (data->request->proxyreq == PROXY_PASS)
#endif
      return Qtrue;
    else
      return Qfalse;
}

static VALUE request_get_cache_resp(VALUE self)
{
    request_data *data;
    table *tbl;
    table_entry *hdrs;
    const array_header *hdrs_arr;
    int i;

    data = get_request_data(self);

    if (NIL_P(data->headers_out))
        data->headers_out = rb_apache_table_new(data->request->headers_out);

    Data_Get_Struct(data->headers_out, table, tbl);
    hdrs_arr = apr_table_elts(tbl);
    hdrs = (table_entry *) hdrs_arr->elts;
    for (i = 0; i < hdrs_arr->nelts; i++) {
        if (hdrs[i].key == NULL)
	    continue;
	if (strcasecmp(hdrs[i].key, "Pragma") ||
	    strcasecmp(hdrs[i].key, "Cache-control")) {
	    return Qtrue;
	}
    }
    return Qfalse;
}

static VALUE request_set_cache_resp(VALUE self, VALUE arg)
{
    request_data *data;
    table *tbl;

    data = get_request_data(self);

    if (NIL_P(data->headers_out))
        data->headers_out = rb_apache_table_new(data->request->headers_out);

    Data_Get_Struct(data->headers_out, table, tbl);
    if (arg == Qtrue) {
        apr_table_setn(tbl, "Pragma", "no-cache");
	apr_table_setn(tbl, "Cache-control", "no-cache");
	return Qtrue;
    } else {
        apr_table_unset(tbl, "Pragma");
	apr_table_unset(tbl, "Cache-control");
	return Qfalse;
    }
}

static VALUE request_lookup_uri(VALUE self, VALUE uri)
{
    request_data *data;
    request_rec *new_rec;

    Check_Type(uri, T_STRING);
    data = get_request_data(self);
#ifdef APACHE2
    new_rec = ap_sub_req_lookup_uri(StringValuePtr(uri), data->request, NULL);
#else
    new_rec = ap_sub_req_lookup_uri(StringValuePtr(uri), data->request);
#endif
    return apache_request_new(new_rec);
}

static VALUE request_lookup_file(VALUE self, VALUE file)
{
    request_data *data;
    request_rec *new_rec;

    Check_Type(file, T_STRING);
    data = get_request_data(self);
#ifdef APACHE2
    new_rec = ap_sub_req_lookup_file(StringValuePtr(file), data->request, NULL);
#else
    new_rec = ap_sub_req_lookup_file(StringValuePtr(file), data->request);
#endif
    return apache_request_new(new_rec);
}

typedef struct request_cleanup {
    pool *pool;
    VALUE plain_cleanup;
    VALUE child_cleanup;
} request_cleanup_t;

static VALUE call_proc_0(VALUE proc)
{
    return rb_funcall(proc, rb_intern("call"), 0);
}

static void call_proc(pool *p, VALUE proc)
{
#if APR_HAS_THREADS
    if (ruby_is_threaded_mpm) {
	ruby_call_interpreter(p, (ruby_interp_func_t) call_proc_0,
			      (void *) proc, NULL, 0);
    }
    else {
#endif
	call_proc_0(proc);
#if APR_HAS_THREADS
    }
#endif
}

static APR_CLEANUP_RETURN_TYPE call_plain_cleanup(void *data)
{
    request_cleanup_t *cleanup = (request_cleanup_t *) data;

    call_proc(cleanup->pool, cleanup->plain_cleanup);
    APR_CLEANUP_RETURN_SUCCESS();
}

static APR_CLEANUP_RETURN_TYPE call_child_cleanup(void *data)
{
    request_cleanup_t *cleanup = (request_cleanup_t *) data;

    call_proc(cleanup->pool, cleanup->child_cleanup);
    APR_CLEANUP_RETURN_SUCCESS();
}

static VALUE request_register_cleanup(int argc, VALUE *argv, VALUE self)
{
    request_data *data;
    VALUE plain_cleanup, child_cleanup;
    request_cleanup_t *cleanup;
    APR_CLEANUP_RETURN_TYPE (*plain_cleanup_func) (void *);
    APR_CLEANUP_RETURN_TYPE (*child_cleanup_func) (void *);

    data = get_request_data(self);
    rb_scan_args(argc, argv, "02", &plain_cleanup, &child_cleanup);
    if (argc == 0)
	plain_cleanup = rb_f_lambda();
    if (NIL_P(plain_cleanup))
	plain_cleanup_func = apr_pool_cleanup_null;
    else
	plain_cleanup_func = call_plain_cleanup;
    if (NIL_P(child_cleanup))
	child_cleanup_func = apr_pool_cleanup_null;
    else
	child_cleanup_func = call_child_cleanup;
    cleanup = apr_palloc(data->request->pool, sizeof(request_cleanup_t));
    cleanup->pool = data->request->pool;
    cleanup->plain_cleanup = plain_cleanup;
    cleanup->child_cleanup = child_cleanup;
    apr_pool_cleanup_register(data->request->pool, (void *) cleanup,
			plain_cleanup_func, child_cleanup_func);
    
    return Qnil;
}

static VALUE request_options(VALUE self)
{
    request_data *data;

    data = get_request_data(self);
    return data->options;
}

static VALUE request_libapreq_p( VALUE klass )
{
    return Qtrue;
}

static VALUE request_uploads( VALUE self )
{
    request_data *data = get_request_data( self );
    ApacheUpload *upload;
    VALUE upobj;

    if ( !data->apreq->parsed ) rb_funcall( self, rb_intern("parse"), 0 );

    if (!RHASH(data->upload_table)->tbl->num_entries) {
	for ( upload = ApacheRequest_upload(data->apreq);
	      upload;
	      upload = upload->next )
        {
            upobj = rb_apache_upload_new(upload);
            rb_hash_aset( data->upload_table,
                          rb_tainted_str_new2(upload->name),
                          upobj );
        }
    }

    return data->upload_table;
}

static VALUE request_post_max_eq( VALUE self, VALUE setting )
{
    request_data *data = get_request_data( self );
    data->apreq->post_max = NUM2INT( setting );
    return INT2FIX( data->apreq->post_max );
}

static VALUE request_post_max( VALUE self )
{
    request_data *data = get_request_data( self );
    return INT2FIX( data->apreq->post_max );
}

static VALUE request_disable_uploads_eq( VALUE self, VALUE setting )
{
    request_data *data = get_request_data( self );
    data->apreq->disable_uploads = RTEST( setting ) ? 1 : 0;
    return data->apreq->disable_uploads ? Qtrue : Qfalse;
}

static VALUE request_disable_uploads_p( VALUE self )
{
    request_data *data = get_request_data( self );
    return data->apreq->disable_uploads ? Qtrue : Qfalse;
}

static VALUE request_temp_dir_eq( VALUE self, VALUE setting )
{
    request_data *data = get_request_data( self );

    SafeStringValue( setting );
    data->apreq->temp_dir = StringValuePtr( setting );
    return setting;
}

static VALUE request_temp_dir( VALUE self )
{
    request_data *data = get_request_data( self );
    return rb_str_new2( data->apreq->temp_dir );
}

static int request_call_upload_hook( VALUE self, char *buffer,
				     int bufsize, ApacheUpload *upload )
{
    request_data *data = get_request_data(self);
    VALUE rbuf, name, upobj, rval;

    rbuf = rb_tainted_str_new(buffer, (long) bufsize);
    name = rb_tainted_str_new2(upload->name);

    upobj = rb_hash_aref( data->upload_table, name );
    if (NIL_P(upobj)) {
        upobj = rb_apache_upload_new(upload);
        rb_hash_aset(data->upload_table, name, upobj);
    }

    rval = rb_funcall(data->upload_hook,
		      rb_intern("call"),
		      3,
		      rbuf,
		      upobj,
		      data->upload_hook_arg);
    return bufsize;
}


static VALUE request_upload_hook_eq( VALUE self, VALUE setting )
{
    request_data *data = get_request_data( self );
    VALUE func = rb_check_convert_type( setting, T_DATA, "Proc", "to_proc" );

    if ( !rb_obj_is_instance_of(func, rb_cProc) ) {
	rb_raise( rb_eTypeError, "wrong argument type %s (expected Proc)",
		  rb_class2name(CLASS_OF(func)) );
    }

    data->upload_hook = func;
    data->apreq->upload_hook =
	(int(*)(void*,char*,int,ApacheUpload*)) request_call_upload_hook;
    data->apreq->hook_data = (void *) self;

    return func;
}

static VALUE request_upload_hook( VALUE self )
{
    request_data *data = get_request_data( self );
    return data->upload_hook;
}

static VALUE request_upload_hook_data_eq( VALUE self, VALUE setting )
{
    request_data *data = get_request_data( self );
    data->upload_hook_arg = setting;
    return setting;
}

static VALUE request_upload_hook_data( VALUE self )
{
    request_data *data = get_request_data( self );
    return data->upload_hook_arg;
}

static VALUE request_set_parse_option( VALUE pair, VALUE self )
{
    ID opt;
    VALUE optval;

    Check_Type( pair, T_ARRAY );
    if ( !RARRAY(pair)->len == 2 )
	rb_raise( rb_eArgError, "Expected an array of 2 elements, not %d",
		  RARRAY(pair)->len );

    opt = rb_to_id( *(RARRAY(pair)->ptr) );
    optval = *(RARRAY(pair)->ptr + 1);

    if ( opt == id_post_max ) {
	request_post_max_eq( self, optval );
    }
    else if ( opt == id_disable_uploads ) {
	request_post_max_eq( self, optval );
    }
    else if ( opt == id_temp_dir ) {
	request_temp_dir_eq( self, optval );
    }
    else if ( opt == id_hook_data ) {
	request_upload_hook_data_eq( self, optval );
    }
    else if ( opt == id_upload_hook ) {
	request_upload_hook_eq( self, optval );
    }
    else {
	rb_raise( rb_eArgError, "Unknown option %s",
		  rb_inspect(*( RARRAY(pair)->ptr )) );
    }

    return optval;
}


static VALUE request_parse( int argc, VALUE *argv, VALUE self )
{
    request_data *data = get_request_data( self );
    int status;
    VALUE options;

    if ( data->apreq->parsed ) return Qfalse;

    if( rb_scan_args(argc, argv, "01", &options) ) {
	Check_Type( options, T_HASH );
	rb_iterate( rb_each, options, request_set_parse_option, self );
    }

    if ( (status = ApacheRequest_parse( data->apreq )) != OK )
	rb_raise( rb_eApacheRequestError,
		  "Failed to parse request params (%d)", status );

    return Qtrue;
}


 static VALUE request_script_name(VALUE self)
{
    request_data *data = get_request_data( self );
    const char *val;

    val = ApacheRequest_script_name(data->apreq);
    return val ? rb_tainted_str_new2(val) : Qnil;
}

static VALUE request_script_path(VALUE self)
{
    request_data *data = get_request_data( self );
    const char *val;

    val = ApacheRequest_script_path(data->apreq);
    return val ? rb_tainted_str_new2(val) : Qnil;
}

static VALUE request_param(VALUE self, VALUE key)
{
    request_data *data = get_request_data(self);
    const char *val;

    if (!data->apreq->parsed) rb_funcall(self, rb_intern("parse"), 0);
    val = ApacheRequest_param(data->apreq, StringValuePtr(key));
    return val ? rb_tainted_str_new2(val) : Qnil;
}

static VALUE request_params(VALUE self, VALUE key)
{
    request_data *data = get_request_data(self);
    array_header *val;

    if (!data->apreq->parsed) rb_funcall(self, rb_intern("parse"), 0);
    val = ApacheRequest_params(data->apreq, StringValuePtr(key));
    return val ? rb_apache_array_new(val) : Qnil;
}

static int make_all_params(void *data, const char *key, const char *val)
{
    VALUE hash = (VALUE) data;
    VALUE vkey, ary;

    vkey = rb_tainted_str_new2(key);
    ary = rb_hash_aref(hash, vkey);
    if (NIL_P(ary)) {
	ary = rb_ary_new();
	rb_hash_aset(hash, vkey, ary);
    }
    rb_ary_push(ary, rb_tainted_str_new2(val));
    return 1;
}

static VALUE request_all_params(VALUE self, VALUE key)
{
    request_data *data = get_request_data(self);
    VALUE result;

    if (!data->apreq->parsed) rb_funcall(self, rb_intern("parse"), 0);
    result = rb_hash_new();
    apr_table_do(make_all_params, (void *) result, data->apreq->parms, NULL);
    return result;
}

static VALUE request_paramtable( VALUE self )
{
    request_data *data = get_request_data( self );

    if ( !data->apreq->parsed ) rb_funcall( self, rb_intern("parse"), 0 );

    if ( !RTEST(data->param_table) ) {
	data->param_table = rb_apache_paramtable_new(data->apreq->parms);
    }

    return data->param_table;
}


static VALUE request_params_as_string(VALUE self, VALUE key)
{
    request_data *data = get_request_data( self );
    const char *val;

    if ( !data->apreq->parsed ) rb_funcall( self, rb_intern("parse"), 0 );
    val = ApacheRequest_params_as_string(data->apreq, StringValuePtr(key));
    return val ? rb_tainted_str_new2(val) : Qnil;
}

static VALUE request_cookies( VALUE self )
{
    request_data *data = get_request_data( self );

    if ( !data->apreq->parsed ) rb_funcall( self, rb_intern("parse"), 0 );

    if ( ! RHASH(data->cookies)->tbl->num_entries ) {
	ApacheCookieJar *jar = ApacheCookie_parse( data->request, NULL );
    int i;

	for ( i = 0; i < ApacheCookieJarItems(jar); i++ ) { 
	    ApacheCookie *c = ApacheCookieJarFetch( jar, i );
	    VALUE cookie = rb_apache_cookie_new(c);
	    rb_hash_aset( data->cookies, rb_tainted_str_new2(c->name), cookie );
	}
    }

    return data->cookies;
}

static VALUE request_cookies_eq( VALUE self, VALUE newhash )
{
    request_data *data = get_request_data( self );

    Check_Type( newhash, T_HASH );
    data->cookies = newhash;

    return newhash;
}

void rb_init_apache_request()
{
    rb_eApachePrematureChunkEndError =
	rb_define_class_under(rb_mApache, "PrematureChunkEndError",
			      rb_eStandardError);

    rb_cApacheRequest = rb_define_class_under(rb_mApache, "Request", rb_cObject);
    rb_include_module(rb_cApacheRequest, rb_mEnumerable);
    rb_undef_method(CLASS_OF(rb_cApacheRequest), "new");
    rb_define_method(rb_cApacheRequest, "inspect", rb_any_to_s, 0);
    rb_define_method(rb_cApacheRequest, "to_s", request_output_buffer, 0);
    rb_define_method(rb_cApacheRequest, "output_buffer",
		     request_output_buffer, 0);
    rb_define_method(rb_cApacheRequest, "replace", request_replace, -1);
    rb_define_method(rb_cApacheRequest, "cancel", request_cancel, 0);
    rb_define_method(rb_cApacheRequest, "sync_header",
		     request_get_sync_header, 0);
    rb_define_method(rb_cApacheRequest, "sync_header=",
		     request_set_sync_header, 1);
    rb_define_method(rb_cApacheRequest, "sync_output",
		     request_get_sync_output, 0);
    rb_define_method(rb_cApacheRequest, "sync_output=",
		     request_set_sync_output, 1);
    rb_define_method(rb_cApacheRequest, "sync=",
		     request_set_sync, 1);
    rb_define_method(rb_cApacheRequest, "write", request_write, 1);
    rb_define_method(rb_cApacheRequest, "putc", request_putc, 1);
    rb_define_method(rb_cApacheRequest, "print", request_print, -1);
    rb_define_method(rb_cApacheRequest, "printf", request_printf, -1);
    rb_define_method(rb_cApacheRequest, "puts", request_puts, -1);
    rb_define_method(rb_cApacheRequest, "<<", request_addstr, 1);
    rb_define_method(rb_cApacheRequest, "send_http_header",
		     rb_apache_request_send_http_header, 0);
    rb_define_method(rb_cApacheRequest, "sent_http_header?",
		     request_sent_http_header, 0);
    rb_define_method(rb_cApacheRequest, "connection", request_connection, 0);
    rb_define_method(rb_cApacheRequest, "server", request_server, 0);
    rb_define_method(rb_cApacheRequest, "next", request_next, 0);
    rb_define_method(rb_cApacheRequest, "prev", request_prev, 0);
    rb_define_method(rb_cApacheRequest, "last", request_last, 0);
    rb_define_method(rb_cApacheRequest, "main", request_main, 0);
    rb_define_method(rb_cApacheRequest, "protocol", request_protocol, 0);
    rb_define_method(rb_cApacheRequest, "hostname", request_hostname, 0);
    rb_define_method(rb_cApacheRequest, "unparsed_uri",
		     request_unparsed_uri, 0);
    rb_define_method(rb_cApacheRequest, "uri", request_get_uri, 0);
    rb_define_method(rb_cApacheRequest, "uri=", request_set_uri, 1);
    rb_define_method(rb_cApacheRequest, "filename", request_get_filename, 0);
    rb_define_method(rb_cApacheRequest, "filename=", request_set_filename, 1);
    rb_define_method(rb_cApacheRequest, "path_info", request_get_path_info, 0);
    rb_define_method(rb_cApacheRequest, "path_info=", request_set_path_info, 1);
    rb_define_method(rb_cApacheRequest, "request_time",
		     request_request_time, 0);
    rb_define_method(rb_cApacheRequest, "status", request_get_status, 0);
    rb_define_method(rb_cApacheRequest, "status=", request_set_status, 1);
    rb_define_method(rb_cApacheRequest, "status_line", request_get_status_line, 0);
    rb_define_method(rb_cApacheRequest, "status_line=",
		     request_set_status_line, 1);
    rb_define_method(rb_cApacheRequest, "request_method",
		     request_request_method, 0);
    rb_define_method(rb_cApacheRequest, "method_number",
		     request_method_number, 0);
    rb_define_method(rb_cApacheRequest, "allowed", request_get_allowed, 0);
    rb_define_method(rb_cApacheRequest, "allowed=", request_set_allowed, 1);
    rb_define_method(rb_cApacheRequest, "the_request",
		     request_the_request, 0);
    rb_define_method(rb_cApacheRequest, "assbackwards?", request_get_assbackwards, 0);
    rb_define_method(rb_cApacheRequest, "assbackwards=", request_set_assbackwards, 1);
    rb_define_method(rb_cApacheRequest, "header_only?", request_header_only, 0);
    rb_define_method(rb_cApacheRequest, "args", request_get_args, 0);
    rb_define_method(rb_cApacheRequest, "args=", request_set_args, 1);
    rb_define_method(rb_cApacheRequest, "content_length",
		     request_content_length, 0);
    rb_define_method(rb_cApacheRequest, "content_type",
		     request_get_content_type, 0);
    rb_define_method(rb_cApacheRequest, "content_type=",
		     request_set_content_type, 1);
    rb_define_method(rb_cApacheRequest, "content_encoding",
		     request_get_content_encoding, 0);
    rb_define_method(rb_cApacheRequest, "content_encoding=",
		     request_set_content_encoding, 1);
    rb_define_method(rb_cApacheRequest, "content_languages",
		     request_get_content_languages, 0);
    rb_define_method(rb_cApacheRequest, "content_languages=",
		     request_set_content_languages, 1);
    rb_define_method(rb_cApacheRequest, "headers_in", request_headers_in, 0);
    rb_define_method(rb_cApacheRequest, "headers_out", request_headers_out, 0);
    rb_define_method(rb_cApacheRequest, "err_headers_out",
		     request_err_headers_out, 0);
    rb_define_method(rb_cApacheRequest, "subprocess_env",
		     request_subprocess_env, 0);
    rb_define_method(rb_cApacheRequest, "notes", request_notes, 0);
    rb_define_method(rb_cApacheRequest, "finfo", request_finfo, 0);
    rb_define_method(rb_cApacheRequest, "parsed_uri", request_parsed_uri, 0);
    rb_define_method(rb_cApacheRequest, "attributes", request_attributes, 0);
    rb_define_method(rb_cApacheRequest, "setup_client_block",
		     request_setup_client_block, -1);
    rb_define_method(rb_cApacheRequest, "should_client_block",
		     request_should_client_block, 0);
    rb_define_method(rb_cApacheRequest, "should_client_block?",
		     request_should_client_block, 0);
    rb_define_method(rb_cApacheRequest, "get_client_block",
		     request_get_client_block, 1);
    rb_define_method(rb_cApacheRequest, "read", request_read, -1);
    rb_define_method(rb_cApacheRequest, "getc", request_getc, 0);
    rb_define_method(rb_cApacheRequest, "eof", request_eof, 0);
    rb_define_method(rb_cApacheRequest, "eof?", request_eof, 0);
    rb_define_method(rb_cApacheRequest, "binmode", request_binmode, 0);
    rb_define_method(rb_cApacheRequest, "allow_options",
		     request_allow_options, 0);
    rb_define_method(rb_cApacheRequest, "allow_overrides",
		     request_allow_overrides, 0);
    rb_define_method(rb_cApacheRequest, "default_type",
		     request_default_type, 0);
    rb_define_method(rb_cApacheRequest, "default_port", request_default_port, 0);
    rb_define_method(rb_cApacheRequest, "remote_host",
		     request_remote_host, -1);
    rb_define_method(rb_cApacheRequest, "remote_logname",
		     request_remote_logname, 0);
    rb_define_method(rb_cApacheRequest, "construct_url",
		     request_construct_url, 1);
    rb_define_method(rb_cApacheRequest, "server_name", request_server_name, 0);
    rb_define_method(rb_cApacheRequest, "server_port", request_server_port, 0);
    rb_define_method(rb_cApacheRequest, "satisfies", request_satisfies, 0);
    rb_define_method(rb_cApacheRequest, "requires", request_requires, 0);
    rb_define_method(rb_cApacheRequest, "escape_html", request_escape_html, 1);
    rb_define_method(rb_cApacheRequest, "signature", request_signature, 0);
    rb_define_method(rb_cApacheRequest, "reset_timeout",
		     request_reset_timeout, 0);
    rb_define_method(rb_cApacheRequest, "hard_timeout",
		     request_hard_timeout, 1);
    rb_define_method(rb_cApacheRequest, "soft_timeout",
		     request_soft_timeout, 1);
    rb_define_method(rb_cApacheRequest, "kill_timeout",
		     request_kill_timeout, 0);
    rb_define_method(rb_cApacheRequest, "internal_redirect",
		     request_internal_redirect, 1);
    rb_define_method(rb_cApacheRequest, "custom_response",
		     request_custom_response, 2);
    rb_define_method(rb_cApacheRequest, "main?",
		     request_is_main, 0);
    rb_define_method(rb_cApacheRequest, "initial?",
		     request_is_initial, 0);
    rb_define_method(rb_cApacheRequest, "note_auth_failure",
		     request_note_auth_failure, 0);
    rb_define_method(rb_cApacheRequest, "note_basic_auth_failure",
		     request_note_basic_auth_failure, 0);
    rb_define_method(rb_cApacheRequest, "note_digest_auth_failure",
		     request_note_digest_auth_failure, 0);
    rb_define_method(rb_cApacheRequest, "get_basic_auth_pw",
		     request_get_basic_auth_pw, 0);
    rb_define_method(rb_cApacheRequest, "add_common_vars",
		     request_add_common_vars, 0);
    rb_define_method(rb_cApacheRequest, "add_cgi_vars",
		     request_add_cgi_vars, 0);
    rb_define_method(rb_cApacheRequest, "setup_cgi_env",
		     request_setup_cgi_env, 0);
    rb_define_method(rb_cApacheRequest, "log_reason",
		     request_log_reason, 2);
    rb_define_method(rb_cApacheRequest, "error_message",
		     request_error_message, 0);
    rb_define_method(rb_cApacheRequest, "exception",
		     request_exception, 0);
    rb_define_method(rb_cApacheRequest, "user",
		     request_user, 0);
    rb_define_method(rb_cApacheRequest, "user=",
		     request_set_user, 1);
    rb_define_method(rb_cApacheRequest, "auth_type", request_auth_type, 0);
    rb_define_method(rb_cApacheRequest, "auth_type=", request_set_auth_type, 1);
    rb_define_method(rb_cApacheRequest, "auth_name", request_auth_name, 0);
    rb_define_method(rb_cApacheRequest, "auth_name=", request_set_auth_name, 1);
    rb_define_method(rb_cApacheRequest, "default_charset",
		     request_default_charset, 0);
    rb_define_method(rb_cApacheRequest, "bytes_sent", request_bytes_sent, 0);
    rb_define_method(rb_cApacheRequest, "send_fd", request_send_fd, 1);
    rb_define_method(rb_cApacheRequest, "proxy?",
		     request_proxy_q, 0);
    rb_define_method(rb_cApacheRequest, "proxy_pass?",
		     request_proxy_pass_q, 0);
    rb_define_method(rb_cApacheRequest, "dispatch_handler",
		     request_get_dispatch_handler, 0);
    rb_define_method(rb_cApacheRequest, "dispatch_handler=",
		     request_set_dispatch_handler, 1);
    rb_define_method(rb_cApacheRequest, "cache_resp",
		     request_get_cache_resp, 0);
    rb_define_method(rb_cApacheRequest, "cache_resp=",
		     request_set_cache_resp, 1);
    rb_define_method(rb_cApacheRequest, "lookup_uri", request_lookup_uri, 1);
    rb_define_method(rb_cApacheRequest, "lookup_file", request_lookup_file, 1);
    rb_define_method(rb_cApacheRequest, "register_cleanup",
		     request_register_cleanup, -1);
    rb_define_method(rb_cApacheRequest, "options", request_options, 0);

    rb_define_singleton_method(rb_cApacheRequest, "libapreq?", request_libapreq_p, 0 );

    id_post_max = rb_intern( "post_max" );
    id_disable_uploads = rb_intern( "disable_uploads" );
    id_temp_dir = rb_intern( "temp_dir" );
    id_hook_data = rb_intern( "hook_data" );
    id_upload_hook = rb_intern( "upload_hook" );

    rb_define_method(rb_cApacheRequest, "parse", request_parse, -1);
    rb_define_method(rb_cApacheRequest, "script_name", request_script_name, 0);
    rb_define_method(rb_cApacheRequest, "script_path", request_script_path, 0);
    rb_define_method(rb_cApacheRequest, "param", request_param, 1);
    rb_define_method(rb_cApacheRequest, "params", request_params, 1);
    rb_define_method(rb_cApacheRequest, "all_params", request_all_params, 0);
    rb_define_method(rb_cApacheRequest, "paramtable", request_paramtable, 0);
    rb_define_method(rb_cApacheRequest, "params_as_string",
		     request_params_as_string, 1);
    rb_define_method(rb_cApacheRequest, "uploads", request_uploads, 0);
    rb_define_method(rb_cApacheRequest, "cookies", request_cookies, 0);
    rb_define_method(rb_cApacheRequest, "cookies=", request_cookies_eq, 1);

    rb_define_method(rb_cApacheRequest, "post_max=", request_post_max_eq, 1);
    rb_define_method(rb_cApacheRequest, "post_max", request_post_max, 0);
    rb_define_method(rb_cApacheRequest, "disable_uploads=", request_disable_uploads_eq, 1);
    rb_define_method(rb_cApacheRequest, "disable_uploads?", request_disable_uploads_p, 0);
    rb_define_alias (rb_cApacheRequest, "uploads_disabled?", "disable_uploads?");
    rb_define_method(rb_cApacheRequest, "temp_dir=", request_temp_dir_eq, 1);
    rb_define_method(rb_cApacheRequest, "temp_dir", request_temp_dir, 0);
    rb_define_method(rb_cApacheRequest, "upload_hook=", request_upload_hook_eq, 1);
    rb_define_method(rb_cApacheRequest, "upload_hook", request_upload_hook, 0);
    rb_define_method(rb_cApacheRequest, "upload_hook_data=", request_upload_hook_data_eq, 1);
    rb_define_method(rb_cApacheRequest, "upload_hook_data", request_upload_hook_data, 0);
}

/*
 * Local variables:
 * mode: C
 * tab-width: 8
 * End:
 */

/* vim: set filetype=c ts=8 sw=4 : */
