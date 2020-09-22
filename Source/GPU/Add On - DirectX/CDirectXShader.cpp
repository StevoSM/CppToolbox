//----------------------------------------------------------------------------------------------------------------------
//	CDirectXShader.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CDirectXShader.h"

#include "CFileReader.h"

#include <DirectXMath.h>

#define Delete(x)	{ delete x; x = nil; }

using namespace DirectX;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: CDirectXVertexShaderInternals

class CDirectXVertexShaderInternals {
	public:
		CDirectXVertexShaderInternals(const CFilesystemPath& filesystemPath) :
			mFilesystemPath(filesystemPath), mShader(NULL)
			{}
		~CDirectXVertexShaderInternals()
			{
				// Cleanup
				if (mShader != NULL)
					mShader->Release();
			}

		CFilesystemPath		mFilesystemPath;

		ID3D11VertexShader*	mShader;

		XMFLOAT4X4			mModelMatrix;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDirectXVertexShader

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CDirectXVertexShader::CDirectXVertexShader(const CFilesystemPath& filesystemPath) : CGPUVertexShader()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CDirectXVertexShaderInternals(filesystemPath);
}

//----------------------------------------------------------------------------------------------------------------------
CDirectXVertexShader::~CDirectXVertexShader()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CDirectXVertexShader::setup(ID3D11Device& d3dDevice, ID3D11DeviceContext3& d3dDeviceContext,
		const XMFLOAT4X4& projectionMatrix, const XMFLOAT4X4& viewMatrix)
//----------------------------------------------------------------------------------------------------------------------
{
	// Finish setup if needed
	if (mInternals->mShader == NULL) {
		// Load data
		UError	error;
		CData	data = CFileReader::readData(CFile(mInternals->mFilesystemPath), error);
		AssertFailIf(error != kNoError);

		// Create Vertex Shader
		HRESULT	result;	
		result = d3dDevice.CreateVertexShader(data.getBytePtr(), data.getSize(), NULL, &mInternals->mShader);
		AssertFailIf(FAILED(result));

		// Setup input layout
		setupInputLayout(d3dDevice, data);
	}

	// Make current
	d3dDeviceContext.VSSetShader(mInternals->mShader, NULL, 0);
}

//----------------------------------------------------------------------------------------------------------------------
void CDirectXVertexShader::setModelMatrix(const SMatrix4x4_32& modelMatrix)
//----------------------------------------------------------------------------------------------------------------------
{
	// Convert and store
	mInternals->mModelMatrix._11 = modelMatrix.m1_1;
	mInternals->mModelMatrix._12 = modelMatrix.m1_2;
	mInternals->mModelMatrix._13 = modelMatrix.m1_3;
	mInternals->mModelMatrix._14 = modelMatrix.m1_4;
	mInternals->mModelMatrix._21 = modelMatrix.m2_1;
	mInternals->mModelMatrix._22 = modelMatrix.m2_2;
	mInternals->mModelMatrix._23 = modelMatrix.m2_3;
	mInternals->mModelMatrix._24 = modelMatrix.m2_4;
	mInternals->mModelMatrix._31 = modelMatrix.m3_1;
	mInternals->mModelMatrix._32 = modelMatrix.m3_2;
	mInternals->mModelMatrix._33 = modelMatrix.m3_3;
	mInternals->mModelMatrix._34 = modelMatrix.m3_4;
	mInternals->mModelMatrix._41 = modelMatrix.m4_1;
	mInternals->mModelMatrix._42 = modelMatrix.m4_2;
	mInternals->mModelMatrix._43 = modelMatrix.m4_3;
	mInternals->mModelMatrix._44 = modelMatrix.m4_4;
}

//----------------------------------------------------------------------------------------------------------------------
const XMFLOAT4X4& CDirectXVertexShader::getModelMatrix() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mModelMatrix;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDirectXPixelShaderInternals

class CDirectXPixelShaderInternals {
	public:
		CDirectXPixelShaderInternals(const CFilesystemPath& filesystemPath) :
			mFilesystemPath(filesystemPath), mShader(NULL)
			{}
		~CDirectXPixelShaderInternals()
			{
				// Cleanup
				if (mShader != NULL)
					mShader->Release();
			}

		CFilesystemPath		mFilesystemPath;

		ID3D11PixelShader*	mShader;
		ID3D11SamplerState*	mSamplerState;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDirectXPixelShader

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CDirectXPixelShader::CDirectXPixelShader(const CFilesystemPath& filesystemPath) : CGPUFragmentShader()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CDirectXPixelShaderInternals(filesystemPath);
}

//----------------------------------------------------------------------------------------------------------------------
CDirectXPixelShader::~CDirectXPixelShader()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CDirectXPixelShader::setup(ID3D11Device& d3dDevice, ID3D11DeviceContext& d3dDeviceContext)
//----------------------------------------------------------------------------------------------------------------------
{
	// Finish setup if needed
	if (mInternals->mShader == NULL) {
		// Load data
		UError	error;
		CData	data = CFileReader::readData(CFile(mInternals->mFilesystemPath), error);
		AssertFailIf(error != kNoError);

		// Create Pixel Shader
		HRESULT	result = d3dDevice.CreatePixelShader(data.getBytePtr(), data.getSize(), NULL, &mInternals->mShader);
		AssertFailIf(FAILED(result));

		// Setup Sampler State
		D3D11_SAMPLER_DESC	samplerDesc;
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc= D3D11_COMPARISON_NEVER;
		samplerDesc.MinLOD = -FLT_MAX;
		samplerDesc.MaxLOD = FLT_MAX;

		AssertFailIf(FAILED(d3dDevice.CreateSamplerState(&samplerDesc, &mInternals->mSamplerState)));
	}

	// Make current
	d3dDeviceContext.PSSetShader(mInternals->mShader, NULL, 0);
	d3dDeviceContext.PSSetSamplers(0, 1, &mInternals->mSamplerState);
}
