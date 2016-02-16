#ifndef DDEBUG
#define DDEBUG 0
#endif
#include "ddebug.h"

#include <ndk.h>
#include <ngx_md5.h>

// Conveniently convert a *ngx_http_variable_value_t into an ngx_str_t
#define make_ngx_string(v)	{ (v)->len, (v)->data }

static size_t	calculate_level_path_length(ngx_str_t* levels);
static void	copy_level_path(u_char *path, ngx_str_t *levels, ngx_str_t *digest);
static void	create_cache_key(u_char* dst, ngx_str_t* src);

/*
 * set_cache_exists()
 * Return "0" or "1" if given URI exists in nginx cache.
 *
 * Arguments:
 * 	target_var	= $cache_exists
 * 	cache_key	= $upstream$uri
 * 	cache_path	= "/tmp/ngx_tmp"
 * 	cache_levels	= 1:2
 */
ngx_int_t
set_cache_exists(ngx_http_request_t *req, ngx_str_t *res, ngx_http_variable_value_t *value)
{
   ngx_log_t* log = req->connection->log;		// shortcut

   // directive arguments
   ngx_str_t cache_key    = make_ngx_string(&value[0]);
   ngx_str_t cache_path   = make_ngx_string(&value[1]);
   ngx_str_t cache_levels = make_ngx_string(&value[2]);

   size_t len = calculate_level_path_length(&cache_levels);
   ngx_log_error(NGX_LOG_DEBUG, log, 0, "[cache_exists]: level_length = %d", len);

   // create MD5 digest (cache key) from given URI
   u_char hexkey[20];
   create_cache_key(hexkey, &cache_key);
   ngx_str_t digest = { ngx_strlen(hexkey), hexkey };
   ngx_log_error(NGX_LOG_DEBUG, log, 0, "[cache_exists]: key digest = %s", hexkey);

   // create final cache, full pathname
   u_char path[NGX_MAX_PATH], level_path[NGX_MAX_PATH];
   ngx_memzero(path, sizeof(path));
   copy_level_path(level_path, &cache_levels, &digest);

   ngx_memcpy(path, cache_path.data, cache_path.len);
   ngx_memcpy(path + cache_path.len, level_path, len);
   ngx_memcpy(path + cache_path.len + len, digest.data, digest.len);
   ngx_log_error(NGX_LOG_DEBUG, log, 0, "[cache_exists]: path = %s", path);

   // reserve memory for return variable (1 byte)
   res->len = 1;
   res->data = ngx_palloc(req->pool, 1);
   if (!res->data) return NGX_ERROR;

   res->data[0] = access((char *) path, F_OK) == 0 ? '1' : '0';
   return NGX_OK;
}

// Find the length of level prefix for the digest_key based on the levels string
size_t
calculate_level_path_length(ngx_str_t *levels)
{
   u_char* p = levels->data;
   size_t i, res = 1, len = levels->len;

   for (i = 0; i < len; i++) {
      if (p[i] == ':') res++;
      else res += p[i] - '0';
   }
   if (res > 1) res++;
   return res;
}

// Copy the level path into the path string, based on the levels string.
void
copy_level_path(u_char *path, ngx_str_t *levels, ngx_str_t *digest)
{
   u_char* lp = levels->data;		// shortcuts
   u_char* dp = digest->data;
   size_t digest_len = digest->len;
   size_t level_len  = levels->len;
   size_t i, pos = digest_len;
   u_char *str = path;
   *str++ = '/';

   for (i = 0; i < level_len; i++) {
      if (lp[i] == ':') {
	 *str++ = '/';
      } else {
	 size_t j, len = lp[i] - '0';
	 pos -= len;
	 for (j = 0; j < len; j++)
	    *str++ = dp[pos + j];
      }
   }

   if (str > path) *str = '/';
}

/**
 * Return the equivalent ASCII text of a cache hash key.
 */
void
create_cache_key(u_char* buf, ngx_str_t* c)
{
   uint32_t crc;
   ngx_md5_t md5;
   u_char key[40];

   ngx_crc32_init(crc);
   ngx_md5_init(&md5);

   ngx_crc32_update(&crc, c->data, c->len);
   ngx_md5_update(&md5, c->data, c->len);

   ngx_crc32_final(crc);
   ngx_md5_final(key, &md5);

   u_char *p = ngx_hex_dump(buf, key, NGX_HTTP_CACHE_KEY_LEN);
   *p = 0;
}
