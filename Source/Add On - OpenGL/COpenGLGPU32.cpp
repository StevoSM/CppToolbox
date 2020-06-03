//----------------------------------------------------------------------------------------------------------------------
//	COpenGLGPU32.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "COpenGLGPU.h"

#include "COpenGLTexture.h"

#include <OpenGL/gl3.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPUInternals

class CGPUInternals {
	public:
		CGPUInternals(const CGPUProcsInfo& procsInfo) : mProcsInfo(procsInfo) {}

	CGPUProcsInfo	mProcsInfo;

	SMatrix4x4_32	mProjectionMatrix;
	SMatrix4x4_32	mViewMatrix;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPU

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CGPU::CGPU(const CGPUProcsInfo& procsInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CGPUInternals(procsInfo);
}

//----------------------------------------------------------------------------------------------------------------------
CGPU::~CGPU()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CGPU methods

//----------------------------------------------------------------------------------------------------------------------
SGPUTextureReference CGPU::registerTexture(const CData& data, EGPUTextureDataFormat gpuTextureDataFormat,
		const S2DSizeU16& size)
//----------------------------------------------------------------------------------------------------------------------
{
	// Register texture
	mInternals->mProcsInfo.acquireContext();
	CGPUTexture*	gpuTexture = new COpenGLTexture(data, gpuTextureDataFormat, size);
	mInternals->mProcsInfo.releaseContext();

	return SGPUTextureReference(*gpuTexture);
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::unregisterTexture(SGPUTextureReference& gpuTexture)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	COpenGLTexture*	openGLTexture = (COpenGLTexture*) gpuTexture.mGPUTexture;
	gpuTexture.reset();

	// Cleanup
	mInternals->mProcsInfo.acquireContext();
	Delete(openGLTexture);
	mInternals->mProcsInfo.releaseContext();
}

//----------------------------------------------------------------------------------------------------------------------
SGPUVertexBuffer CGPU::allocateVertexBuffer(const SGPUVertexBufferInfo& gpuVertexBufferInfo, UInt32 vertexCount)
//----------------------------------------------------------------------------------------------------------------------
{
	return SGPUVertexBuffer(gpuVertexBufferInfo, CData(gpuVertexBufferInfo.mTotalSize * vertexCount), nil);
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::disposeBuffer(const SGPUBuffer& buffer)
//----------------------------------------------------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::renderStart() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get info
	S2DSizeU16	size = mInternals->mProcsInfo.getSize();
	Float32		scale = mInternals->mProcsInfo.getScale();

	// Update
	mInternals->mProjectionMatrix =
			SMatrix4x4_32::makeOrthographicProjection(0.0, size.mWidth, size.mHeight, 0.0, -1.0, 1.0);

	// Prepare to render
	mInternals->mProcsInfo.acquireContext();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(0, 0, size.mWidth * (GLsizei) scale, size.mHeight * (GLsizei) scale);
#if defined(DEBUG)
	glClearColor(0.5, 0.0, 0.25, 1.0);
#else
	glClearColor(0.0, 0.0, 0.0, 1.0);
#endif
	glClear(GL_COLOR_BUFFER_BIT);
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::setViewMatrix(const SMatrix4x4_32& viewMatrix)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mViewMatrix = viewMatrix;
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::renderTriangleStrip(CGPURenderState& renderState, const SMatrix4x4_32& modelMatrix, UInt32 triangleCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup render state
	renderState.setProjectionMatrix(mInternals->mProjectionMatrix);
	renderState.setViewMatrix(mInternals->mViewMatrix);
	renderState.setModelMatrix(modelMatrix);

	// Commit
	renderState.commit();

	// Draw
	glDrawArrays(GL_TRIANGLE_STRIP, 0, triangleCount + 2);
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::renderEnd() const
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mProcsInfo.releaseContext();
}