/* Copyright 1999-2004 The Apache Software Foundation

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   	http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
/* Modified by Shugo Maeda for mod_ruby. */


#ifndef _APACHE_COOKIE_H
#define _APACHE_COOKIE_H

#include "apache_request.h"

typedef array_header ApacheCookieJar;

typedef struct {
    request_rec *r;
    char *name;
    array_header *values;
    char *domain;
    char *expires;
    char *path;
    int secure;
} ApacheCookie;

#ifdef  __cplusplus
 extern "C" {
#endif 

#define ApacheCookieJarItems(arr) arr->nelts

#define ApacheCookieJarFetch(arr,i) \
((ApacheCookie *)(((ApacheCookie **)arr->elts)[i]))

#define ApacheCookieJarAdd(arr,c) \
*(ApacheCookie **)ap_push_array(arr) = c

#define ApacheCookieItems(c) c->values->nelts

#define ApacheCookieFetch(c,i) \
((char *)(((char **)c->values->elts)[i]))

#define ApacheCookieAddn(c,val) \
    if(val) *(char **)ap_push_array(c->values) = (char *)val

#define ApacheCookieAdd(c,val) \
    ApacheCookieAddn(c, ap_pstrdup(c->r->pool, val))

#define ApacheCookieAddLen(c,val,len) \
    ApacheCookieAddn(c, ap_pstrndup(c->r->pool, val, len))

ApacheCookie *ApacheCookie_new(request_rec *r, ...);
ApacheCookieJar *ApacheCookie_parse(request_rec *r, const char *data);
char *ApacheCookie_as_string(ApacheCookie *c);
char *ApacheCookie_attr(ApacheCookie *c, char *key, char *val);
char *ApacheCookie_expires(ApacheCookie *c, char *time_str);
void ApacheCookie_bake(ApacheCookie *c);

#ifdef __cplusplus
 }
#endif

#ifdef APACHE2
#define APC_ERROR APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, 0, c->r
#else
#define APC_ERROR APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, c->r
#endif

#endif
