/*
 * paramtable.c - The Apache::ParamTable class
 * $Id$
 *
 * Author: Michael Granger <mgranger@RubyCrafters.com>
 * Copyright (c) 2003 RubyCrafters, LLC. All rights reserved.
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
 * 
 */

#include "mod_ruby.h"
#include "apachelib.h"

VALUE rb_cApacheParamTable;

static ID atargs_id;


VALUE rb_apache_paramtable_new(table *tbl)
{
    return Data_Wrap_Struct(rb_cApacheParamTable, NULL, NULL, tbl);
}

/*
 * Object validity checker. Returns the data pointer.
 */
static table *check_paramtable( VALUE self )
{
    Check_Type( self, T_DATA );
    if ( !rb_obj_is_instance_of(self, rb_cApacheParamTable) ) {
        rb_raise( rb_eTypeError,
                  "wrong argument type %s (expected Apache::ParamTable)",
                  rb_class2name(CLASS_OF( self )) );
    }
    return DATA_PTR( self );
}


/*
 * Fetch the data pointer and check it for sanity.
 */
static table *get_paramtable( VALUE self )
{
    table *ptr = check_paramtable( self );

    if ( !ptr )
        rb_raise( rb_eRuntimeError, "uninitialized Apache::ParamTable" );
    return ptr;
}



/* Instance methods */

/*
 * The code below is largely code from mod_ruby/table.c hacked to accept and
 * return Apache::MultiVals instead of Strings.
 */

/*
 * clear()
 * --
 * Clear the parameter table.
 */
static VALUE paramtable_clear( VALUE self )
{
    table *tbl = get_paramtable( self );

    ap_clear_table( tbl );
    return Qnil;
}


/*
 * Convert the specified +val+ to a tainted String object and push it onto the
 * given +ary+. Called as the block of a hash iterator by parameter-fetching
 * methods. The +key+ parameter is ignored because the iteration is done over
 * all values associated with the same key -- it exists only to appease
 * ap_table_do().
 */
static int rb_ary_tainted_push( void *ary, const char *key, const char *val )
{
    rb_ary_push( *(VALUE *)ary, rb_tainted_str_new2(val) );

    return 1;
}


/*
 * get( name )
 * --
 * Fetch the parameter with the specified name, or +nil+ if no such parameter
 * exists. The +name+ is case-insensitive.
 */
static VALUE paramtable_get( VALUE self, VALUE name )
{
    table *tbl = get_paramtable( self );
    const char *key;
    VALUE res = Qnil;

    key = StringValuePtr( name );

    if ( ap_table_get(tbl, key) ) {
        VALUE ary;

        res = rb_class_new_instance(0, 0, rb_cApacheMultiVal);
        ary = rb_ivar_get( res, atargs_id );
        rb_ary_clear( ary );

        ap_table_do( rb_ary_tainted_push, &ary, tbl, key, NULL );
    }
		
    return res;
}


/*
 * set( name, val )
 * --
 * Set the parameter with the specified +name+ to the given +val+.
 */
static VALUE paramtable_set( VALUE self, VALUE name, VALUE val )
{
    table *tbl = get_paramtable( self );
    const char* key = StringValuePtr( name );
	
    if ( rb_obj_is_instance_of(val, rb_cApacheMultiVal) ) {
        VALUE ary = rb_ivar_get( val, atargs_id );
        VALUE str;
        int i;
		
        ap_table_unset( tbl, key );
        for ( i = 0; i < RARRAY(ary)->len; i++ ) {
            str = rb_check_convert_type( *(RARRAY(ary)->ptr+i), T_STRING,
                                         "String", "to_str" );
            ap_table_add( tbl, key, StringValuePtr(str) );
        }
    }
    else { 
        val = rb_check_convert_type( val, T_STRING, "String", "to_str" );
        ap_table_set( tbl, key, StringValuePtr(val) );
    }

    return val;
}



/*
 * unset( name )
 * --
 * Clear the parameter with the given +name+ from the parameter table.
 */
static VALUE paramtable_unset( VALUE self, VALUE name )
{
    table *tbl = get_paramtable( self );
    ap_table_unset( tbl, StringValuePtr(name) );
    return Qnil;
}



/*
 * each {|multival| ...}
 * --
 * Iterate over the parameters in the table, passing the specified block a key
 * and an Apache::MultiVal for each one.
 */
