/*
 * $Id$
 * Copyright (C) 2000  ZetaBITS, Inc.
 * Copyright (C) 2000  Information-technology Promotion Agency, Japan
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

#define CORE_PRIVATE
#include "mod_ruby.h"
#include "ruby_config.h"

static int default_safe_level = MR_DEFAULT_SAFE_LEVEL;

#define push_handler(p, handler, arg) { \
    if ((handler) == NULL) \
	(handler) = ap_make_array(p, 1, sizeof(char*)); \
    *(char **) ap_push_array(handler) = arg; \
}

void *ruby_create_server_config(pool *p, server_rec *s)
{
    ruby_server_config *conf =
	(ruby_server_config *) ap_pcalloc(p, sizeof(ruby_server_config));

    conf->load_path = ap_make_array(p, 1, sizeof(char*));
    conf->env = ap_make_table(p, 1);
    conf->timeout = MR_DEFAULT_TIMEOUT;
    conf->restrict_directives = MR_DEFAULT_RESTRICT_DIRECTIVES;
    conf->ruby_child_init_handler = NULL;
    return conf;
}

static array_header *merge_handlers(pool *p,
				    array_header *base,
				    array_header *add)
{
    if (base == NULL)
	return add;
    if (add == NULL)
	return base;
    return ap_append_arrays(p, add, base);
}

void *ruby_merge_server_config(pool *p, void *basev, void *addv)
{
    ruby_server_config *new =
	(ruby_server_config *) ap_pcalloc(p, sizeof(ruby_server_config));
    ruby_server_config *base = (ruby_server_config *) basev;
    ruby_server_config *add = (ruby_server_config *) addv;

    if (add->load_path == NULL) {
	new->load_path = base->load_path;
    }
    else if (base->load_path == NULL) {
	new->load_path = add->load_path;
    }
    else {
	new->load_path = ap_append_arrays(p, base->load_path, add->load_path);
    }
    new->env = ap_overlay_tables(p, add->env, base->env);
    new->timeout = add->timeout ? add->timeout : base->timeout;
    new->restrict_directives = add->restrict_directives ?
	add->restrict_directives : base->restrict_directives;
    new->ruby_child_init_handler =
	merge_handlers(p, base->ruby_child_init_handler,
		       add->ruby_child_init_handler);
    return (void *) new;
}

void *ruby_create_dir_config(pool *p, char *dirname)
{
    ruby_dir_config *conf =
	(ruby_dir_config *) ap_palloc(p, sizeof (ruby_dir_config));

    conf->kcode = NULL;
    conf->env = ap_make_table(p, 5); 
    conf->safe_level = default_safe_level;
    conf->output_mode = MR_OUTPUT_DEFAULT;
    conf->load_path = NULL;
    conf->ruby_handler = NULL;
    conf->ruby_trans_handler = NULL;
    conf->ruby_authen_handler = NULL;
    conf->ruby_authz_handler = NULL;
    conf->ruby_access_handler = NULL;
    conf->ruby_type_handler = NULL;
    conf->ruby_fixup_handler = NULL;
    conf->ruby_log_handler = NULL;
    conf->ruby_header_parser_handler = NULL;
    conf->ruby_post_read_request_handler = NULL;
    conf->ruby_init_handler = NULL;
    conf->ruby_cleanup_handler = NULL;
    return conf;
}

void *ruby_merge_dir_config(pool *p, void *basev, void *addv)
{
    ruby_dir_config *new =
	(ruby_dir_config *) ap_pcalloc(p, sizeof(ruby_dir_config));
    ruby_dir_config *base = (ruby_dir_config *) basev;
    ruby_dir_config *add = (ruby_dir_config *) addv;

    new->kcode = add->kcode ? add->kcode : base->kcode;
    new->env = ap_overlay_tables(p, add->env, base->env);
    if (add->safe_level >= base->safe_level) {
	new->safe_level = add->safe_level;
    }
    else {
	fprintf(stderr, "mod_ruby: can't decrease RubySafeLevel\n");
	new->safe_level = base->safe_level;
    }
    new->output_mode = add->output_mode ? add->output_mode : base->output_mode;

    if (add->load_path == NULL) {
	new->load_path = base->load_path;
    }
    else if (base->load_path == NULL) {
	new->load_path = add->load_path;
    }
    else {
	new->load_path = ap_append_arrays(p, base->load_path, add->load_path);
    }

    new->ruby_handler =
	merge_handlers(p, base->ruby_handler, add->ruby_handler);
    new->ruby_trans_handler =
	merge_handlers(p, base->ruby_trans_handler, add->ruby_trans_handler);
    new->ruby_authen_handler =
	merge_handlers(p, base->ruby_authen_handler, add->ruby_authen_handler);
    new->ruby_authz_handler =
	merge_handlers(p, base->ruby_authz_handler, add->ruby_authz_handler);
    new->ruby_access_handler =
	merge_handlers(p, base->ruby_access_handler, add->ruby_access_handler);
    new->ruby_type_handler =
	merge_handlers(p, base->ruby_type_handler, add->ruby_type_handler);
    new->ruby_fixup_handler =
	merge_handlers(p, base->ruby_fixup_handler, add->ruby_fixup_handler);
    new->ruby_log_handler =
	merge_handlers(p, base->ruby_log_handler, add->ruby_log_handler);
    new->ruby_header_parser_handler =
	merge_handlers(p, base->ruby_header_parser_handler,
		       add->ruby_header_parser_handler);
    new->ruby_post_read_request_handler =
	merge_handlers(p, base->ruby_post_read_request_handler,
		       add->ruby_post_read_request_handler);
    new->ruby_init_handler =
	merge_handlers(p, base->ruby_init_handler, add->ruby_init_handler);
    new->ruby_cleanup_handler =
	merge_handlers(p, base->ruby_cleanup_handler, add->ruby_cleanup_handler);
    return (void *) new;
}

static int is_restrict_directives(server_rec *server)
{
    ruby_server_config *sconf = get_server_config(server);

    return sconf->restrict_directives;
}

static int is_from_htaccess(cmd_parms *cmd, ruby_dir_config *conf)
{
    core_server_config *sconf;
    int access_name_len;
    int config_file_path_len;

    if (cmd->path == NULL || conf == NULL) return 0;
    sconf = ap_get_module_config(cmd->server->module_config, &core_module);

    access_name_len = strlen(sconf->access_name);
    if (cmd->config_file == NULL) return 0;
    config_file_path_len = strlen(cmd->config_file->name);
    if (config_file_path_len < access_name_len) return 0;
    if (strcmp(cmd->config_file->name + config_file_path_len - access_name_len,
	       sconf->access_name) != 0)
	return 0;

    return 1;
}

#define check_restrict_directives(cmd, dconf) { \
    if (is_restrict_directives((cmd)->server) && \
	is_from_htaccess((cmd), (dconf))) { \
	return ap_psprintf(cmd->pool, \
			   "RubyRestrictDirectives is enabled, " \
			   "%s is not available in .htaccess", \
			   cmd->cmd->name); \
    } \
}

const char *ruby_cmd_kanji_code(cmd_parms *cmd, ruby_dir_config *conf,
				char *arg)
{
    check_restrict_directives(cmd, conf)
    conf->kcode = ap_pstrdup(cmd->pool, arg);
    return NULL;
}

const char *ruby_cmd_add_path(cmd_parms *cmd, ruby_dir_config *dconf, char *arg)
{
    ruby_server_config *sconf;

    check_restrict_directives(cmd, dconf)
    if (cmd->path == NULL) {
	sconf = get_server_config(cmd->server);
	*(char **) ap_push_array(sconf->load_path) = arg;
    }
    else {
	if (dconf->load_path == NULL)
	    dconf->load_path = ap_make_array(cmd->pool, 1, sizeof(char*));
	*(char **) ap_push_array(dconf->load_path) = arg;
    }
    return NULL;
}

typedef struct require_internal_arg {
    char *filename;
    server_rec *server;
    ruby_server_config *sconf;
    ruby_dir_config *dconf;
    
} require_internal_arg_t;

static void *ruby_require_internal(require_internal_arg_t *arg)
{
    char *filename = arg->filename;
    server_rec *server = arg->server;
    ruby_server_config *sconf = arg->sconf;
    ruby_dir_config *dconf = arg->dconf;
    VALUE fname, exit_status;
    int state;

    mod_ruby_setup_loadpath(sconf, dconf);
    fname = rb_str_new2(filename);
    rb_protect_funcall(Qnil, rb_intern("require"), &state, 1, fname);
    if (state == TAG_RAISE &&
	rb_obj_is_kind_of(ruby_errinfo, rb_eSystemExit)) {
	exit_status = rb_iv_get(ruby_errinfo, "status");
	exit(NUM2INT(exit_status));
    }
    if (state) {
	ruby_log_error(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, server,
		       "failed to require %s", filename);
	ruby_log_error_string(server, ruby_get_error_info(state));
    }
    return NULL;
}

static void ruby_require(pool *p, char *filename,
			 server_rec *server,
			 ruby_server_config *sconf, ruby_dir_config *dconf)
{
    require_internal_arg_t *arg;

    arg = ap_palloc(p, sizeof(require_internal_arg_t));
    arg->filename = filename;
    arg->server = server;
    arg->sconf = sconf;
    arg->dconf = dconf;
#if APR_HAS_THREADS
    if (ruby_is_threaded_mpm) {
	apr_status_t status;
	char buf[256];

	status = ruby_call_interpreter(p,
				       (ruby_interp_func_t) ruby_require_internal,
				       arg, NULL, 0);
	if (status != APR_SUCCESS) {
	    apr_strerror(status, buf, sizeof(buf));
	    ruby_log_error(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, server,
			   "ruby_call_interpreter() failed: %s", buf);
	}
    }
    else {
#endif
	ruby_require_internal(arg);
#if APR_HAS_THREADS
    }
#endif
}

const char *ruby_cmd_require(cmd_parms *cmd, ruby_dir_config *dconf, char *arg)
{
    ruby_server_config *sconf = get_server_config(cmd->server);
    ruby_library_context *lib;

    check_restrict_directives(cmd, dconf)
    if (ruby_running()) {
	ruby_require(cmd->pool, arg, cmd->server, sconf, dconf);
    }
    else {
	if (ruby_required_libraries == NULL)
	    ruby_required_libraries =
		ap_make_array(cmd->pool, 1, sizeof(ruby_library_context));
	lib = (ruby_library_context *) ap_push_array(ruby_required_libraries);
	lib->filename = arg;
	lib->server_config = sconf;
	lib->dir_config = dconf;
    }
    return NULL;
}

const char *ruby_cmd_pass_env(cmd_parms *cmd, void *dummy, char *arg)
{
    ruby_server_config *conf = get_server_config(cmd->server);
    char *key;
    char *val = strchr(arg, ':');

    if (val) {
	key = ap_pstrndup(cmd->pool, arg, val - arg);
	val++;
    }
    else {
	key = arg;
	val = getenv(key);
    }
    ap_table_set(conf->env, key, val);
    return NULL;
}

const char *ruby_cmd_set_env(cmd_parms *cmd, ruby_dir_config *conf,
			     char *key, char *val)
{
    check_restrict_directives(cmd, conf)
    ap_table_set(conf->env, key, val);
    if (cmd->path == NULL) {
	ruby_server_config *sconf = get_server_config(cmd->server);
	ap_table_set(sconf->env, key, val);
    }
    return NULL;
}

const char *ruby_cmd_restrict_directives(cmd_parms *cmd, void *dummy, int flag)
{
    ruby_server_config *sconf = get_server_config(cmd->server);
    sconf->restrict_directives = flag;
    return NULL;
}

const char *ruby_cmd_timeout(cmd_parms *cmd, void *dummy, char *arg)
{
    ruby_server_config *conf = get_server_config(cmd->server);

    conf->timeout = atoi(arg);
    return NULL;
}

const char *ruby_cmd_safe_level(cmd_parms *cmd, ruby_dir_config *conf, char *arg)
{
    check_restrict_directives(cmd, conf)
    if (cmd->path == NULL && !cmd->server->is_virtual) {
	conf->safe_level = default_safe_level = atoi(arg);
    }
    else {
	conf->safe_level = atoi(arg);
    }
    return NULL;
}

const char *ruby_cmd_output_mode(cmd_parms *cmd, ruby_dir_config *conf, char *arg)
{
    check_restrict_directives(cmd, conf)
    if (strcasecmp(arg, "nosync") == 0) {
	conf->output_mode = MR_OUTPUT_NOSYNC;
    }
    else if (strcasecmp(arg, "sync") == 0) {
	conf->output_mode = MR_OUTPUT_SYNC;
    }
    else if (strcasecmp(arg, "syncheader") == 0) {
	conf->output_mode = MR_OUTPUT_SYNC_HEADER;
    }
    else {
	return "unknown mode";
    }
    return NULL;
}

const char *ruby_cmd_handler(cmd_parms *cmd, ruby_dir_config *conf, char *arg)
{
    check_restrict_directives(cmd, conf)
    push_handler(cmd->pool, conf->ruby_handler, arg);
    return NULL;
}

const char *ruby_cmd_trans_handler(cmd_parms *cmd, ruby_dir_config *conf, char *arg)
{
    check_restrict_directives(cmd, conf)
    push_handler(cmd->pool, conf->ruby_trans_handler, arg);
    return NULL;
}

const char *ruby_cmd_authen_handler(cmd_parms *cmd, ruby_dir_config *conf, char *arg)
{
    check_restrict_directives(cmd, conf)
    push_handler(cmd->pool, conf->ruby_authen_handler, arg);
    return NULL;
}

const char *ruby_cmd_authz_handler(cmd_parms *cmd, ruby_dir_config *conf, char *arg)
{
    check_restrict_directives(cmd, conf)
    push_handler(cmd->pool, conf->ruby_authz_handler, arg);
    return NULL;
}

const char *ruby_cmd_access_handler(cmd_parms *cmd, ruby_dir_config *conf, char *arg)
{
    check_restrict_directives(cmd, conf)
    push_handler(cmd->pool, conf->ruby_access_handler, arg);
    return NULL;
}

const char *ruby_cmd_type_handler(cmd_parms *cmd, ruby_dir_config *conf, char *arg)
{
    check_restrict_directives(cmd, conf)
    push_handler(cmd->pool, conf->ruby_type_handler, arg);
    return NULL;
}

const char *ruby_cmd_fixup_handler(cmd_parms *cmd, ruby_dir_config *conf, char *arg)
{
    check_restrict_directives(cmd, conf)
    push_handler(cmd->pool, conf->ruby_fixup_handler, arg);
    return NULL;
}

const char *ruby_cmd_log_handler(cmd_parms *cmd, ruby_dir_config *conf, char *arg)
{
    check_restrict_directives(cmd, conf)
    push_handler(cmd->pool, conf->ruby_log_handler, arg);
    return NULL;
}

const char *ruby_cmd_header_parser_handler(cmd_parms *cmd,
					   ruby_dir_config *conf, char *arg)
{
    check_restrict_directives(cmd, conf)
    push_handler(cmd->pool, conf->ruby_header_parser_handler, arg);
    return NULL;
}

const char *ruby_cmd_post_read_request_handler(cmd_parms *cmd,
					       ruby_dir_config *conf, char *arg)
{
    check_restrict_directives(cmd, conf)
    push_handler(cmd->pool, conf->ruby_post_read_request_handler, arg);
    return NULL;
}

const char *ruby_cmd_init_handler(cmd_parms *cmd,
				  ruby_dir_config *conf, char *arg)
{
    check_restrict_directives(cmd, conf)
    push_handler(cmd->pool, conf->ruby_init_handler, arg);
    return NULL;
}

const char *ruby_cmd_cleanup_handler(cmd_parms *cmd,
				     ruby_dir_config *conf, char *arg)
{
    check_restrict_directives(cmd, conf)
    push_handler(cmd->pool, conf->ruby_cleanup_handler, arg);
    return NULL;
}

const char *ruby_cmd_child_init_handler(cmd_parms *cmd,
					void *dummy, char *arg)
{
    ruby_server_config *conf = get_server_config(cmd->server);

    push_handler(cmd->pool, conf->ruby_child_init_handler, arg);
    return NULL;
}

/*
 * Local variables:
 * mode: C
 * tab-width: 8
 * End:
 */

/* vim: set filetype=c ts=8 sw=4 : */
