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

VALUE rb_cApacheTable;

VALUE rb_apache_table_new(table *tbl)
{
    return Data_Wrap_Struct(rb_cApacheTable, NULL, NULL, tbl);
}

static VALUE table_clear(VALUE self)
{
    table *tbl;

    Data_Get_Struct(self, table, tbl);
    apr_table_clear(tbl);
    return Qnil;
}

static VALUE table_get(VALUE self, VALUE name)
{
    table *tbl;
    const char *key, *res;

    key = StringValuePtr(name);
    Data_Get_Struct(self, table, tbl);
    res = apr_table_get(tbl, key);
    if (res)
	return rb_tainted_str_new2(res);
    else
	return Qnil;
}

static VALUE table_set(VALUE self, VALUE name, VALUE val)
{
    table *tbl;

    Check_Type(val, T_STRING);
    Data_Get_Struct(self, table, tbl);
    apr_table_set(tbl, StringValuePtr(name), StringValuePtr(val));
    return val;
}

static VALUE table_merge(VALUE self, VALUE name, VALUE val)
{
    table *tbl;

    Check_Type(val, T_STRING);
    Data_Get_Struct(self, table, tbl);
    apr_table_merge(tbl, StringValuePtr(name), StringValuePtr(val));
    return val;
}

static VALUE table_unset(VALUE self, VALUE name)
{
    table *tbl;

    Data_Get_Struct(self, table, tbl);
    apr_table_unset(tbl, StringValuePtr(name));
    return Qnil;
}

static VALUE table_add(VALUE self, VALUE name, VALUE val)
{
    table *tbl;

    Check_Type(val, T_STRING);
    Data_Get_Struct(self, table, tbl);
    apr_table_add(tbl, StringValuePtr(name), StringValuePtr(val));
    return val;
}

static VALUE table_each(VALUE self)
{
    VALUE key, val, assoc;
    table *tbl;
    const array_header *hdrs_arr;
    table_entry *hdrs;
    int i;

    Data_Get_Struct(self, table, tbl);
    hdrs_arr = apr_table_elts(tbl);
    hdrs = (table_entry *) hdrs_arr->elts;
    for (i = 0; i < hdrs_arr->nelts; i++) {
	if (hdrs[i].key == NULL)
	    continue;
	key = rb_tainted_str_new2(hdrs[i].key);
	val = hdrs[i].val ? rb_tainted_str_new2(hdrs[i].val) : Qnil;
	assoc = rb_assoc_new(key, val);
	rb_yield(assoc);
    }
    return Qnil;
}

static VALUE table_each_key(VALUE self)
{
    table *tbl;
    const array_header *hdrs_arr;
    table_entry *hdrs;
    int i;

    Data_Get_Struct(self, table, tbl);
    hdrs_arr = apr_table_elts(tbl);
    hdrs = (table_entry *) hdrs_arr->elts;
    for (i = 0; i < hdrs_arr->nelts; i++) {
	if (hdrs[i].key == NULL)
	    continue;
	rb_yield(rb_tainted_str_new2(hdrs[i].key));
    }
    return Qnil;
}

static VALUE table_each_value(VALUE self)
{
    table *tbl;
    const array_header *hdrs_arr;
    table_entry *hdrs;
    int i;

    Data_Get_Struct(self, table, tbl);
    hdrs_arr = apr_table_elts(tbl);
    hdrs = (table_entry *) hdrs_arr->elts;
    for (i = 0; i < hdrs_arr->nelts; i++) {
	if (hdrs[i].key == NULL)
	    continue;
	rb_yield(hdrs[i].val ? rb_tainted_str_new2(hdrs[i].val) : Qnil);
    }
    return Qnil;
}

void rb_init_apache_table()
{
    rb_cApacheTable = rb_define_class_under(rb_mApache, "Table", rb_cObject);
    rb_include_module(rb_cApacheTable, rb_mEnumerable);
    rb_undef_method(CLASS_OF(rb_cApacheTable), "new");
    rb_define_method(rb_cApacheTable, "clear", table_clear, 0);
    rb_define_method(rb_cApacheTable, "get", table_get, 1);
    rb_define_method(rb_cApacheTable, "[]", table_get, 1);
    rb_define_method(rb_cApacheTable, "set", table_set, 2);
    rb_define_method(rb_cApacheTable, "[]=", table_set, 2);
    rb_define_method(rb_cApacheTable, "merge", table_merge, 2);
    rb_define_method(rb_cApacheTable, "unset", table_unset, 1);
    rb_define_method(rb_cApacheTable, "add", table_add, 2);
    rb_define_method(rb_cApacheTable, "each", table_each, 0);
    rb_define_method(rb_cApacheTable, "each_key", table_each_key, 0);
    rb_define_method(rb_cApacheTable, "each_value", table_each_value, 0);
}

/* vim: set filetype=c ts=8 sw=4 : */
