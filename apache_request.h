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

#ifndef _APACHE_REQUEST_H

#define _APACHE_REQUEST_H

#include "httpd.h"
#include "http_config.h"
#include "http_core.h"
#include "http_log.h"
#include "http_main.h"
#include "http_protocol.h"
#include "util_script.h"

typedef struct ApacheUpload ApacheUpload;

typedef struct {
    table *parms;
    ApacheUpload *upload;
    int status;
    int parsed;
    int post_max;
    int disable_uploads;
    int (*upload_hook)(void *ptr, char *buf, int len, ApacheUpload *upload);
    void *hook_data;
    char* temp_dir;
    request_rec *r;
    int nargs;
} ApacheRequest;

struct ApacheUpload {
    ApacheUpload *next;
    char *filename;
    char *name;
    char *tempname;
    table *info;
    FILE *fp;
    long size;
    ApacheRequest *req;
};

#ifndef strEQ
#define strEQ(s1,s2) (!strcmp((s1),(s2)))
#endif

#ifndef strEQN
#define strEQN(s1,s2,n) (!strncmp((s1),(s2),(n)))
#endif

#ifndef strcaseEQ
#define strcaseEQ(s1,s2) (!strcasecmp((s1),(s2)))
#endif

#ifndef strncaseEQ
#define strncaseEQ(s1,s2,n) (!strncasecmp((s1),(s2),(n)))
#endif

#define DEFAULT_TABLE_NELTS 10

#define DEFAULT_ENCTYPE "application/x-www-form-urlencoded"
#define DEFAULT_ENCTYPE_LENGTH 33

#define MULTIPART_ENCTYPE "multipart/form-data"
#define MULTIPART_ENCTYPE_LENGTH 19

#ifdef  __cplusplus
 extern "C" {
#endif 

ApacheRequest *ApacheRequest_new(request_rec *r);
int ApacheRequest_parse_multipart(ApacheRequest *req);
int ApacheRequest_parse_urlencoded(ApacheRequest *req);
char *ApacheRequest_script_name(ApacheRequest *req);
char *ApacheRequest_script_path(ApacheRequest *req);
const char *ApacheRequest_param(ApacheRequest *req, const char *key);
array_header *ApacheRequest_params(ApacheRequest *req, const char *key);
char *ApacheRequest_params_as_string(ApacheRequest *req, const char *key);
int ApacheRequest___parse(ApacheRequest *req);
#define ApacheRequest_parse(req) \
    ((req)->status = (req)->parsed ? (req)->status : ApacheRequest___parse(req)) 
table *ApacheRequest_query_params(ApacheRequest *req, ap_pool *p);
table *ApacheRequest_post_params(ApacheRequest *req, ap_pool *p);

FILE *ApacheRequest_tmpfile(ApacheRequest *req, ApacheUpload *upload);
ApacheUpload *ApacheUpload_new(ApacheRequest *req);
ApacheUpload *ApacheUpload_find(ApacheUpload *upload, char *name);

#define ApacheRequest_upload(req) \
    (((req)->parsed || (ApacheRequest_parse(req) == OK)) ? (req)->upload : NULL)

#define ApacheUpload_FILE(upload) ((upload)->fp)

#define ApacheUpload_size(upload) ((upload)->size)

#ifdef APACHE2
#define ApacheUpload_info(upload, key) \
apr_table_get((upload)->info, (key))
#else
#define ApacheUpload_info(upload, key) \
ap_table_get((upload)->info, (key))
#endif

#define ApacheUpload_type(upload) \
ApacheUpload_info((upload), "Content-Type")

#define ApacheRequest_set_post_max(req, max) ((req)->post_max = (max))
#define ApacheRequest_set_temp_dir(req, dir) ((req)->temp_dir = (dir))

char *ApacheUtil_expires(pool *p, char *time_str, int type);
#define EXPIRES_HTTP   1
#define EXPIRES_COOKIE 2
char *ApacheRequest_expires(ApacheRequest *req, char *time_str);

#ifdef __cplusplus
 }
#endif

#ifdef APACHE2
#define REQ_ERROR APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, 0, req->r
#else
#define REQ_ERROR APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, req->r
#endif

#ifdef REQDEBUG
#define REQ_DEBUG(a) (a)
#else
#define REQ_DEBUG(a)
#endif

#endif /* _APACHE_REQUEST_H */
