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

#include "mod_ruby.h"
#include "apachelib.h"

VALUE rb_cApacheArrayHeader;

VALUE rb_apache_array_new(array_header *arr)
{
    return Data_Wrap_Struct(rb_cApacheArrayHeader, NULL, NULL, arr);
}

static VALUE array_length(VALUE self, VALUE idx)
{
    array_header *arr;

    Data_Get_Struct(self, array_header, arr);
    return INT2NUM(arr->nelts);
}

static VALUE array_aref(VALUE self, VALUE idx)
{
    array_header *arr;
    int n;

    Data_Get_Struct(self, array_header, arr);
    n = NUM2INT(idx);
    if (n < 0) {
	n += arr->nelts;
	if (n < 0) {
	    rb_raise(rb_eIndexError, "index %d out of array", n - arr->nelts);
	}
    }
    else if (n >= arr->nelts) {
	rb_raise(rb_eIndexError, "index %d out of array", n);
    }
    return rb_tainted_str_new2(((char **) arr->elts)[n]);
}

static VALUE array_aset(VALUE self, VALUE idx, VALUE val)
{
    array_header *arr;
    int n;

    Check_Type(val, T_STRING);
    Data_Get_Struct(self, array_header, arr);
    n = NUM2INT(idx);
    if (n < 0) {
	n += arr->nelts;
	if (n < 0) {
	    rb_raise(rb_eIndexError, "index %d out of array", n - arr->nelts);
	}
    }
    else if (n >= arr->nelts) {
	rb_raise(rb_eIndexError, "index %d out of array", n);
    }
    ((char **) arr->elts)[n] = ap_pstrndup(arr->pool,
					   RSTRING(val)->ptr,
					   RSTRING(val)->len);
    return val;
}

static VALUE array_each(VALUE self)
{
    array_header *arr;
    int i;

    Data_Get_Struct(self, array_header, arr);
    for (i = 0; i < arr->nelts; i++) {
	rb_yield(rb_tainted_str_new2(((char **) arr->elts)[i]));
    }
    return Qnil;
}

void rb_init_apache_array()
{
    rb_cApacheArrayHeader = rb_define_class_under(rb_mApache, "ArrayHeader",
						  rb_cObject);
    rb_include_module(rb_cApacheArrayHeader, rb_mEnumerable);
    rb_undef_method(CLASS_OF(rb_cApacheArrayHeader), "new");
    rb_define_method(rb_cApacheArrayHeader, "length", array_length, 0);
    rb_define_method(rb_cApacheArrayHeader, "[]", array_aref, 1);
    rb_define_method(rb_cApacheArrayHeader, "[]=", array_aset, 2);
    rb_define_method(rb_cApacheArrayHeader, "each", array_each, 0);
}

/* vim: set filetype=c ts=8 sw=4 : */