static VALUE paramtable_each( VALUE self )
{
    table *tbl = get_paramtable( self );
    VALUE key, val, ary, assoc, res;
    const array_header *hdrs_arr;
    table_entry *hdrs;
    int i;

    hdrs_arr = ap_table_elts( tbl );
    hdrs = (table_entry *) hdrs_arr->elts;

    res = rb_ary_new2( hdrs_arr->nelts + 1 );

    for ( i = 0; i < hdrs_arr->nelts; i++ ) {
        if ( hdrs[i].key == NULL )
            continue;
        key = rb_tainted_str_new2( hdrs[i].key );

        val = rb_class_new_instance(0, 0, rb_cApacheMultiVal);
        ary = rb_ivar_get( val, atargs_id );
        rb_ary_clear( ary );
        ap_table_do( rb_ary_tainted_push, &ary, tbl, hdrs[i].key, NULL );

        assoc = rb_assoc_new( key, val );
        rb_yield( assoc );

        rb_ary_store( res, i, assoc );
    }
    return res;
}


/*
 * keys
 * --
 * Returns an Array of the names of all the parameters in the param table.
 */
static VALUE paramtable_keys( VALUE self )
{
    table *tbl = get_paramtable( self );
    const array_header *hdrs_arr;
    table_entry *hdrs;
    int i;
    VALUE res;

    hdrs_arr = ap_table_elts(tbl);
    hdrs = (table_entry *) hdrs_arr->elts;

    res = rb_ary_new2( hdrs_arr->nelts + 1 );

    for ( i = 0; i < hdrs_arr->nelts; i++ ) {
        if ( hdrs[i].key == NULL ) continue;
        rb_ary_store( res, i, rb_tainted_str_new2(hdrs[i].key) );
    }

    return res;
}



/*
 * values
 * --
 * Returns an array of the values of the parameters (Apache::MultiVal objects).
 */
static VALUE paramtable_values( VALUE self )
{
    table *tbl = get_paramtable( self );
    VALUE key, val, ary, res;
    const array_header *hdrs_arr;
    table_entry *hdrs;
    int i;

    hdrs_arr = ap_table_elts( tbl );
    hdrs = (table_entry *) hdrs_arr->elts;

    res = rb_ary_new2( hdrs_arr->nelts + 1 );

    for ( i = 0; i < hdrs_arr->nelts; i++ ) {
        if ( hdrs[i].key == NULL )
            continue;
        key = rb_tainted_str_new2( hdrs[i].key );

        val = rb_class_new_instance(0, 0, rb_cApacheMultiVal);
        ary = rb_ivar_get( val, atargs_id );
        rb_ary_clear( ary );
        ap_table_do( rb_ary_tainted_push, &ary, tbl, hdrs[i].key, NULL );
        rb_ary_store( res, i, val );
    }

    return res;
}



/* --- End of stuff from table.c ------------------------------ */


/* Module initializer */
void rb_init_apache_paramtable()
{
    atargs_id = rb_intern( "@args" );

    /* Kluge to make Rdoc see the associations in this file */
#if FOR_RDOC_PARSER
    rb_mApache = rb_define_module( "Apache" );
    rb_cApacheRequest = rb_define_class_under( rb_mApache, "Request", rb_cObject );
    rb_cApacheTable   = rb_define_class_under( rb_mApache, "Table", rb_cObject );
    rb_cApacheMultiVal = rb_define_class_under( rb_mApache, "MultiVal", rb_cObject );
#endif

    rb_cApacheParamTable = rb_define_class_under( rb_mApache, "ParamTable", rb_cApacheTable );

    /* Remove the constructor */
    rb_undef_method ( CLASS_OF(rb_cApacheParamTable), "new" );

    /* Instance methods */
    rb_define_method( rb_cApacheParamTable, "clear", paramtable_clear, 0 );
    rb_define_method( rb_cApacheParamTable, "get", paramtable_get, 1 );
    rb_define_alias ( rb_cApacheParamTable, "[]", "get");
    rb_define_method( rb_cApacheParamTable, "set", paramtable_set, 2 );
    rb_define_alias ( rb_cApacheParamTable, "[]=", "set" );
    rb_define_method( rb_cApacheParamTable, "unset", paramtable_unset, 1 );
    rb_define_method( rb_cApacheParamTable, "each", paramtable_each, 0 );
    rb_define_method( rb_cApacheParamTable, "keys", paramtable_keys, 0 );
    rb_define_method( rb_cApacheParamTable, "values", paramtable_values, 0 );
}



/* vim: set filetype=c ts=8 sw=4 : */
