//----------------------------------------------------------------------------------------------------------------------
//	CGPUShader.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CGPUShader.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPUVertexShaderInternals

class CGPUVertexShaderInternals {
	public:
		CGPUVertexShaderInternals() {}

		CUUID	mUUID;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUVertexShader

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CGPUVertexShader::CGPUVertexShader()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CGPUVertexShaderInternals();
}

//----------------------------------------------------------------------------------------------------------------------
CGPUVertexShader::~CGPUVertexShader()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
const CUUID& CGPUVertexShader::getUUID() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mUUID;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUFragmentShaderInternals

class CGPUFragmentShaderInternals {
	public:
		CGPUFragmentShaderInternals() {}

		CUUID	mUUID;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUFragmentShader

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CGPUFragmentShader::CGPUFragmentShader()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CGPUFragmentShaderInternals();
}

//----------------------------------------------------------------------------------------------------------------------
CGPUFragmentShader::~CGPUFragmentShader()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
const CUUID& CGPUFragmentShader::getUUID() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mUUID;
}
