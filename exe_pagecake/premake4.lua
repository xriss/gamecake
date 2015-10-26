
--[[

make sure lua_preloadlibs is called in ngx_http_lua_util.c if we bump nginx sourcecode

]]


project "pagecake"
language "C++"

includedirs {	"." ,
		"objs" ,
		"src" ,
		"src/core" ,
		"src/os" ,
		"src/os/unix" ,
		"src/event" ,
		"src/mail" ,
		"src/http" ,
		"src/misc" ,
		"src/http/modules" ,
		"src/http/modules/echo" ,
		"ndk/objs" ,
		"ndk/src" ,
		"lua/src" ,
		"lua/src/api" ,
		"../lib_z" ,
		"../lib_pcre" ,
}

if NACL then
elseif ANDROID then 
else

-- look around the exe for any dynamic code we might want	
	if CPU=="64" or ( CPU~="32" and BUILD_CPU=="64" ) then
		linkoptions { "-Wl,-R\\$$ORIGIN/x64" } -- so much escape \\$$ -> $
	elseif CPU=="32" or BUILD_CPU=="32" then
		linkoptions { "-Wl,-R\\$$ORIGIN/x32" } -- so much escape \\$$ -> $
	else
		linkoptions { "-Wl,-R\\$$ORIGIN" } -- so much escape \\$$ -> $
	end


if LSB then
	linkoptions { "--lsb-use-default-linker" }
	linkoptions { "--lsb-besteffort" }
end
	
	
	defines "NGX_LINUX"
	defines "NGX_THREADS"

-- enable SSL
	defines "NGX_HTTP_SSL"
	defines "NGX_OPENSSL"
	defines "NGX_SSL"
	defines "NGX_HAVE_SHA1"
	defines "NGX_HAVE_OPENSSL_SHA1_H"

	files { "./**.h" }
	files { "./objs/**.c" }
	files { "./src/core/**.c" }

	files { --"./src/event/**.c" }
--		"./src/event/modules",
		"./src/event/ngx_event.c",
		"./src/event/ngx_event_accept.c",
		"./src/event/ngx_event_busy_lock.c",
		"./src/event/ngx_event_connect.c",
		"./src/event/ngx_event_mutex.c",
		"./src/event/ngx_event_openssl.c",
		"./src/event/ngx_event_openssl_stapling.c",
		"./src/event/ngx_event_pipe.c",
		"./src/event/ngx_event_pipe.h",
		"./src/event/ngx_event_posted.c",
		"./src/event/ngx_event_timer.c",

--		"./src/event/modules/ngx_aio_module.c",
--		"./src/event/modules/ngx_devpoll_module.c",
		"./src/event/modules/ngx_epoll_module.c",
--		"./src/event/modules/ngx_eventport_module.c",
--		"./src/event/modules/ngx_kqueue_module.c",
--		"./src/event/modules/ngx_poll_module.c",
--		"./src/event/modules/ngx_rtsig_module.c",
--		"./src/event/modules/ngx_select_module.c",
--		"./src/event/modules/ngx_win32_select_module.c",

	}

	files { --"./src/http/**.c" }
		"./src/http/ngx_http.c",
		"./src/http/ngx_http_busy_lock.c",
		"./src/http/ngx_http_copy_filter_module.c",
		"./src/http/ngx_http_core_module.c",
		"./src/http/ngx_http_file_cache.c",
		"./src/http/ngx_http_header_filter_module.c",
		"./src/http/ngx_http_parse.c",
		"./src/http/ngx_http_parse_time.c",
		"./src/http/ngx_http_postpone_filter_module.c",
		"./src/http/ngx_http_request.c",
		"./src/http/ngx_http_request_body.c",
		"./src/http/ngx_http_script.c",
		"./src/http/ngx_http_special_response.c",
		"./src/http/ngx_http_upstream.c",
		"./src/http/ngx_http_upstream_round_robin.c",
		"./src/http/ngx_http_variables.c",
		"./src/http/ngx_http_write_filter_module.c",

		"./src/http/modules/ngx_http_access_module.c",
		"./src/http/modules/ngx_http_addition_filter_module.c",
		"./src/http/modules/ngx_http_auth_basic_module.c",
		"./src/http/modules/ngx_http_autoindex_module.c",
		"./src/http/modules/ngx_http_browser_module.c",
		"./src/http/modules/ngx_http_charset_filter_module.c",
		"./src/http/modules/ngx_http_chunked_filter_module.c",
--		"./src/http/modules/ngx_http_dav_module.c",
--		"./src/http/modules/ngx_http_degradation_module.c",
		"./src/http/modules/ngx_http_empty_gif_module.c",
		"./src/http/modules/ngx_http_fastcgi_module.c",
		"./src/http/modules/ngx_http_flv_module.c",
--		"./src/http/modules/ngx_http_geoip_module.c",
		"./src/http/modules/ngx_http_geo_module.c",
		"./src/http/modules/ngx_http_gzip_filter_module.c",
		"./src/http/modules/ngx_http_gzip_static_module.c",
		"./src/http/modules/ngx_http_headers_filter_module.c",
