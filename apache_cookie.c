/* Copyright 2000-2004 The Apache Software Foundation

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

#include "mod_ruby.h"

char *ApacheCookie_expires(ApacheCookie *c, char *time_str)
{
    char *expires;

    expires = ApacheUtil_expires(c->r->pool, time_str, EXPIRES_COOKIE);
    if (expires)
	c->expires = expires;

    return c->expires;
}

#define cookie_get_set(thing,val) \
    retval = thing; \
    if(val) thing = ap_pstrdup(c->r->pool, val)

char *ApacheCookie_attr(ApacheCookie *c, char *key, char *val)
{
    char *retval = NULL;
    int ix = key[0] == '-' ? 1 : 0;

    switch (key[ix]) {
    case 'n':
	cookie_get_set(c->name, val);
	break;
    case 'v':
	ApacheCookieAdd(c, val);
	break;
    case 'e':
	retval = ApacheCookie_expires(c, val);
	break;
    case 'd':
	cookie_get_set(c->domain, val);
	break;
    case 'p':
	cookie_get_set(c->path, val);
	break;
    case 's':
	if(val) {
	    c->secure =
		!strcaseEQ(val, "off") &&
		!strcaseEQ(val, "0");
	}
	retval = c->secure ? "on" : "";
	break;
    default:
	ap_log_rerror(APC_ERROR,
		      "[libapreq] unknown cookie pair: `%s' => `%s'", key, val);
    };

    return retval;
}

ApacheCookie *ApacheCookie_new(request_rec *r, ...)
{
    va_list args;
    ApacheRequest req;
    ApacheCookie *c =
	ap_pcalloc(r->pool, sizeof(ApacheCookie));

    req.r = r;
    c->r = r;
    c->values = ap_make_array(r->pool, 1, sizeof(char *));
    c->secure = 0;
    c->name = c->expires = NULL;

    c->domain = NULL;
    c->path = ApacheRequest_script_path(&req);

    va_start(args, r);
    for(;;) {
	char *key, *val;
	key = va_arg(args, char *);
	if (key == NULL)
	    break;

	val = va_arg(args, char *);
	(void)ApacheCookie_attr(c, key, val);
    }
    va_end(args);

    return c;
}

ApacheCookieJar *ApacheCookie_parse(request_rec *r, const char *data)
{
    const char *pair;
    ApacheCookieJar *retval =
	ap_make_array(r->pool, 1, sizeof(ApacheCookie *));

    if (!data)
	if (!(data = ap_table_get(r->headers_in, "Cookie")))
	    return retval;

    while (*data && (pair = ap_getword(r->pool, &data, ';'))) {
	const char *key, *val;
	ApacheCookie *c;

	while (ap_isspace(*data))
	    ++data;

	key = ap_getword(r->pool, &pair, '=');
	ap_unescape_url((char *)key);
	c = ApacheCookie_new(r, "-name", key, NULL);

	if (c->values)
	    c->values->nelts = 0;
	else
	    c->values = ap_make_array(r->pool, 4, sizeof(char *));

	if (!*pair)
	    ApacheCookieAdd(c, "");


	while (*pair && (val = ap_getword_nulls(r->pool, &pair, '&'))) {
	    ap_unescape_url((char *)val);
#ifdef DEBUG
	    ap_log_rerror(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, r, 
                          "[apache_cookie] added (%s)", val);
#endif
	    ApacheCookieAdd(c, val);
	}
	ApacheCookieJarAdd(retval, c);
    }

    return retval;
}

#define cookie_push_arr(arr, val) \
    *(char **)ap_push_array(arr) = (char *)val

#define cookie_push_named(arr, name, val) \
    if(val && strlen(val) > 0) { \
        cookie_push_arr(arr, ap_pstrcat(p, name, "=", val, NULL)); \
    }

static char * escape_url(pool *p, char *val)
{
  char *result = ap_os_escape_path(p, val?val:"", 1);
  char *end = result + strlen(result);
  char *seek;

  /* touchup result to ensure that special chars are escaped */
  for ( seek = end-1; seek >= result; --seek) {
    char *ptr, *replacement;

    switch (*seek) {

    case '&':
	replacement = "%26";
	break;
    case '=':
	replacement = "%3d";
	break;
    /* additional cases here */

    default:
	continue; /* next for() */
    }

    for (ptr = end; ptr > seek; --ptr)
        ptr[2] = ptr[0];

    strncpy(seek, replacement, 3);
    end += 2;
  }

  return(result);
}

char *ApacheCookie_as_string(ApacheCookie *c)
{
    array_header *values;
    pool *p = c->r->pool;
    char *cookie, *retval;
    int i;

    if (!c->name)
	return "";

    values = ap_make_array(p, 6, sizeof(char *));
    cookie_push_named(values, "domain",  c->domain);
    cookie_push_named(values, "path",    c->path);
    cookie_push_named(values, "expires", c->expires);
    if (c->secure) {
	cookie_push_arr(values, "secure");
    }

    cookie = ap_pstrcat(p, escape_url(p, c->name), "=", NULL);
    for (i=0; i<c->values->nelts; i++) {
	cookie = ap_pstrcat(p, cookie,
			    escape_url(p, ((char**)c->values->elts)[i]),
			    (i < (c->values->nelts -1) ? "&" : NULL),
			    NULL);
    }

    retval = cookie;
    for (i=0; i<values->nelts; i++) {
	retval = ap_pstrcat(p, retval, "; ",
			    ((char**)values->elts)[i], NULL);
    }

    return retval;
}

void ApacheCookie_bake(ApacheCookie *c)
{
    ap_table_add(c->r->err_headers_out, "Set-Cookie",
		 ApacheCookie_as_string(c));
}
