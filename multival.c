/*
 * multival.c - The Apache::MultiVal class
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

/*
 * This file contains Apache::MultiVal, a multi-valued variable class for Apache
 * request parameters. Instances of it are used to represent request parameters
 * in an Apache mod_ruby application. Since parameters can be multi-valued (ie.,
 * have more than one value associated with any key), a Hash (and even an
 * Apache::Table) is a poor way to represent them. You must either discard all
 * but the first value read in, or put multiple values into Arrays. If you put
 * only parameters which have multiple values into Arrays, it requires that the
 * application author always check for Array-ness for every parameter (even if
 * it's only ever expected to take one) in case someone modifies the query and
 * sends more than one. For example, if you have the following code:
 *
 *   foo = request.param['foo'].downcase
 *	
 * And someone sends a request with the following QUERY_STRING to your
 * application:
 *
 *   ?foo=bad&foo=second+value
 *
 * then your code will blithely try to call 'downcase' on the resultant Array,
 * which will result in an exception:
 *
 *   NoMethodError: undefined method `downcase' for ["bad", "second value"]:Array
 *
 * You're then forced to do something like this for each parameter:
 * 
 *   if request.param['foo'].is_a?( Array )
 *	  foo = request.param['foo'][0].downcase
 *	else
 *	  foo = request.param['foo'].downcase
 *	end
 *
 * Making *every* parameter an Array, no matter if there are multiple values or
 * not solves this somewhat, but forces you do use an index for *every*
 * parameter:
 *
 *	foo = request.params['foo'][0].downcase
 *
 * Apache::MultiVal solves this by making each parameter capable of being
 * treated like both a String *and* an Array by delegating String and Array
 * instance methods to either the first value or the Array of values,
 * respectively. The instance methods which String and Array share in common are
 * delegated to the String, except #each, which is sent to the Array.  This
 * allows the code to be kept simple: if you only ever expect a parameter to
 * have a single value, you can treat it as if it is a String:
 *
 *   foo = request.params['foo'].downcase
 *
 * and treat parameters which can have multiple values (mostly) as an Array:
 *
 *	 bars = request.params['bar'].collect {|val| val.downcase}
 *
 * For the methods that Array and String share in common, you can cast the
 * parameter to the object you wish with the normal #to_a and #to_s methods:
 *
 *	 foo = request.params['foo']
 *	 if foo.to_a.length > 1
 *	   request.log_warn( "Request had more than one 'foo' parameter: %s",
 *						    foo.to_a.inspect )
 *
 * Of course, the Array's length could be obtained with foo.nitems, too, since
 * Array#nitems isn't obscured by String's instance methods.
 *
 *
 * === Obscured Array methods
 *
 * As indicated above, some of Array's methods are obscured by those of String,
 * so you should take special note when using them to be sure you know what
 * you'll be getting. For the version of Ruby that was most recent as of this
 * writing (ruby 1.8.0 (2003-04-27)), these are:
 *
 * String::instance_methods(false) & Array::instance_methods(false) - ["each"]
 *		# ["slice!", "length", "delete", "*", "+", "empty?", 
 *			"to_s", "copy_object", "rindex", "slice", "reverse!", "[]",
 *			"concat", "[]=", "size", "<<", "reverse", "inspect", "insert",
 *			"replace", "<=>", "eql?", "==", "index", "include?", "hash"]
 *
 * 
 */


#include "mod_ruby.h"
#include "apachelib.h"

VALUE rb_cApacheMultiVal;
ID stringish, arrayish;
extern VALUE rb_load_path;

/*
 * Apache::MultiVal#initialize( *values )
 * --
 * Create a new MultiVal object with the specified +values+. The first of
 * the values given will be the value used for String operations.
 */
static VALUE
multival_init( VALUE self, VALUE args )
{
    VALUE collect;
    long len, i;

    /* Make sure there's at least one argument to work with */
    if ( RARRAY(args)->len == 0 )
        rb_ary_push( args, rb_tainted_str_new("", 0) );

    /* Stringify all arguments */
    len = RARRAY(args)->len;
    collect = rb_ary_new2(len);
    for ( i = 0; i < len; i++ ) {
        VALUE str;
		
        str = rb_str_dup( rb_obj_as_string(RARRAY(args)->ptr[i]) );
        OBJ_INFECT( str, RARRAY(args)->ptr[i] );
		
        rb_ary_push( collect, str );
    }
	
    rb_iv_set( self, "@args", collect );
    return self;
}


/*
 * Apache::MultiVal#to_s
 * --
 * Return the String part of the MultiVal.
 */
