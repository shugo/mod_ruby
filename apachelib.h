/*
 * $Id$
 * Copyright (C) 2000  ZetaBITS, Inc.
 * Copyright (C) 2000  Information-technology Promotion Agency, Japan
 * Copyright (C) 2000-2003  Shugo Maeda <shugo@modruby.net>
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

#ifndef APACHELIB_H
#define APACHELIB_H

/* apachelib.c */
extern VALUE rb_mApache;
extern VALUE rb_eApacheTimeoutError;
extern VALUE rb_request;
extern VALUE rb_apache_objrefs;
void rb_init_apache();
void rb_apache_exit(int status);
VALUE rb_apache_register_object(VALUE obj);
VALUE rb_apache_unregister_object(VALUE obj);
#ifdef APACHE2
void rb_apr_fail(apr_status_t status);
#endif

/* array_header.c */
extern VALUE rb_cApacheArrayHeader;
void rb_init_apache_array();
VALUE rb_apache_array_new(array_header *arr);

/* table.c */
extern VALUE rb_cApacheTable;
extern VALUE rb_cApacheRestrictedTable;
void rb_init_apache_table();
VALUE rb_apache_table_new(table *tbl);

/* connection.c */
extern VALUE rb_cApacheConnection;
void rb_init_apache_connection();
VALUE rb_apache_connection_new(conn_rec *conn);

/* server.c */
extern VALUE rb_cApacheServer;
void rb_init_apache_server();
VALUE rb_apache_server_new(server_rec *server);

/* request.c */
extern VALUE rb_cApacheRequest;
void rb_init_apache_request();
VALUE rb_get_request_object(request_rec *r);
request_rec *rb_get_request_rec(VALUE obj);
long rb_apache_request_length(VALUE self);
VALUE rb_apache_request_send_http_header(VALUE self);
void rb_apache_request_flush(VALUE request);
void rb_apache_request_set_error(VALUE request, VALUE error, VALUE exception);

/* bucket.c */
#ifdef APACHE2
extern VALUE rb_cApacheBucket;
void rb_init_apache_bucket();
#endif

/* uri.c */
extern VALUE rb_cApacheUri;
void rb_init_apache_uri();
VALUE rb_apache_uri_new(uri_components *uri);

/* error.c */
extern VALUE rb_cApacheError;
void rb_init_apache_error();
VALUE rb_apache_error_new(request_rec *r, error_log_data *error);

/* --- Libapreq Extensions ------------------------------  */

extern VALUE rb_eApacheRequestError;

/* upload.c */
extern VALUE rb_cApacheUpload;
void rb_init_apache_upload();
VALUE rb_apache_upload_new(ApacheUpload *upload);

/* paramtable.c */
extern VALUE rb_cApacheParamTable;
void rb_init_apache_paramtable();
VALUE rb_apache_paramtable_new(table *tbl);

/* cookie.c */
extern VALUE rb_cApacheCookie;
void rb_init_apache_cookie();
VALUE rb_apache_cookie_new(ApacheCookie *cookie);

/* multival.c */
extern VALUE rb_cApacheMultiVal;
void rb_init_apache_multival();


#define STRING_LITERAL(s) rb_str_new(s, sizeof(s) - 1)
#define CSTR2OBJ(s) ((s) ? rb_tainted_str_new2(s) : Qnil)
#define INT2BOOL(n) ((n) ? Qtrue : Qfalse)

#define DEFINE_ATTR_READER(fname, type, member, convert) \
static VALUE fname(VALUE self) \
{ \
    type *data; \
    Data_Get_Struct(self, type, data); \
    if (data == NULL) \
	rb_raise(rb_eArgError, "destroyed object"); \
    return convert(data->member); \
}

#define DEFINE_ATTR_WRITER(fname, type, member, convert) \
static VALUE fname(VALUE self, VALUE val) \
{ \
    type *data; \
    Data_Get_Struct(self, type, data); \
    if (data == NULL) \
	rb_raise(rb_eArgError, "destroyed object"); \
    data->member = convert(val); \
    return val; \
}

#define DEFINE_STRING_ATTR_READER(fname, type, member) \
	DEFINE_ATTR_READER(fname, type, member, CSTR2OBJ)

#define DEFINE_INT_ATTR_READER(fname, type, member) \
	DEFINE_ATTR_READER(fname, type, member, INT2NUM)
#define DEFINE_INT_ATTR_WRITER(fname, type, member) \
	DEFINE_ATTR_WRITER(fname, type, member, NUM2INT)

#define DEFINE_UINT_ATTR_READER(fname, type, member) \
	DEFINE_ATTR_READER(fname, type, member, UINT2NUM)
#define DEFINE_UINT_ATTR_WRITER(fname, type, member) \
	DEFINE_ATTR_WRITER(fname, type, member, NUM2UINT)

#define DEFINE_LONG_ATTR_READER(fname, type, member) \
	DEFINE_ATTR_READER(fname, type, member, INT2NUM)
#define DEFINE_LONG_ATTR_WRITER(fname, type, member) \
	DEFINE_ATTR_WRITER(fname, type, member, NUM2LONG)

#define DEFINE_ULONG_ATTR_READER(fname, type, member) \
	DEFINE_ATTR_READER(fname, type, member, UINT2NUM)
#define DEFINE_ULONG_ATTR_WRITER(fname, type, member) \
	DEFINE_ATTR_WRITER(fname, type, member, NUM2ULONG)

#define DEFINE_BOOL_ATTR_READER(fname, type, member) \
	DEFINE_ATTR_READER(fname, type, member, INT2BOOL)
#define DEFINE_BOOL_ATTR_WRITER(fname, type, member) \
	DEFINE_ATTR_WRITER(fname, type, member, RTEST)

#endif /* !APACHELIB_H */

/*
 * Local variables:
 * mode: C
 * tab-width: 8
 * End:
 */

/* vim: set filetype=c ts=8 sw=4 : */
