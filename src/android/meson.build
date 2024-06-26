# SPDX-License-Identifier: CC0-1.0

# XXX library lookup paths are not being passed through from the cross compile configuration
android_ndk = get_option('prefix')
android_lib_dir = get_option('libdir')
android_include_dir = get_option('includedir').split(':')
# android_libui_dep = cc.find_library('libui', dirs : android_lib_dir)

message('Android? = ', get_option('android'))
message('Android NDK=', android_ndk)
message('Android LibDir=', android_lib_dir)
message('Android IncDir=', android_include_dir)
# message('LibCamera_private', libcamera_private)

android_deps = [
    dependency('libexif', required : get_option('android')),
    dependency('libjpeg', required : get_option('android')),
   # dependency('libui', required : get_option('android')),
    libcamera_private,
]

android_enabled = true

foreach dep : android_deps
    if not dep.found()
        android_enabled = false
        subdir_done()
    endif
endforeach

libyuv_dep = dependency('libyuv', required : false)

# Fallback to a subproject if libyuv isn't found, as it's typically not
# provided by distributions.
if not libyuv_dep.found()
    cmake = import('cmake')

    libyuv_vars = cmake.subproject_options()
    libyuv_vars.add_cmake_defines({'CMAKE_POSITION_INDEPENDENT_CODE': 'ON'})
    libyuv_vars.set_override_option('cpp_std', 'c++17')
    libyuv_vars.append_compile_args('cpp',
         '-Wno-sign-compare',
         '-Wno-unused-variable',
         '-Wno-unused-parameter')
    libyuv_vars.append_link_args('-ljpeg')
    libyuv = cmake.subproject('libyuv', options : libyuv_vars)
    libyuv_dep = libyuv.dependency('yuv')
endif

android_deps += [libyuv_dep]

android_hal_sources = files([
    'camera3_hal.cpp',
    'camera_capabilities.cpp',
    'camera_device.cpp',
    'camera_hal_config.cpp',
    'camera_hal_manager.cpp',
    'camera_metadata.cpp',
    'camera_ops.cpp',
    'camera_request.cpp',
    'camera_stream.cpp',
    'hal_framebuffer.cpp',
    'yuv/post_processor_yuv.cpp'
])

android_cpp_args = []

subdir('cros')
subdir('jpeg')
subdir('mm')

android_camera_metadata_sources = files([
    'metadata/camera_metadata.c',
])

android_camera_metadata = static_library('camera_metadata',
                                         android_camera_metadata_sources,
                                         c_args : '-Wno-shadow',
                                         include_directories : android_includes)

libcamera_hal = shared_library('libcamera-hal',
                               android_hal_sources,
                               name_prefix : '',
                               link_with : android_camera_metadata,
                               install : true,
                               cpp_args : android_cpp_args,
                               include_directories : android_includes,
                               dependencies : android_deps)
