/*
 * upload.c - Apache::Upload class for the Ruby binding for libapreq.
 * $Id: upload.c,v 1.7 2003/08/07 03:48:11 shugo Exp $
 *
 * Author: Michael Granger <ged@FaerieMUD.org>
 * Copyright (c) 2003 The FaerieMUD Consortium. All rights reserved.
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

VALUE rb_cApacheUpload;


VALUE rb_apache_upload_new(ApacheUpload *upload)
{
    return Data_Wrap_Struct(rb_cApacheUpload, NULL, NULL, upload);
}


/*
 * Object validity checker. Returns the data pointer.
 */
static ApacheUpload *check_upload( VALUE self )
{
    Check_Type( self, T_DATA );

    if ( !rb_obj_is_instance_of(self, rb_cApacheUpload) ) {
        rb_raise( rb_eTypeError, "wrong argument type %s (expected Apache::Upload)",
                  rb_class2name(CLASS_OF( self )) );
    }

    return DATA_PTR( self );
}


/*
 * Fetch the data pointer and check it for sanity.
 */
static ApacheUpload *get_upload( VALUE self )
{
    ApacheUpload *ptr = check_upload( self );

    if ( !ptr )
        rb_raise( rb_eRuntimeError, "uninitialized Apache::Upload" );

    return ptr;
}



/* Instance methods */

/*
 * name
 * --
 * Returns the name of the file.
 */
static VALUE upload_name( VALUE self )
{
    ApacheUpload *upload = get_upload( self );
    return rb_tainted_str_new2( upload->name );
}


/*
 * filename
 * --
 * Returns the name of the uploaded file as reported by the client.
 */
static VALUE upload_filename( self )
    VALUE self;
{
    ApacheUpload *upload = get_upload( self );
    return rb_tainted_str_new2( upload->filename );
}


#if RUBY_VERSION_CODE < 180
static void upload_io_finalize(OpenFile *fptr)
#else
static void upload_io_finalize(OpenFile *fptr, int noraise)
#endif
{
    fptr->f = NULL;
    fptr->f2 = NULL;
}

static VALUE io_new(FILE *fp)
{
    OpenFile *fptr;

    NEWOBJ(io, struct RFile);
    OBJSETUP(io, rb_cIO, T_FILE);
    MakeOpenFile((VALUE) io, fptr);
    fptr->f = fp;
    fptr->mode = FMODE_WRITABLE | FMODE_READABLE;
    fptr->finalize = upload_io_finalize;
    return (VALUE) io;
}

/*
 * io
 * --
 * Returns an IO object opened to the temporary file associated with
 * the upload.
 */
static VALUE upload_io( VALUE self )
{
    ApacheUpload *upload = get_upload(self);

    if (!upload->fp && !ApacheRequest_tmpfile(upload->req, upload))
        rb_raise( rb_eApacheRequestError, "can't open temporary file" );
    return io_new(upload->fp);
}


/*
 * tempname
 * --
 * Returns the name of the temporary upload file on the server.
 */
static VALUE upload_tempname( VALUE self )
{
    ApacheUpload *upload = get_upload( self );
    return upload->tempname ? rb_tainted_str_new2(upload->tempname) : Qnil;
}


/*
 * size
 * --
 * Return the size of the uploaded file in bytes.
 */
static VALUE upload_size( VALUE self )
{
    ApacheUpload *upload = get_upload( self );
    return LONG2NUM( upload->size );
}


/*
 * info
 * --
 * Return the header information for the uploaded file as an Apache::Table
 * object.
 */
static VALUE upload_info( VALUE self )
{
    ApacheUpload *upload = get_upload( self );
    return rb_apache_table_new( upload->info );
}


/*
 * type
 * --
 * Returns the file's MIME content type. This is a shortcut for accessing the
 * uploaded file's 'Content-Type' header (ie.,
 * <tt>req.info['content-type']</tt>).
 */
static VALUE upload_type( VALUE self )
{
    ApacheUpload *upload = get_upload( self );
    return rb_tainted_str_new2( ApacheUpload_type(upload) );
}



/* Module initializer */
void rb_init_apache_upload()
{
    static char
        rcsid[]		= "$Id: upload.c,v 1.7 2003/08/07 03:48:11 shugo Exp $",
        revision[]	= "$Revision: 1.7 $";

    VALUE vstr		= rb_str_new( (revision+11), strlen(revision) - 11 - 2 );

    /* Kluge to make Rdoc see the associated classes/modules */
#if FOR_RDOC_PARSER
    rb_mApache = rb_define_module( "Apache" );
#endif

    rb_cApacheUpload = rb_define_class_under( rb_mApache, "Upload", rb_cObject );

    /* Constants */
    rb_obj_freeze( vstr );
    rb_define_const( rb_cApacheUpload, "Version", vstr );
    vstr = rb_str_new2( rcsid );
    rb_obj_freeze( vstr );
    rb_define_const( rb_cApacheUpload, "Rcsid", vstr );

    /* Remove the constructor, as the class requires an ApacheUpload * */
    rb_undef_method( CLASS_OF(rb_cApacheUpload), "new" );

    /* Instance methods */
    rb_define_method( rb_cApacheUpload, "name",	upload_name, 0 );
    rb_define_method( rb_cApacheUpload, "filename", upload_filename, 0 );
    rb_define_method( rb_cApacheUpload, "io", upload_io, 0 );
    rb_define_alias ( rb_cApacheUpload, "fp", "io" );
    rb_define_method( rb_cApacheUpload, "tempname", upload_tempname, 0 );
    rb_define_method( rb_cApacheUpload, "size", upload_size, 0 );
    rb_define_method( rb_cApacheUpload, "info", upload_info, 0 );
    rb_define_method( rb_cApacheUpload, "type", upload_type, 0 );
}
