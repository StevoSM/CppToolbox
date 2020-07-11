//----------------------------------------------------------------------------------------------------------------------
//	CMetalShader.mm			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#import "CMetalShader.h"

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: CMetalVertexShaderInternals

class CMetalVertexShaderInternals {
	public:
		CMetalVertexShaderInternals() {}

		SMatrix4x4_32	mModelMatrix;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMetalVertexShader

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CMetalVertexShader::CMetalVertexShader() : CGPUVertexShader()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CMetalVertexShaderInternals();
}

//----------------------------------------------------------------------------------------------------------------------
CMetalVertexShader::~CMetalVertexShader()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CMetalVertexShader::setModelMatrix(const SMatrix4x4_32& modelMatrix)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mModelMatrix = modelMatrix;
}

//----------------------------------------------------------------------------------------------------------------------
const SMatrix4x4_32& CMetalVertexShader::getModelMatrix() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mModelMatrix;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMetalVertexShaderInternals

class CMetalFragmentShaderInternals {
	public:
		CMetalFragmentShaderInternals() {}
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMetalFragmentShader

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CMetalFragmentShader::CMetalFragmentShader() : CGPUFragmentShader()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CMetalFragmentShaderInternals();
}

//----------------------------------------------------------------------------------------------------------------------
CMetalFragmentShader::~CMetalFragmentShader()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}
