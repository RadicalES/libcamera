# SPDX-License-Identifier: CC0-1.0

platform = get_option('android_platform')
if platform == 'generic'
    android_hal_sources += files(['generic_camera_buffer.cpp',
                                  'generic_frame_buffer_allocator.cpp'])

    # libui = dependency('ui', required : true)
  #  libhardware = dependency('libhardware', required : true)
  #  if libhardware.found()
  #      android_deps += [libhardware]
  #  else
#        android_hal_sources += files(['graphic_buffer_allocator_stub.cpp'])
   # endif
elif platform == 'cros'
    android_hal_sources += files(['cros_camera_buffer.cpp',
                                  'cros_frame_buffer_allocator.cpp'])
    android_deps += [dependency('libcros_camera')]
endif