
project "pepper"
kind "StaticLib"
language "C++"

files{
	"pepper.js/examples/ppapi_gles2/**.cc",
	"pepper.js/examples/ppapi_cpp/**.cc",
	"pepper.js/examples/ppapi/**.cc",
}

files{

	naclsdk_path.."/src/ppapi_cpp/array_output.cc",
	naclsdk_path.."/src/ppapi_cpp/audio.cc",
	naclsdk_path.."/src/ppapi_cpp/audio_config.cc",
	naclsdk_path.."/src/ppapi_cpp/core.cc",
	naclsdk_path.."/src/ppapi_cpp/cursor_control_dev.cc",
	naclsdk_path.."/src/ppapi_cpp/directory_entry.cc",
	naclsdk_path.."/src/ppapi_cpp/file_chooser_dev.cc",
	naclsdk_path.."/src/ppapi_cpp/file_io.cc",
	naclsdk_path.."/src/ppapi_cpp/file_ref.cc",
	naclsdk_path.."/src/ppapi_cpp/file_system.cc",
	naclsdk_path.."/src/ppapi_cpp/font_dev.cc",
	naclsdk_path.."/src/ppapi_cpp/fullscreen.cc",
	naclsdk_path.."/src/ppapi_cpp/graphics_2d.cc",
	naclsdk_path.."/src/ppapi_cpp/graphics_3d.cc",
	naclsdk_path.."/src/ppapi_cpp/graphics_3d_client.cc",
	naclsdk_path.."/src/ppapi_cpp/host_resolver.cc",
	naclsdk_path.."/src/ppapi_cpp/image_data.cc",
	naclsdk_path.."/src/ppapi_cpp/input_event.cc",
	naclsdk_path.."/src/ppapi_cpp/instance.cc",
	naclsdk_path.."/src/ppapi_cpp/instance_handle.cc",
	naclsdk_path.."/src/ppapi_cpp/lock.cc",
	naclsdk_path.."/src/ppapi_cpp/memory_dev.cc",
	naclsdk_path.."/src/ppapi_cpp/message_loop.cc",
	naclsdk_path.."/src/ppapi_cpp/module.cc",
	naclsdk_path.."/src/ppapi_cpp/mouse_cursor.cc",
	naclsdk_path.."/src/ppapi_cpp/mouse_lock.cc",
	naclsdk_path.."/src/ppapi_cpp/net_address.cc",
	naclsdk_path.."/src/ppapi_cpp/network_list.cc",
	naclsdk_path.."/src/ppapi_cpp/network_monitor.cc",
	naclsdk_path.."/src/ppapi_cpp/network_proxy.cc",
	naclsdk_path.."/src/ppapi_cpp/paint_aggregator.cc",
	naclsdk_path.."/src/ppapi_cpp/paint_manager.cc",
	naclsdk_path.."/src/ppapi_cpp/ppp_entrypoints.cc",
	naclsdk_path.."/src/ppapi_cpp/printing_dev.cc",
	naclsdk_path.."/src/ppapi_cpp/rect.cc",
	naclsdk_path.."/src/ppapi_cpp/resource.cc",
	naclsdk_path.."/src/ppapi_cpp/scriptable_object_deprecated.cc",
	naclsdk_path.."/src/ppapi_cpp/selection_dev.cc",
	naclsdk_path.."/src/ppapi_cpp/simple_thread.cc",
	naclsdk_path.."/src/ppapi_cpp/socket_dev.cc",
	naclsdk_path.."/src/ppapi_cpp/tcp_socket.cc",
	naclsdk_path.."/src/ppapi_cpp/text_input_controller.cc",
	naclsdk_path.."/src/ppapi_cpp/truetype_font_dev.cc",
	naclsdk_path.."/src/ppapi_cpp/udp_socket.cc",
	naclsdk_path.."/src/ppapi_cpp/url_loader.cc",
	naclsdk_path.."/src/ppapi_cpp/url_request_info.cc",
	naclsdk_path.."/src/ppapi_cpp/url_response_info.cc",
	naclsdk_path.."/src/ppapi_cpp/var.cc",
	naclsdk_path.."/src/ppapi_cpp/var_array.cc",
	naclsdk_path.."/src/ppapi_cpp/var_array_buffer.cc",
	naclsdk_path.."/src/ppapi_cpp/var_dictionary.cc",
	naclsdk_path.."/src/ppapi_cpp/view.cc",
	naclsdk_path.."/src/ppapi_cpp/view_dev.cc",
	naclsdk_path.."/src/ppapi_cpp/websocket.cc",
	naclsdk_path.."/src/ppapi_cpp/websocket_api.cc",
	naclsdk_path.."/src/ppapi_cpp/zoom_dev.cc",

--	naclsdk_path.."/src/ppapi_cpp/alarms_dev.cc",
--	naclsdk_path.."/src/ppapi_cpp/event_base.cc",
--	naclsdk_path.."/src/ppapi_cpp/events_dev.cc",

--	naclsdk_path.."/src/ppapi_cpp/alarms_dev.cc",
--	naclsdk_path.."/src/ppapi_cpp/media_stream_video_track.cc",
--	naclsdk_path.."/src/ppapi_cpp/string_wrapper_dev.cc",
--	naclsdk_path.."/src/ppapi_cpp/video_frame.cc",

	naclsdk_path.."/src/ppapi_cpp/resource_array_dev.cc",
	naclsdk_path.."/src/ppapi_cpp/var_resource_dev.cc",

	naclsdk_path.."/src/ppapi_cpp/audio_input_dev.cc",
	naclsdk_path.."/src/ppapi_cpp/buffer_dev.cc",
	naclsdk_path.."/src/ppapi_cpp/crypto_dev.cc",
	naclsdk_path.."/src/ppapi_cpp/device_ref_dev.cc",
	naclsdk_path.."/src/ppapi_cpp/find_dev.cc",
	naclsdk_path.."/src/ppapi_cpp/graphics_2d_dev.cc",
	naclsdk_path.."/src/ppapi_cpp/ime_input_event_dev.cc",
	naclsdk_path.."/src/ppapi_cpp/scrollbar_dev.cc",
	naclsdk_path.."/src/ppapi_cpp/text_input_dev.cc",
	naclsdk_path.."/src/ppapi_cpp/url_util_dev.cc",
	naclsdk_path.."/src/ppapi_cpp/video_capture_client_dev.cc",
	naclsdk_path.."/src/ppapi_cpp/video_capture_dev.cc",
	naclsdk_path.."/src/ppapi_cpp/video_decoder_client_dev.cc",
	naclsdk_path.."/src/ppapi_cpp/video_decoder_dev.cc",
	naclsdk_path.."/src/ppapi_cpp/widget_client_dev.cc",
	naclsdk_path.."/src/ppapi_cpp/widget_dev.cc",
	
	
	
	
	naclsdk_path.."/src/ppapi_gles2/gl2ext_ppapi.c",
	naclsdk_path.."/src/ppapi_gles2/gles2.c",

}



KIND{}
