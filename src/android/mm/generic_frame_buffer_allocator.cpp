/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2021, Google Inc.
 *
 * Allocate FrameBuffer using gralloc API
 */

#include <dlfcn.h>
#include <memory>
#include <vector>
#include <unistd.h>


#include <libcamera/base/log.h>
#include <libcamera/base/shared_fd.h>

#include "libcamera/internal/formats.h"
#include "libcamera/internal/framebuffer.h"

#include <hardware/camera3.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wextra-semi"
#include <ui/GraphicBufferAllocator.h>
#pragma GCC diagnostic pop
#include <utils/Errors.h>

#include "../camera_device.h"
#include "../frame_buffer_allocator.h"
#include "../hal_framebuffer.h"

using namespace libcamera;

LOG_DECLARE_CATEGORY(HAL)

namespace {
class GenericFrameBufferData : public FrameBuffer::Private
{
	LIBCAMERA_DECLARE_PUBLIC(FrameBuffer)

public:
	GenericFrameBufferData(android::GraphicBufferAllocator &allocDevice,
			       buffer_handle_t handle,
			       const std::vector<FrameBuffer::Plane> &planes)
		: FrameBuffer::Private(planes), allocDevice_(allocDevice),
		  handle_(handle)
	{
		ASSERT(handle_);
	}

	~GenericFrameBufferData() override
	{
		/*
		 * \todo Thread safety against alloc_device_t is not documented.
		 * Is it no problem to call alloc/free in parallel?
		 */
		//allocDevice_->free(allocDevice_, handle_);
		android::status_t status = allocDevice_.free(handle_);
		if (status != android::NO_ERROR)
			LOG(HAL, Error) << "Error freeing framebuffer: " << status;
	}

private:
	android::GraphicBufferAllocator &allocDevice_;
	const buffer_handle_t handle_;
};
} /* namespace */

class PlatformFrameBufferAllocator::Private : public Extensible::Private
{
	LIBCAMERA_DECLARE_PUBLIC(PlatformFrameBufferAllocator)

public:
	Private(CameraDevice *const cameraDevice)
		 : cameraDevice_(cameraDevice),
		  allocDevice_(android::GraphicBufferAllocator::get())
	{
		// hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &hardwareModule_);
		// LOG(HAL, Debug) << "PlatformFrameBufferAllocator::Private hardwareModule=" << hardwareModule_ << " / " << cameraDevice->camera3Device()->common.module;
		// ASSERT(hardwareModule_);
		
	}

	~Private() = default;

	std::unique_ptr<HALFrameBuffer>
	allocate(int halPixelFormat, const libcamera::Size &size, uint32_t usage);

private:
	const CameraDevice *const cameraDevice_;
	//const struct hw_module_t *hardwareModule_;
	//struct alloc_device_t *allocDevice_;
	android::GraphicBufferAllocator &allocDevice_;
};

// PlatformFrameBufferAllocator::Private::~Private()
// {
// 	// if (allocDevice_)
// 	// 	gralloc_close(allocDevice_);
// }

std::unique_ptr<HALFrameBuffer>
PlatformFrameBufferAllocator::Private::allocate(int halPixelFormat,
						const libcamera::Size &size,
						uint32_t usage)
{

	uint32_t stride = 0;
	buffer_handle_t handle = nullptr;

	LOG(HAL, Debug) << "Private::allocate: pixelFormat=" << halPixelFormat << " size=" << size << " usage=" << usage;

	// if (!allocDevice_) {
	// 	int ret = gralloc_open(hardwareModule_, &allocDevice_);
	// 	if (ret) {
	// 		LOG(HAL, Fatal) << "gralloc_open() failed: " << ret;
	// 		return nullptr;
	// 	}
	// }

	// int stride = 0;
	// buffer_handle_t handle = nullptr;
	// int ret = allocDevice_->alloc(allocDevice_, size.width, size.height,
	// 			      halPixelFormat, usage, &handle, &stride);
	// if (ret) {
	// 	LOG(HAL, Error) << "failed buffer allocation: " << ret;
	// 	return nullptr;
	// }

	android::status_t status = allocDevice_.allocate(size.width, size.height, halPixelFormat,
							 1 /*layerCount*/, usage, &handle, &stride,
							 "libcameraHAL");

 // If there's no additional options, fall back to previous allocate version
    //  android::status_t status = allocDevice_.allocate(request.requestorName, request.width, request.height,
    //                                          request.format, request.layerCount, request.usage,
    //                                          &result.stride, &result.handle, request.importBuffer);

	if (status != android::NO_ERROR) {
		LOG(HAL, Error) << "failed buffer allocation: " << status;
		return nullptr;
	}


	if (!handle) {
		LOG(HAL, Fatal) << "invalid buffer_handle_t";
		return nullptr;
	}

	/* This code assumes the planes are mapped consecutively. */
	const libcamera::PixelFormat pixelFormat =
		cameraDevice_->capabilities()->toPixelFormat(halPixelFormat);
	const auto &info = PixelFormatInfo::info(pixelFormat);
	std::vector<FrameBuffer::Plane> planes(info.numPlanes());

	SharedFD fd{ handle->data[0] };
	const size_t maxDmaLength = lseek(fd.get(), 0, SEEK_END);
	
	LOG(HAL, Debug) << "Private::allocate: created fd=" << fd.get() 
					<< " pixelFormat=" << info.name 
					<< " stride=" << stride
					<< " numPlanes=" << info.numPlanes()
					<< " numFds=" << handle->numFds
					<< " numInts=" << handle->numInts
					<< " dmaLength=" << maxDmaLength;

	for(int i=0; i<handle->numFds; i++) {
		int fdd = handle->data[i];
		const size_t len = lseek(fdd, 0, SEEK_END);
		LOG(HAL, Debug) << "Private::allocate: fd info fd=" << fdd << " len=" << len;
	}
	
	size_t offset = 0;
	for (auto [i, plane] : utils::enumerate(planes)) {
		//SharedFD fdd{ handle->data[i] };
		//size_t planeSize = info.planeSize(size.height, i, stride);
		size_t planeSize = info.planeSize(size, i);

		// if(planeSize > maxDmaLength) {
		// 	planeSize = maxDmaLength;
		// }

		LOG(HAL, Debug) << "Private::allocate: planeInfo i=" << i << " offset=" << offset << " size=" << planeSize;

		plane.fd = fd;
		plane.offset = offset;
		plane.length = planeSize;
		offset += planeSize;
	}

	

	return std::make_unique<HALFrameBuffer>(
		std::make_unique<GenericFrameBufferData>(
			allocDevice_, handle, planes),
		handle);
}

PUBLIC_FRAME_BUFFER_ALLOCATOR_IMPLEMENTATION