--		"./src/http/modules/ngx_http_image_filter_module.c",
		"./src/http/modules/ngx_http_index_module.c",
--		"./src/http/modules/ngx_http_limit_conn_module.c",
		"./src/http/modules/ngx_http_limit_req_module.c",
		"./src/http/modules/ngx_http_log_module.c",
		"./src/http/modules/ngx_http_map_module.c",
		"./src/http/modules/ngx_http_memcached_module.c",
		"./src/http/modules/ngx_http_mp4_module.c",
		"./src/http/modules/ngx_http_not_modified_filter_module.c",
		"./src/http/modules/ngx_http_proxy_module.c",
		"./src/http/modules/ngx_http_random_index_module.c",
		"./src/http/modules/ngx_http_range_filter_module.c",
--		"./src/http/modules/ngx_http_realip_module.c",
		"./src/http/modules/ngx_http_referer_module.c",
		"./src/http/modules/ngx_http_rewrite_module.c",
		"./src/http/modules/ngx_http_scgi_module.c",
		"./src/http/modules/ngx_http_secure_link_module.c",
		"./src/http/modules/ngx_http_split_clients_module.c",
		"./src/http/modules/ngx_http_ssi_filter_module.c",
		"./src/http/modules/ngx_http_ssl_module.c",
		"./src/http/modules/ngx_http_static_module.c",
		"./src/http/modules/ngx_http_sub_filter_module.c",
		"./src/http/modules/ngx_http_upstream_ip_hash_module.c",
--		"./src/http/modules/ngx_http_upstream_keepalive_module.c",
		"./src/http/modules/ngx_http_userid_filter_module.c",
		"./src/http/modules/ngx_http_uwsgi_module.c",
--		"./src/http/modules/ngx_http_xslt_filter_module.c",

}

--	files { "./src/http/modules/echo/**.c" }


	files {
--		"./src/os/unix/ngx_aio_read.c",
--		"./src/os/unix/ngx_aio_read_chain.c",
--		"./src/os/unix/ngx_aio_write.c",
--		"./src/os/unix/ngx_aio_write_chain.c",
		"./src/os/unix/ngx_alloc.c",
		"./src/os/unix/ngx_channel.c",
		"./src/os/unix/ngx_daemon.c",
--		"./src/os/unix/ngx_darwin_init.c",
--		"./src/os/unix/ngx_darwin_sendfile_chain.c",
		"./src/os/unix/ngx_errno.c",
--		"./src/os/unix/ngx_file_aio_read.c",
		"./src/os/unix/ngx_files.c",
--		"./src/os/unix/ngx_freebsd_init.c",
--		"./src/os/unix/ngx_freebsd_rfork_thread.c",
--		"./src/os/unix/ngx_freebsd_sendfile_chain.c",
--		"./src/os/unix/ngx_linux_aio_read.c",
		"./src/os/unix/ngx_linux_init.c",
		"./src/os/unix/ngx_linux_sendfile_chain.c",
		"./src/os/unix/ngx_posix_init.c",
		"./src/os/unix/ngx_process.c",
		"./src/os/unix/ngx_process_cycle.c",
		"./src/os/unix/ngx_pthread_thread.c",
		"./src/os/unix/ngx_readv_chain.c",
		"./src/os/unix/ngx_recv.c",
		"./src/os/unix/ngx_send.c",
		"./src/os/unix/ngx_setproctitle.c",
		"./src/os/unix/ngx_shmem.c",
		"./src/os/unix/ngx_socket.c",
--		"./src/os/unix/ngx_solaris_init.c",
--		"./src/os/unix/ngx_solaris_sendfilev_chain.c",
		"./src/os/unix/ngx_time.c",
		"./src/os/unix/ngx_udp_recv.c",
		"./src/os/unix/ngx_user.c",
		"./src/os/unix/ngx_writev_chain.c",
		"./src/os/unix/ngx_setaffinity.c",
	}

	files { "./lua/src/**.h" , "./lua/src/**.c" }

	files { "../exe_gamecake/*.c" }
	excludes { "../exe_gamecake/lua.c" }
	
-- link in luajit that was compiled externaly
if LUA_LIBDIRS then	libdirs(LUA_LIBDIRS) end
if LUA_LINKS   then links  (LUA_LINKS)   end

	links(static_lib_names)

-- luajit and SDL2 dev must be available
-- use the vagrant boxes if you dont want to deal with it
	libdirs { "/usr/local/lib/" }
	links { "luajit-5.1" }
	links { "SDL2" }	

	links { "GL" }
	links { "crypt" }
	links { "pthread" }
	links { "dl" , "m" , "rt" }
	links { "X11" }

	links { "ssl" , "crypto"}
	
	if CPU=="64" then
		KIND{kind="ConsoleApp",name="pagecake.x64"}
	elseif CPU=="32" then
		KIND{kind="ConsoleApp",name="pagecake.x32"}
	else
		KIND{kind="ConsoleApp",name="pagecake.nix"}
	end
	
end
