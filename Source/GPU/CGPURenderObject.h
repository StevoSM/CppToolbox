//----------------------------------------------------------------------------------------------------------------------
//	CGPURenderObject.h			©2018 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CGPU.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SGPURenderObjectRenderInfo

struct SGPURenderObjectRenderInfo {
	// Lifecycle methods
	SGPURenderObjectRenderInfo() {}
	SGPURenderObjectRenderInfo(const S2DOffset32& offset) : mOffset(offset) {}
	SGPURenderObjectRenderInfo(SMatrix4x1_32 clipPlane) : mClipPlane(clipPlane) {}
	SGPURenderObjectRenderInfo(const SGPURenderObjectRenderInfo& other, const S2DOffset32& offset) :
			mOffset(offset), mClipPlane(other.mClipPlane)
			{}

	// Properties
	S2DOffset32			mOffset;
	OV<SMatrix4x1_32>	mClipPlane;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPURenderObject

class CGPURenderObject {
	// Methods
	public:
						// Lifecycle methods
						CGPURenderObject() {}
						CGPURenderObject(const CGPURenderObject& other) {}
		virtual			~CGPURenderObject() {}

						// Instance methods
		virtual	void	render(CGPU& gpu,
										const SGPURenderObjectRenderInfo& renderInfo = SGPURenderObjectRenderInfo())
										const = 0;
};