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

VALUE rb_cApacheUri;

VALUE rb_apache_uri_new(uri_components *uri)
{
    if (uri == NULL)
        return Qnil;
    return Data_Wrap_Struct(rb_cApacheUri, NULL, NULL, uri);
}

DEFINE_STRING_ATTR_READER(uri_scheme, uri_components, scheme);
DEFINE_STRING_ATTR_READER(uri_hostinfo, uri_components, hostinfo);
DEFINE_STRING_ATTR_READER(uri_user, uri_components, user);
DEFINE_STRING_ATTR_READER(uri_password, uri_components, password);
DEFINE_STRING_ATTR_READER(uri_hostname, uri_components, hostname);
DEFINE_STRING_ATTR_READER(uri_port_str, uri_components, port_str);
DEFINE_STRING_ATTR_READER(uri_path, uri_components, path);
DEFINE_STRING_ATTR_READER(uri_query, uri_components, query);
DEFINE_STRING_ATTR_READER(uri_fragment, uri_components, fragment);
DEFINE_INT_ATTR_READER(uri_port, uri_components, port);
DEFINE_BOOL_ATTR_READER(uri_is_initialized, uri_components, is_initialized);
DEFINE_BOOL_ATTR_READER(uri_dns_looked_up, uri_components, dns_looked_up);
DEFINE_BOOL_ATTR_READER(uri_dns_resolved, uri_components, dns_resolved);

/* 
 * struct hostent *hostent; defined but unused by apr-util or apache at present
 */

void rb_init_apache_uri()
{
    rb_cApacheUri = rb_define_class_under(rb_mApache, "Uri", rb_cObject);
    rb_define_method(rb_cApacheUri, "scheme", uri_scheme, 0);
    rb_define_method(rb_cApacheUri, "hostinfo", uri_hostinfo, 0);
    rb_define_method(rb_cApacheUri, "user", uri_user, 0);
    rb_define_method(rb_cApacheUri, "password", uri_password, 0);
    rb_define_method(rb_cApacheUri, "hostname", uri_hostname, 0);
    rb_define_method(rb_cApacheUri, "port_str", uri_port_str, 0);
    rb_define_method(rb_cApacheUri, "path", uri_path, 0);
    rb_define_method(rb_cApacheUri, "query", uri_query, 0);
    rb_define_method(rb_cApacheUri, "fragment", uri_fragment, 0);
    rb_define_method(rb_cApacheUri, "port", uri_port, 0);
    rb_define_method(rb_cApacheUri, "is_initialized", uri_is_initialized, 0);
    rb_define_method(rb_cApacheUri, "initialized?", uri_is_initialized, 0);
    rb_define_method(rb_cApacheUri, "dns_looked_up?", uri_dns_looked_up, 0);
    rb_define_method(rb_cApacheUri, "dns_resolved?", uri_dns_resolved, 0);
}

/* vim: set filetype=c ts=8 sw=4 : */
