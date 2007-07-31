/*
 * $Id$
 * Copyright (C) 2000  ZetaBITS, Inc.
 * Copyright (C) 2000  Information-technology Promotion Agency, Japan
 * Copyright (C) 2000  Shugo Maeda <shugo@modruby.net>
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

#ifndef RUBY_CONFIG_H
#define RUBY_CONFIG_H

void *ruby_create_server_config(pool*, server_rec*);
void *ruby_merge_server_config(pool*, void*, void*);
void *ruby_create_dir_config (pool*, char*);
void *ruby_merge_dir_config(pool*, void*, void*);
const char *ruby_cmd_kanji_code(cmd_parms*, void*, const char*);
const char *ruby_cmd_add_path(cmd_parms*, void*, const char*);
const char *ruby_cmd_require(cmd_parms*, void*, const char*);
const char *ruby_cmd_pass_env(cmd_parms*, void*, const char*);
const char *ruby_cmd_set_env(cmd_parms*, void*, const char*, const char*);
const char *ruby_cmd_timeout(cmd_parms*, void*, const char*);
const char *ruby_cmd_safe_level(cmd_parms*, void*, const char*);
const char *ruby_cmd_output_mode(cmd_parms*, void*, const char*);
const char *ruby_cmd_restrict_directives(cmd_parms*, void*, int);
const char *ruby_cmd_option(cmd_parms*, void*, const char*, const char*);
const char *ruby_cmd_gc_per_request(cmd_parms*, void*, int);
const char *ruby_cmd_handler(cmd_parms*, void*, const char*);
const char *ruby_cmd_trans_handler(cmd_parms*, void*, const char*);
const char *ruby_cmd_authen_handler(cmd_parms*, void*, const char*);
const char *ruby_cmd_authz_handler(cmd_parms*, void*, const char*);
const char *ruby_cmd_access_handler(cmd_parms*, void*, const char*);
const char *ruby_cmd_type_handler(cmd_parms*, void*, const char*);
const char *ruby_cmd_fixup_handler(cmd_parms*, void*, const char*);
const char *ruby_cmd_log_handler(cmd_parms*, void*, const char*);
#ifdef APACHE2
const char *ruby_cmd_error_log_handler(cmd_parms*, void*, const char*);
#endif
const char *ruby_cmd_header_parser_handler(cmd_parms*, void*, const char*);
const char *ruby_cmd_post_read_request_handler(cmd_parms*, void*, const char*);
const char *ruby_cmd_init_handler(cmd_parms*, void*, const char*);
const char *ruby_cmd_cleanup_handler(cmd_parms*, void*, const char*);
const char *ruby_cmd_child_init_handler(cmd_parms*, void*, const char*);

#endif /* !RUBY_CONFIG_H */

/*
 * Local variables:
 * mode: C
 * tab-width: 8
 * End:
 */

/* vim: set filetype=c ts=8 sw=4 : */
