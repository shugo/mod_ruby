/*
 * $Id$
 * Copyright (C) 2004  Shugo Maeda <shugo@modruby.net>
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

#include "mod_ruby.h"
#include "apachelib.h"

#ifdef APACHE2

VALUE rb_cApacheBucket;
VALUE rb_cApacheBucketBrigade;

static VALUE bucket_new(apr_bucket *b)
{
    if (b == NULL)
	return Qnil;
    return Data_Wrap_Struct(rb_cApacheBucket, NULL, NULL, b);
}

static VALUE bucket_next(VALUE self)
{
    apr_bucket *b;

    Data_Get_Struct(self, apr_bucket, b);
    return bucket_new(APR_BUCKET_NEXT(b));
}

static VALUE bucket_prev(VALUE self)
{
    apr_bucket *b;

    Data_Get_Struct(self, apr_bucket, b);
    return bucket_new(APR_BUCKET_PREV(b));
}

static VALUE bucket_is_metadata(VALUE self)
{
    apr_bucket *b;

    Data_Get_Struct(self, apr_bucket, b);
    return APR_BUCKET_IS_METADATA(b) ? Qtrue : Qfalse;
}

static VALUE bucket_is_flush(VALUE self)
{
    apr_bucket *b;

    Data_Get_Struct(self, apr_bucket, b);
    return APR_BUCKET_IS_FLUSH(b) ? Qtrue : Qfalse;
}

static VALUE bucket_is_eos(VALUE self)
{
    apr_bucket *b;

    Data_Get_Struct(self, apr_bucket, b);
    return APR_BUCKET_IS_EOS(b) ? Qtrue : Qfalse;
}

static VALUE bucket_is_file(VALUE self)
{
    apr_bucket *b;

    Data_Get_Struct(self, apr_bucket, b);
    return APR_BUCKET_IS_FILE(b) ? Qtrue : Qfalse;
}

static VALUE bucket_is_pipe(VALUE self)
{
    apr_bucket *b;

    Data_Get_Struct(self, apr_bucket, b);
    return APR_BUCKET_IS_PIPE(b) ? Qtrue : Qfalse;
}

static VALUE bucket_is_socket(VALUE self)
{
    apr_bucket *b;

    Data_Get_Struct(self, apr_bucket, b);
    return APR_BUCKET_IS_SOCKET(b) ? Qtrue : Qfalse;
}

static VALUE bucket_is_heap(VALUE self)
{
    apr_bucket *b;

    Data_Get_Struct(self, apr_bucket, b);
    return APR_BUCKET_IS_HEAP(b) ? Qtrue : Qfalse;
}

static VALUE bucket_is_transient(VALUE self)
{
    apr_bucket *b;

    Data_Get_Struct(self, apr_bucket, b);
    return APR_BUCKET_IS_TRANSIENT(b) ? Qtrue : Qfalse;
}

static VALUE bucket_is_immortal(VALUE self)
{
    apr_bucket *b;

    Data_Get_Struct(self, apr_bucket, b);
    return APR_BUCKET_IS_IMMORTAL(b) ? Qtrue : Qfalse;
}

static VALUE bucket_is_mmap(VALUE self)
{
#if APR_HAS_MMAP
    apr_bucket *b;

    Data_Get_Struct(self, apr_bucket, b);
    return APR_BUCKET_IS_MMAP(b) ? Qtrue : Qfalse;
#else
    return Qfalse;
#endif
}

static VALUE bucket_is_pool(VALUE self)
{
    apr_bucket *b;

    Data_Get_Struct(self, apr_bucket, b);
    return APR_BUCKET_IS_POOL(b) ? Qtrue : Qfalse;
}

static VALUE bucket_read(int argc, VALUE *argv, VALUE self)
{
    VALUE length, block;
    apr_bucket *b;
    apr_size_t len;
    const char *str;
    apr_status_t status;
    apr_read_type_e read_type = APR_BLOCK_READ;

    if (rb_scan_args(argc, argv, "11", &length, &block) == 2) {
	read_type = RTEST(block) ? APR_BLOCK_READ : APR_NONBLOCK_READ;
    }
    Data_Get_Struct(self, apr_bucket, b);
    len = NUM2ULONG(length);
    status = apr_bucket_read(b, &str, &len, read_type);
    if (status != APR_SUCCESS) {
	rb_raise(rb_eRuntimeError, "error");
    }
    return rb_str_new(str, len);
}

VALUE rb_apache_bucket_brigade_new(apr_bucket_brigade *bb)
{
    return Data_Wrap_Struct(rb_cApacheBucketBrigade, NULL, NULL, bb);
}

static VALUE brigade_sentinel(VALUE self)
{
    apr_bucket_brigade *bb;

    Data_Get_Struct(self, apr_bucket_brigade, bb);
    return bucket_new(APR_BRIGADE_SENTINEL(bb));
}

static VALUE brigade_is_empty(VALUE self)
{
    apr_bucket_brigade *bb;

    Data_Get_Struct(self, apr_bucket_brigade, bb);
    return APR_BRIGADE_EMPTY(bb) ? Qtrue : Qfalse;
}

static VALUE brigade_first(VALUE self)
{
    apr_bucket_brigade *bb;

    Data_Get_Struct(self, apr_bucket_brigade, bb);
    return bucket_new(APR_BRIGADE_FIRST(bb));
}

static VALUE brigade_last(VALUE self)
{
    apr_bucket_brigade *bb;

    Data_Get_Struct(self, apr_bucket_brigade, bb);
    return bucket_new(APR_BRIGADE_LAST(bb));
}

static VALUE brigade_each(VALUE self)
{
    apr_bucket_brigade *bb;
    apr_bucket *e;

    Data_Get_Struct(self, apr_bucket_brigade, bb);
    for (e = APR_BRIGADE_FIRST(bb);
	 e != APR_BRIGADE_SENTINEL(bb);
	 e = APR_BUCKET_NEXT(e)) {
	rb_yield(bucket_new(e));
    }
    return Qnil;
}

void rb_init_apache_bucket()
{
    rb_cApacheBucket = rb_define_class_under(rb_mApache, "Bucket", rb_cObject);
    rb_define_method(rb_cApacheBucket, "next", bucket_next, 0);
    rb_define_method(rb_cApacheBucket, "prev", bucket_prev, 0);
    rb_define_method(rb_cApacheBucket, "metadata?", bucket_is_metadata, 0);
    rb_define_method(rb_cApacheBucket, "flush?", bucket_is_flush, 0);
    rb_define_method(rb_cApacheBucket, "eos?", bucket_is_eos, 0);
    rb_define_method(rb_cApacheBucket, "file?", bucket_is_file, 0);
    rb_define_method(rb_cApacheBucket, "pipe?", bucket_is_pipe, 0);
    rb_define_method(rb_cApacheBucket, "socket?", bucket_is_socket, 0);
    rb_define_method(rb_cApacheBucket, "heap?", bucket_is_heap, 0);
    rb_define_method(rb_cApacheBucket, "transient?", bucket_is_transient, 0);
    rb_define_method(rb_cApacheBucket, "immortal?", bucket_is_immortal, 0);
    rb_define_method(rb_cApacheBucket, "mmap?", bucket_is_mmap, 0);
    rb_define_method(rb_cApacheBucket, "pool?", bucket_is_pool, 0);
    rb_define_method(rb_cApacheBucket, "read", bucket_read, -1);

    rb_cApacheBucketBrigade =
	rb_define_class_under(rb_mApache, "BucketBrigade", rb_cObject);
    rb_include_module(rb_cApacheArrayHeader, rb_mEnumerable);
    rb_define_method(rb_cApacheBucketBrigade, "sentinel", brigade_sentinel, 0);
    rb_define_method(rb_cApacheBucketBrigade, "empty?", brigade_is_empty, 0);
    rb_define_method(rb_cApacheBucketBrigade, "first", brigade_first, 0);
    rb_define_method(rb_cApacheBucketBrigade, "last", brigade_last, 0);
    rb_define_method(rb_cApacheBucketBrigade, "each", brigade_each, 0);
}

#endif
