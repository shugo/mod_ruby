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

VALUE rb_cApacheError;

VALUE rb_apache_error_new(request_rec *r, error_log_data *error)
{
    if (error == NULL)
        return Qnil;
    return Data_Wrap_Struct(rb_cApacheError, NULL, NULL, error);
}

DEFINE_STRING_ATTR_READER(error_file, error_log_data, file);
DEFINE_INT_ATTR_READER(error_line, error_log_data, line);
DEFINE_INT_ATTR_READER(error_level, error_log_data, level);
DEFINE_INT_ATTR_READER(error_status, error_log_data, status);
DEFINE_STRING_ATTR_READER(error_string, error_log_data, error);

void rb_init_apache_error()
{
    rb_cApacheError = rb_define_class_under(rb_mApache, "ErrorLogItem", rb_cObject);
    rb_define_method(rb_cApacheError, "file", error_file, 0);
    rb_define_method(rb_cApacheError, "line", error_line, 0);
    rb_define_method(rb_cApacheError, "level", error_level, 0);
    rb_define_method(rb_cApacheError, "status", error_status, 0);
    rb_define_method(rb_cApacheError, "errstr", error_string, 0);
    rb_define_method(rb_cApacheError, "msg", error_string, 0);
    rb_define_method(rb_cApacheError, "string", error_string, 0);
}

/* vim: set filetype=c ts=8 sw=4 : */