static VALUE
multival_to_s( VALUE self )
{
    VALUE args = rb_iv_get( self, "@args" );
    return rb_ary_entry( args, 0 );
}


/*
 * Apache::MultiVal#to_a
 * --
 * Return the Array part of the MultiVal.
 */
static VALUE
multival_to_a( VALUE self )
{
    return rb_iv_get( self, "@args" );
}


/*
 * Apache::MultiVal#<=>( other )
 * --
 * Comparison -- Returns -1 if the receiver is less than, 0 if it is equal to,
 * or -1 if it is greater than the specified +other+ object. Returns +nil+ if
 * the +other+ object isn't an Apache::MultiVal.
 */
static VALUE
multival_compare( VALUE self, VALUE other )
{
    /* If it's another MultiVal (or a subclass thereof), do array-wise comparison. */
    if ( rb_obj_is_kind_of(other, CLASS_OF(self)) ) {
        VALUE args = rb_iv_get( self, "@args" );
        VALUE otherval = rb_funcall( other, rb_intern("to_a"), 0 );

        return rb_funcall( args, rb_intern("<=>"), 1, otherval );
    }
	
    else {
        return Qnil;
    }
}



/*
 * Delegate the current method to the stringish version of the multival (ie.,
 * the first element of the array).
 */
static VALUE
multival_string_delegator( int argc, VALUE *argv, VALUE self )
{
    ID meth = rb_frame_last_func();
    VALUE args = rb_iv_get( self, "@args" );
	
    return rb_funcall2( RARRAY(args)->ptr[0], meth, argc, argv );
}


/*
 * Delegate the current method to the arrayish version of the multival.
 */
static VALUE
multival_array_delegator( int argc, VALUE *argv, VALUE self )
{
    ID meth = rb_frame_last_func();
    VALUE args = rb_iv_get( self, "@args" );
	
    return rb_funcall2( args, meth, argc, argv );
}


/*
 * Make a delegator method to either the @args Array or the first element of
 * @args.
 */
static VALUE
multival_make_delegator( VALUE name, ID which )
{
    if ( which == stringish ) {
        rb_define_method( rb_cApacheMultiVal, StringValuePtr(name), multival_string_delegator, -1 );
    } else {
        rb_define_method( rb_cApacheMultiVal, StringValuePtr(name), multival_array_delegator, -1 );
    }
	
    return Qtrue;
}


/* Module initializer */
void rb_init_apache_multival()
{
    VALUE dmethods;
    VALUE args[1];

    /* Kluge to make Rdoc see the associations in this file */
#if FOR_RDOC_PARSER
    rb_mApache = rb_define_module( "Apache" );
#endif

    rb_cApacheMultiVal = rb_define_class_under( rb_mApache, "MultiVal", rb_cObject );

    /* Define IDs which indicate to the delegator-creator whether it's supposed
       to delegate to the whole array or just the first element. */
    stringish = rb_intern("stringish");
    arrayish = rb_intern("arrayish");

    /* 	Since rb_load_path isn't yet set up, we have to set up our own
        delegation instead of relying on 'forwardable'. Set up stringish
        delegation first. */
    args[0] = Qfalse;
    dmethods = rb_class_instance_methods( 1, args, rb_cString );
    rb_ary_delete( dmethods, rb_str_new2("each") );
    rb_ary_delete( dmethods, rb_str_new2("[]") );
    rb_ary_delete( dmethods, rb_str_new2("[]=") );
    rb_iterate( rb_each, dmethods, multival_make_delegator, stringish );

    /* Now set up array-ish delegation */	
    dmethods = rb_class_instance_methods( 1, args, rb_cArray );
    rb_iterate( rb_each, dmethods, multival_make_delegator, arrayish );
	
    /* include Comparable */
    rb_include_module( rb_cApacheMultiVal, rb_mComparable );

    /* Instance methods */
    rb_define_method( rb_cApacheMultiVal, "initialize", multival_init, -2 );
    rb_define_method( rb_cApacheMultiVal, "to_s", multival_to_s, 0 );
    rb_define_method( rb_cApacheMultiVal, "to_str", multival_to_s, 0 );
    rb_define_alias ( rb_cApacheMultiVal, "as_string", "to_s" );
    rb_define_method( rb_cApacheMultiVal, "to_a", multival_to_a, 0 );
    rb_define_method( rb_cApacheMultiVal, "to_ary", multival_to_a, 0 );
    rb_define_alias ( rb_cApacheMultiVal, "as_array", "to_a" );
    rb_define_method( rb_cApacheMultiVal, "<=>", multival_compare, 1 );
}

/* vim: set filetype=c ts=8 sw=4 : */
