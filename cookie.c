/*
 * cookie.c - The Apache::Cookie class
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

VALUE rb_cApacheCookie;

/* Cookie option IDs */
static ID id_name, id_value, id_expires, id_domain, id_path, id_secure;
static VALUE cookie_date_format;


VALUE rb_apache_cookie_new(ApacheCookie *cookie)
{
    return Data_Wrap_Struct(rb_cApacheCookie, 0, 0, cookie);
}


/*
 * Object validity checker. Returns the data pointer.
 */
static ApacheCookie *check_cookie( VALUE self )
{
    Check_Type( self, T_DATA );
    if ( !rb_obj_is_instance_of(self, rb_cApacheCookie) ) {
        rb_raise( rb_eTypeError,
                  "wrong argument type %s (expected Apache::Cookie)",
                  rb_class2name(CLASS_OF( self )) );
    }
    return DATA_PTR( self );
}


/*
 * Fetch the data pointer and check it for sanity.
 */
static ApacheCookie *get_cookie( VALUE self )
{
    ApacheCookie *ptr = check_cookie( self );
    if ( !ptr ) rb_raise( rb_eRuntimeError, "uninitialized Apache::Cookie" );
    return ptr;
}



/* Class methods */

/*
 * allocate()
 * --
 * Allocate a new Apache::Cookie object.
 */
static VALUE cookie_s_alloc( VALUE klass )
{
    return Data_Wrap_Struct( klass, 0, 0, 0 );
}



/* Instance methods */

/*
 * name
 * --
 * Returns the name of the cookie.
 */
static VALUE cookie_name( VALUE self )
{
    ApacheCookie *cookie = get_cookie( self );
    return rb_tainted_str_new2( cookie->name );
}


/*
 * name=
 * --
 * Sets the name of the cookie.
 */
static VALUE cookie_name_eq( VALUE self, VALUE arg )
{
    ApacheCookie *cookie = get_cookie( self );
    VALUE name = rb_check_convert_type( arg, T_STRING, "String", "to_s" );

    cookie->name = StringValuePtr( name );
    return name;
}


/*
 * value
 * --
 * Returns the first value stored in the cookie as a String.
 */
static VALUE cookie_value( VALUE self )
{
    ApacheCookie *cookie = get_cookie( self );
    VALUE item;

    item = rb_tainted_str_new2( ApacheCookieFetch(cookie, 0) );
    return item;
}


/*
 * values
 * --
 * Returns all the values stored in the cookie as an Array.
 */
static VALUE cookie_values( VALUE self )
{
    ApacheCookie *cookie = get_cookie( self );
    VALUE items = rb_ary_new();
    int i;

    for ( i = 0; i < ApacheCookieItems(cookie); i++ ) {
        VALUE val = rb_tainted_str_new2( ApacheCookieFetch(cookie, i) );
        rb_ary_push( items, val );
    }

    return items;
	
}


/*
 * Iterator: stringify the +val+ and push it onto the given +ary+.
 */
static VALUE cookie_stringify_push( VALUE val, VALUE ary )
{
    VALUE sval = rb_check_convert_type( val, T_STRING, "String", "to_s" );
    rb_ary_push( ary, sval );
    return ary;
}

    
/*
 * value=( newval )
 * --
 * Sets the value of the cookie to +newval+. If the +newval+ responds to
 * #each, it will be called, and the iterated value will be added by calling
 * ##to_s on it.  Otherwise, just the given +newval+ object will be
 * turned into a string via #to_s.
 */
static VALUE cookie_value_eq( VALUE self, VALUE newval )
{
    ApacheCookie *cookie = get_cookie( self );
    VALUE items = rb_ary_new();
    int i;

    if ( rb_respond_to(newval, rb_intern("each")) ) {
        rb_iterate( rb_each, newval, cookie_stringify_push, items );
    }
    else {
        rb_ary_push( items,
                     rb_check_convert_type(newval, T_STRING, "String", "to_s") );
    }

    ApacheCookieItems(cookie) = 0;
    for ( i = 0; i < RARRAY(items)->len; i++ ) {
        ApacheCookieAddLen( cookie,
                            RSTRING_PTR(*(RARRAY(items)->ptr + i)),
                            RSTRING_LEN(*(RARRAY(items)->ptr + i)) );
    }

    return items;
}


/*
 * domain
 * --
 * Returns the domain of the cookie.
 */
static VALUE cookie_domain( VALUE self )
{
    ApacheCookie *cookie = get_cookie( self );
    if ( cookie->domain == NULL ) 
        return Qnil;
    else
        return rb_tainted_str_new2( cookie->domain );
}

    
/*
 * domain=( newdomain )
 * --
 * Sets the domain of the cookie to the specified +newdomain+.
 */
static VALUE cookie_domain_eq( VALUE self, VALUE newdomain )
{
    ApacheCookie *cookie = get_cookie( self );
    cookie->domain = StringValuePtr( newdomain );
    return newdomain;
}


