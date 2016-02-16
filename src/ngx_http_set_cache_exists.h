#ifndef NGX_HTTP_SET_CACHE_EXISTS
#define NGX_HTTP_SET_CACHE_EXISTS

#include <ngx_core.h>

ngx_int_t
set_cache_exists(ngx_http_request_t *r, ngx_str_t *res, ngx_http_variable_value_t *v);

#endif		/* NGX_HTTP_SET_CACHE_EXISTS */
