#include "httpd.h"
#include "http_config.h"
extern module ruby_module;
module * stub_function(void)
{
    return &ruby_module;
}

/* vim: set filetype=c ts=8 sw=4 : */
