
project "nginx"
language "C"

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
		"../lib_z" ,
		"../lib_pcre" ,
}

if NACL then
elseif ANDROID then 
else

--	defines "NGX_HAVE_AIO"
	defines "NGX_LINUX"
	defines "NGX_THREADS"
--	defines "NDK"

-- enable SSL
	defines "NGX_HTTP_SSL"
	defines "NGX_OPENSSL"
	defines "NGX_SSL"

	files { "./**.h" }
	files { "./objs/**.c" }
	files { "./src/core/**.c" }

--	files { "./src/misc/**.c" }

	files { --"./src/event/**.c" }
--		"./src/event/modules",
		"./src/event/ngx_event.c",
		"./src/event/ngx_event_accept.c",
		"./src/event/ngx_event_busy_lock.c",
		"./src/event/ngx_event_connect.c",
		"./src/event/ngx_event_mutex.c",
		"./src/event/ngx_event_openssl.c",
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

	files { "./src/http/modules/echo/**.c" }


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
	}

	files { "./lua/src/**.h" , "./lua/src/**.c" }
--	excludes { "./lua/src/ngx_http_lua_ndk.c" }
--	excludes { "./lua/src/ngx_http_lua_directive.c" }

--	files { "./ndk/src/**.h" }
--	files { "./ndk/src/ndk.c" }
--[[
	files { "./ndk/src/**.h" , "./ndk/src/**.c" }
	excludes { "./ndk/src/ndk_upstream_list.*" }
	excludes { "./ndk/src/ndk_process.*" }
	excludes { "./ndk/src/ndk_uri.*" }
	excludes { "./ndk/src/ndk_complex_path.*" }
	excludes { "./ndk/src/ndk_set_var.*" }
	excludes { "./ndk/src/ndk_encoding.*" }
	excludes { "./ndk/src/ndk_regex.*" }
	excludes { "./ndk/src/ndk_http.*" }
]]

	files { "../gamecake/*.c" }
	

	links(static_lib_names)

	links { "GL" , "GLU" }
	links { "crypt" }
	links { "pthread" }
	links { "dl" , "m" , "rt" }

	links { "X11" , "udev" }
	
	links { "ssl" , "crypto"}
	
	if CPU=="64" then
		KIND{kind="ConsoleApp",name="nginx.x64"}
	else
		KIND{kind="ConsoleApp",name="nginx"}
	end
	
end