/*
 * path
 * --
 * Returns the path of the cookie.
 */
static VALUE cookie_path( VALUE self )
{
    ApacheCookie *cookie = get_cookie( self );
    return rb_tainted_str_new2( cookie->path );
}

    
/*
 * path=( newpath )
 * --
 * Sets the path of the cookie to +newpath+.
 */
static VALUE cookie_path_eq( VALUE self, VALUE newpath )
{
    ApacheCookie *cookie = get_cookie( self );
    cookie->path = StringValuePtr( newpath );
    return newpath;
}


/*
 * expires
 * --
 * Returns the value of the 'expires' field.
 */
static VALUE cookie_expires( VALUE self )
{
    ApacheCookie *cookie = get_cookie( self );
    if ( cookie->expires == NULL )
        return Qnil;
    else
        return rb_tainted_str_new2( cookie->expires );
}


/*
 * expires=( expiration )
 * --
 * Sets the 'expires' field. The +expiration+ object can be either a String or a
 * Time object. If it is a string, any of the following formats are permitted:
 *
 *   +30s::                                30 seconds from now 
 *   +10m::                                ten minutes from now 
 *   +1h::                                 one hour from now 
 *   -1d::                                 yesterday (i.e. "ASAP!") 
 *   now::                                 immediately 
 *   +3M::                                 in three months 
 *   +10y::                                in ten years time 
 *   Thursday, 25-Apr-1999 00:40:33 GMT::  at the indicated time & date
 */
static VALUE cookie_expires_eq( VALUE self, VALUE expiration )
{
    ApacheCookie *cookie = get_cookie( self );
    
    if ( rb_obj_is_kind_of(expiration, rb_cTime) ) {
        expiration = rb_funcall( expiration, rb_intern("gmtime"), 0 );
        expiration = rb_funcall( expiration, rb_intern("strftime"),
                                 1, cookie_date_format );
    }

    ApacheCookie_expires( cookie, StringValuePtr(expiration) );
    return expiration;
}


/*
 * secure?
 * --
 * Returns +true+ if the cookie's 'secure' flag is set.
 */
static VALUE cookie_secure( VALUE self )
{
    ApacheCookie *cookie = get_cookie( self );
    return cookie->secure ? Qtrue : Qfalse;
}

    
/*
 * secure=( bool )
 * --
 * Sets the 'secure' flag of the cookie to either +true+ or +false+.
 */
static VALUE cookie_secure_eq( VALUE self, VALUE val )
{
    ApacheCookie *cookie = get_cookie( self );
    cookie->secure = RTEST(val) ? 1 : 0;
    return cookie->secure ? Qtrue : Qfalse;
}


/*
 * Iterator: set attributes one at a time.
 */
static VALUE cookie_set_attr( VALUE pair, VALUE self )
{
    ID attr;
    VALUE val;

    /* Make sure the argument's an Array. */
    Check_Type( pair, T_ARRAY );
    if ( !RARRAY(pair)->len == 2 )
        rb_raise( rb_eArgError, "Expected an array of 2 elements, not %d",
                  (int) RARRAY(pair)->len );

    /* Pick the attr name (converted to an ID) and value out of the iterator
       pair. */
    attr = rb_to_id( *(RARRAY(pair)->ptr) );
    val = *(RARRAY(pair)->ptr + 1);

    /* Set the option specified. */
    if ( attr == id_name ) {
        cookie_name_eq( self, val );
    }
    else if ( attr == id_value ) {
        cookie_value_eq( self, val );
    }
    else if ( attr == id_expires ) {
        cookie_expires_eq( self, val );
    }
    else if ( attr == id_domain ) {
        cookie_domain_eq( self, val );
    }
    else if ( attr == id_path ) {
        cookie_path_eq( self, val );
    }
    else if ( attr == id_secure ) {
        cookie_secure_eq( self, val );
    }
    else {
	VALUE s = rb_inspect(*( RARRAY(pair)->ptr ));
        rb_raise( rb_eArgError, "Unknown attribute %s",
                  StringValuePtr(s) );
    }

    return val;
}


/*
 * initialize( request, options={} )
 * --
 * Returns a new Apache::Cookie for the specified +request+ (an Apache::Request
 * object). The optional +options+ Hash may be used to initialize the cookie's
 * attributes. The following keys are supported:
 *
 *   :name
 *	    Sets the name field to the given value. 
 *	 :value
 *	    Adds the value to values field. 
 *	 :expires
 *	    Sets the expires field to the calculated date String or Time object. See
 *	    Apache::Cookie#expires for a listing of format options. The default is
 *	    +nil+.
 *	 :domain
 *	    Sets the domain field to the given value. The default is nil. 
 *	 :path
 *	    Sets the path field to the given value. The default path is derived from
 *	    the requested uri.
 *	 :secure
 *	    Sets the secure field to +true+ or +false+.
 */
