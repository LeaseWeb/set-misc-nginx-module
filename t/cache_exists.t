# vi:filetype=perl

use lib 'lib';
use Test::Nginx::Socket;

plan tests => repeat_each() * blocks();

no_long_string();
no_shuffle();

run_tests();

__DATA__

=== TEST 1: Get a file that doesn't exist in cache
--- init
    use File::Path qw(remove_tree make_path);
    remove_tree '/tmp/nginx_test';
    make_path   '/tmp/nginx_test';

--- http_config
    proxy_cache_path /tmp/nginx_test keys_zone=main:8m;

--- config
    location /bar {
	set_cache_exists $dst http://127.0.0.1$uri /tmp/nginx_test 0;
	if ($dst) {
	   return 200;
	}
	return 303 http://example.com$uri;
    }
--- request
    GET /bar/file.mp4
--- error_code: 303

=== TEST 2: Check for a file that exists in cache
--- init
    use File::Path qw(remove_tree make_path);
    remove_tree '/tmp/nginx_test';
    make_path   '/tmp/nginx_test';

    open q/TMPF/, "> /tmp/nginx_test/145c46d84b18e8f22c3d7f01a07ab42d";
    print TMPF "Sample text\n";

--- http_config
    proxy_cache_path /tmp/nginx_test keys_zone=main:8m;

--- config
    location /bar {
	set_cache_exists $dst http://127.0.0.1$uri /tmp/nginx_test 0;
	if ($dst) {
	   return 200;
	}
	return 303 http://example.com$uri;
    }
--- request
    GET /bar/file.mp4
--- error_code: 200

=== TEST 3: Check for a file in a 2-level cache path
--- init
    use File::Path qw(remove_tree make_path);
    remove_tree '/tmp/nginx_test';
    make_path   '/tmp/nginx_test/d/';

    open q/TMPF/, "> /tmp/nginx_test/d/145c46d84b18e8f22c3d7f01a07ab42d";
    print(TMPF "Sample text\n");

--- http_config
    proxy_cache_path /tmp/nginx_test keys_zone=main:8m levels=1;

--- config
    location /bar {
	set_cache_exists $dst http://127.0.0.1$uri /tmp/nginx_test 1;
	if ($dst) {
	   return 200;
	}
	return 303 http://example.com$uri;
    }
--- request
    GET /bar/file.mp4
--- error_code: 200

=== TEST 4: Check for a file in a 3-level cache path
--- init
    use File::Path qw(remove_tree make_path);
    remove_tree '/tmp/nginx_test';
    make_path   '/tmp/nginx_test/d/42/';

    open q/TMPF/, "> /tmp/nginx_test/d/42/145c46d84b18e8f22c3d7f01a07ab42d";
    print(TMPF "Sample text\n");

--- http_config
    proxy_cache_path /tmp/nginx_test keys_zone=main:8m levels=1:2;

--- config
    location /bar {
	set_cache_exists $dst http://127.0.0.1$uri /tmp/nginx_test 1:2;
	if ($dst) {
	   return 200;
	}
	return 303 http://example.com$uri;
    }
--- request
    GET /bar/file.mp4
--- error_code: 200