static VALUE cookie_init( int argc, VALUE *argv, VALUE self )
{
    if ( !check_cookie(self) ) {
        ApacheCookie *ptr;
        request_rec *r;
        VALUE request, attrs;
    
        /* Read arguments, making sure the second one is a Hash if given */
        if ( rb_scan_args(argc, argv, "11", &request, &attrs) == 2 ) {
            Check_Type( attrs, T_HASH );
        }

        /* Check request argument to be sure it's an Apache::Request */
        if ( !rb_obj_is_instance_of(request, rb_cApacheRequest) )
            rb_raise( rb_eTypeError, "wrong argument type %s: expected a %s",
                      rb_class2name(CLASS_OF(request)),
                      rb_class2name(rb_cApacheRequest) );

        /* Fetch the request_rec* from the request object and create the new
           ApacheCookie* */
        r = rb_get_request_rec( request );
        DATA_PTR( self ) = ptr = ApacheCookie_new( r, NULL );

        /* Set attributes if there were any */
        if ( RTEST(attrs) )
            rb_iterate( rb_each, attrs, cookie_set_attr, self );
    }
    else {
        rb_raise( rb_eArgError, "Cannot re-initialize Apache::Cookie object." );
    }
    
    return self;
}

    
/*
 * bake
 * --
 * Add the cookie to the output headers of the request to which it belongs.
 */
static VALUE cookie_bake( VALUE self )
{
    ApacheCookie *cookie = get_cookie( self );
    ApacheCookie_bake(cookie);
    return Qtrue;
}


/*
 * to_s
 * --
 * Returns the cookie as a String.
 */
static VALUE cookie_to_s( VALUE self )
{
    ApacheCookie *cookie = get_cookie( self );
    return rb_tainted_str_new2( ApacheCookie_as_string(cookie) );
}





/* --- End of stuff from table.c ------------------------------ */


#ifndef HAVE_RB_DEFINE_ALLOC_FUNC
static VALUE cookie_s_new(int argc, VALUE *argv, VALUE klass)
{
    VALUE obj = cookie_s_alloc(klass);
    cookie_init(argc, argv, obj);
    return obj;
}
#endif


/* Module initializer */
void rb_init_apache_cookie()
{
    id_name		= rb_intern( "name" );
    id_value	= rb_intern( "value" );
    id_expires	= rb_intern( "expires" );
    id_domain	= rb_intern( "domain" );
    id_path		= rb_intern( "path" );
    id_secure	= rb_intern( "secure" );

    cookie_date_format = rb_str_new2( "%A, %d-%b-%Y %H:%M:%S GMT" );

    /* Kluge to make Rdoc see the associations in this file */
#if FOR_RDOC_PARSER
    rb_mApache = rb_define_module( "Apache" );
#endif

    rb_cApacheCookie = rb_define_class_under( rb_mApache, "Cookie", rb_cObject );

    /* Constants */
    rb_obj_freeze( cookie_date_format );
    rb_define_const( rb_cApacheCookie, "DateFormat", cookie_date_format );


    /* Allocator */
#ifdef HAVE_RB_DEFINE_ALLOC_FUNC
    rb_define_alloc_func( rb_cApacheCookie, cookie_s_alloc );
#else
    rb_define_singleton_method( rb_cApacheCookie, "allocate", cookie_s_alloc, 0 );
    rb_define_singleton_method(rb_cApacheCookie, "new", cookie_s_new, -1);
#endif

    /* Instance methods */
    rb_define_method( rb_cApacheCookie, "initialize", cookie_init, -1 );

    rb_define_method( rb_cApacheCookie, "name", cookie_name, 0 );
    rb_define_method( rb_cApacheCookie, "name=", cookie_name_eq, 1 );
    rb_define_method( rb_cApacheCookie, "value", cookie_value, 0 );
    rb_define_method( rb_cApacheCookie, "values", cookie_values, 0 );
    rb_define_method( rb_cApacheCookie, "value=", cookie_value_eq, 1 );
    rb_define_method( rb_cApacheCookie, "domain", cookie_domain, 0 );
    rb_define_method( rb_cApacheCookie, "domain=", cookie_domain_eq, 1 );
    rb_define_method( rb_cApacheCookie, "path", cookie_path, 0 );
    rb_define_method( rb_cApacheCookie, "path=", cookie_path_eq, 1 );
    rb_define_method( rb_cApacheCookie, "expires", cookie_expires, 0 );
    rb_define_method( rb_cApacheCookie, "expires=", cookie_expires_eq, 1 );
    rb_define_method( rb_cApacheCookie, "secure", cookie_secure, 0 );
    rb_define_method( rb_cApacheCookie, "secure=", cookie_secure_eq, 1 );

    rb_define_method( rb_cApacheCookie, "bake", cookie_bake, 0 );
    rb_define_method( rb_cApacheCookie, "to_s", cookie_to_s, 0 );
}

/* vim: set filetype=c ts=8 sw=4 : */
