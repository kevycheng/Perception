/********************************************************************
Vireio Perception: Open-Source Stereoscopic 3D Driver
Copyright (C) 2012 Andres Hernandez

Vireio Matrix Modifier - Vireio Stereo Matrix Modification Node
Copyright (C) 2015 Denis Reischl

File <VireioMatrixModifier.cpp> and
Class <VireioMatrixModifier> :
Copyright (C) 2015 Denis Reischl

Parts of this class directly derive from Vireio source code originally
authored by Chris Drain (v1.1.x 2013).

The stub class <AQU_Nodus> is the only public class from the Aquilinus
repository and permitted to be used for open source plugins of any kind.
Read the Aquilinus documentation for further information.

Vireio Perception Version History:
v1.0.0 2012 by Andres Hernandez
v1.0.X 2013 by John Hicks, Neil Schneider
v1.1.x 2013 by Primary Coding Author: Chris Drain
Team Support: John Hicks, Phil Larkson, Neil Schneider
v2.0.x 2013 by Denis Reischl, Neil Schneider, Joshua Brown
v2.0.4 onwards 2014 by Grant Bagwell, Simon Brown and Neil Schneider

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************/

#define DEBUG_UINT(a) { wchar_t buf[128]; wsprintf(buf, L"%u", a); OutputDebugString(buf); }
#define DEBUG_HEX(a) { wchar_t buf[128]; wsprintf(buf, L"%x", a); OutputDebugString(buf); }

#include"VireioMatrixModifier.h"

#define INTERFACE_ID3D11DEVICE                                               6
#define INTERFACE_ID3D10DEVICE                                               7
#define INTERFACE_ID3D11DEVICECONTEXT                                        11
#define INTERFACE_IDXGISWAPCHAIN                                             29

#define METHOD_IDXGISWAPCHAIN_PRESENT                                        8
#define METHOD_ID3D11DEVICE_CREATEBUFFER                                     3
#define METHOD_ID3D11DEVICE_CREATEVERTEXSHADER                               12
#define METHOD_ID3D11DEVICECONTEXT_VSSETCONSTANTBUFFERS                      7
#define METHOD_ID3D11DEVICECONTEXT_PSSETSHADER                               9
#define METHOD_ID3D11DEVICECONTEXT_VSSETSHADER                               11
#define METHOD_ID3D11DEVICECONTEXT_PSSETCONSTANTBUFFERS                      16
#define METHOD_ID3D11DEVICECONTEXT_COPYSUBRESOURCEREGION                     46
#define METHOD_ID3D11DEVICECONTEXT_COPYRESOURCE                              47
#define METHOD_ID3D11DEVICECONTEXT_UPDATESUBRESOURCE                         48
#define METHOD_ID3D11DEVICECONTEXT_VSGETCONSTANTBUFFERS                      72
#define METHOD_ID3D11DEVICECONTEXT_PSGETCONSTANTBUFFERS                      77
#define METHOD_ID3D10DEVICE_COPYSUBRESOURCEREGION                            32
#define METHOD_ID3D10DEVICE_COPYRESOURCE                                     33
#define METHOD_ID3D10DEVICE_UPDATESUBRESOURCE                                34

#define SAFE_RELEASE(a) if (a) { a->Release(); a = nullptr; }

/**
* Constructor.
***/
MatrixModifier::MatrixModifier() : AQU_Nodus(),
	m_apcActiveConstantBuffers11(D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, nullptr)
{
	// create a new HRESULT pointer
	m_pvReturn = (void*)new HRESULT();

	// create output pointers
#if defined(VIREIO_D3D11) || defined(VIREIO_D3D10)
	m_pvOutput[STS_Commanders::ppActiveConstantBuffers_DX10_VertexShader] = (void*)&m_apcActiveConstantBuffers11[0];
#endif
}

/**
* Destructor.
***/
MatrixModifier::~MatrixModifier()
{
}

/**
* Return the name of the Matrix Modifier node.
***/
const char* MatrixModifier::GetNodeType()
{
#if defined(VIREIO_D3D11) || defined(VIREIO_D3D10)
	return "Matrix ModifierDx10";
#elif defined(VIREIO_D3D9)
	return "Matrix Modifier";
#endif
}

/**
* Returns a global unique identifier for the Matrix Modifier node.
***/
UINT MatrixModifier::GetNodeTypeId()
{
#define DEVELOPER_IDENTIFIER 2006
#if defined(VIREIO_D3D11) || defined(VIREIO_D3D10)
#define MY_PLUGIN_IDENTIFIER 76
#elif defined(VIREIO_D3D9)
#define MY_PLUGIN_IDENTIFIER 75
#endif
	return ((DEVELOPER_IDENTIFIER << 16) + MY_PLUGIN_IDENTIFIER);
}

/**
* Returns the name of the category for the Matrix Modifier node.
***/
LPWSTR MatrixModifier::GetCategory()
{
	return L"Vireio Core";
}

/**
* Returns a logo to be used for the Matrix Modifier node.
***/
HBITMAP MatrixModifier::GetLogo()
{
#if defined(VIREIO_D3D11) || defined(VIREIO_D3D10)
	HMODULE hModule = GetModuleHandle(L"VireioMatrixModifierDx10.dll");
#elif defined(VIREIO_D3D9)
	HMODULE hModule = GetModuleHandle(L"VireioMatrixModifier.dll");
#endif
	HBITMAP hBitmap = LoadBitmap(hModule, MAKEINTRESOURCE(IMG_LOGO01));
	return hBitmap;
}

/**
* Returns the updated control for the Matrix Modifier node.
* Allways return >nullptr< if there is no update for the control !!
***/
HBITMAP MatrixModifier::GetControl()
{
	return nullptr;
}

/**
* Provides the name of the requested commander.
***/
LPWSTR MatrixModifier::GetCommanderName(DWORD dwCommanderIndex)
{
#if defined(VIREIO_D3D11) || defined(VIREIO_D3D10)
	switch ((STS_Commanders)dwCommanderIndex)
	{
	case ppActiveConstantBuffers_DX10_VertexShader:
		return L"ppConstantBuffers_DX10_VS";
	case ppActiveConstantBuffers_DX11_VertexShader:
		return L"ppConstantBuffers_DX11_VS";
	case ppActiveConstantBuffers_DX10_PixelShader:
		return L"ppConstantBuffers_DX10_PS";
	case ppActiveConstantBuffers_DX11_PixelShader:
		return L"ppConstantBuffers_DX11_PS";
	default:
		break;
	}
#elif defined(VIREIO_D3D9)
#endif

	return L"UNTITLED";
}

/**
* Provides the name of the requested decommander.
***/
LPWSTR MatrixModifier::GetDecommanderName(DWORD dwDecommanderIndex)
{
#if defined(VIREIO_D3D11) || defined(VIREIO_D3D10)
	switch ((STS_Decommanders)dwDecommanderIndex)
	{
	case pShaderBytecode_VertexShader:
		return L"pShaderBytecode_VS";
	case BytecodeLength_VertexShader:
		return L"BytecodeLength_VS";
	case pClassLinkage_VertexShader:
		return L"pClassLinkage_VertexShader";
	case ppVertexShader_DX10:
		return L"ppVertexShader_DX10";
	case ppVertexShader_DX11:
		return L"ppVertexShader_DX11";
	case pShaderBytecode_PixelShader:
		return L"pShaderBytecode_PS";
	case BytecodeLength_PixelShader:
		return L"BytecodeLength_PS";
	case pClassLinkage_PixelShader:
		return L"pClassLinkage_PixelShader";
	case ppPixelShader_DX10:
		return L"ppPixelShader_DX10";
	case ppPixelShader_DX11:
		return L"ppPixelShader_DX11";
	case pVertexShader_10:
		return L"pVertexShader_10";
	case pVertexShader_11:
		return L"pVertexShader_11";
	case pPixelShader_10:
		return L"pPixelShader_10";
	case pPixelShader_11:
		return L"pPixelShader_11";
	case pDesc_DX10:
		return L"pDesc_DX10";
	case pInitialData_DX10:
		return L"pInitialData_DX10";
	case ppBuffer_DX10:
		return L"ppBuffer_DX10";
	case pDesc_DX11:
		return L"pDesc_DX11";
	case pInitialData_DX11:
		return L"pInitialData_DX11";
	case ppBuffer_DX11:
		return L"ppBuffer_DX11";
	case StartSlot_VertexShader:
		return L"StartSlot_VS";
	case NumBuffers_VertexShader:
		return L"NumBuffers_VS";
	case ppConstantBuffers_DX10_VertexShader:
		return L"ppConstantBuffers_DX10_VS";
	case ppConstantBuffers_DX11_VertexShader:
		return L"ppConstantBuffers_DX11_VS";
	case StartSlot_PixelShader:
		return L"StartSlot_PS";
	case NumBuffers_PixelShader:
		return L"NumBuffers_PS";
	case ppConstantBuffers_DX10_PixelShader:
		return L"ppConstantBuffers_DX10_PS";
	case ppConstantBuffers_DX11_PixelShader:
		return L"ppConstantBuffers_DX11_PS";
	case pDstResource_DX10:
		return L"pDstResource_DX10";
	case pDstResource_DX11:
		return L"pDstResource_DX11";
	case DstSubresource:
		return L"DstSubresource";
	case pDstBox_DX10:
		return L"pDstBox_DX10";
	case pDstBox_DX11:
		return L"pDstBox_DX11";
	case pSrcData:
		return L"pSrcData";
	case SrcRowPitch:
		return L"SrcRowPitch";
	case SrcDepthPitch:
		return L"SrcDepthPitch";
	case pDstResource_DX10_Copy:
		return L"pDstResource_DX10_Copy";
	case pSrcResource_DX10_Copy:
		return L"pSrcResource_DX10_Copy";
	case pDstResource_DX11_Copy:
		return L"pDstResource_DX11_Copy";
	case pSrcResource_DX11_Copy:
		return L"pSrcResource_DX11_Copy";
	case pDstResource_DX10_CopySub:
		return L"pDstResource_DX10_CopySub";
	case pDstResource_DX11_CopySub:
		return L"pDstResource_DX11_CopySub";
	case DstSubresource_CopySub:
		return L"DstSubresource_CopySub";
	case DstX:
		return L"DstX";
	case DstY:
		return L"DstY";
	case DstZ:
		return L"DstZ";
	case pSrcResource_DX10_CopySub:
		return L"pSrcResource_DX10_CopySub";
	case pSrcResource_DX11_CopySub:
		return L"pSrcResource_DX11_CopySub";
	case SrcSubresource:
		return L"SrcSubresource";
	case pSrcBox_DX10:
		return L"pSrcBox_DX10";
	case pSrcBox_DX11:
		return L"pSrcBox_DX11";
	default:
		return L"UNTITLED";
	}
#elif defined(VIREIO_D3D9)
	switch ((STS_Decommanders)dwDecommanderIndex)
	{
	case pShader_Vertex:
		return L"pShader_Vertex";
	case pShader_Pixel:
		return L"pShader_Pixel";
	case State:
		return L"State";
	case pMatrix:
		return L"pMatrix";
	case State_Multiply:
		return L"State_Multiply";
	case pMatrix_Multiply:
		return L"pMatrix_Multiply";
	case StartRegister_VertexShader:
		return L"StartRegister_VS";
	case pConstantData_VertexShader:
		return L"pConstantData_VS";
	case Vector4fCount_VertexShader:
		return L"Vector4fCount_VS";
	case StartRegister_PixelShader:
		return L"StartRegister_PS";
	case pConstantData_PixelShader:
		return L"pConstantData_PS";
	case Vector4fCount_PixelShader:
		return L"Vector4fCount_PS";
	}
#endif
	return L"UNTITLED";
}

/**
* Returns the plug type for the requested commander.
***/
DWORD MatrixModifier::GetCommanderType(DWORD dwCommanderIndex)
{
#if defined(VIREIO_D3D11) || defined(VIREIO_D3D10)
	switch ((STS_Commanders)dwCommanderIndex)
	{
	case ppActiveConstantBuffers_DX10_VertexShader:
		return PPNT_ID3D10BUFFER_PLUG_TYPE;
	case ppActiveConstantBuffers_DX11_VertexShader:
		return PPNT_ID3D11BUFFER_PLUG_TYPE;
	case ppActiveConstantBuffers_DX10_PixelShader:
		return PPNT_ID3D10BUFFER_PLUG_TYPE;
	case ppActiveConstantBuffers_DX11_PixelShader:
		return PPNT_ID3D11BUFFER_PLUG_TYPE;
	default:
		break;
	}
#elif defined(VIREIO_D3D9)
#endif

	return NULL;
}

/**
* Returns the plug type for the requested decommander.
***/
DWORD MatrixModifier::GetDecommanderType(DWORD dwDecommanderIndex)
{
#if defined(VIREIO_D3D11) || defined(VIREIO_D3D10)
	switch ((STS_Decommanders)dwDecommanderIndex)
	{
	case pShaderBytecode_VertexShader:
		return PNT_VOID_PLUG_TYPE;
	case BytecodeLength_VertexShader:
		return SIZE_T_PLUG_TYPE;
	case pClassLinkage_VertexShader:
		return PNT_ID3D11CLASSLINKAGE_PLUG_TYPE;
	case ppVertexShader_DX10:
		return PPNT_ID3D10VERTEXSHADER_PLUG_TYPE;
	case ppVertexShader_DX11:
		return PPNT_ID3D11VERTEXSHADER_PLUG_TYPE;
	case pShaderBytecode_PixelShader:
		return PNT_VOID_PLUG_TYPE;
	case BytecodeLength_PixelShader:
		return SIZE_T_PLUG_TYPE;
	case pClassLinkage_PixelShader:
		return PNT_ID3D11CLASSLINKAGE_PLUG_TYPE;
	case ppPixelShader_DX10:
		return PPNT_ID3D10PIXELSHADER_PLUG_TYPE;
	case ppPixelShader_DX11:
		return PPNT_ID3D11PIXELSHADER_PLUG_TYPE;
	case pVertexShader_10:
		return PNT_ID3D10VERTEXSHADER_PLUG_TYPE;
	case pVertexShader_11:
		return PNT_ID3D11VERTEXSHADER_PLUG_TYPE;
	case pPixelShader_10:
		return PNT_ID3D10PIXELSHADER_PLUG_TYPE;
	case pPixelShader_11:
		return PNT_ID3D11VERTEXSHADER_PLUG_TYPE;
	case pDesc_DX10:
		return PNT_D3D10_BUFFER_DESC_PLUG_TYPE;
	case pInitialData_DX10:
		return PNT_D3D10_SUBRESOURCE_DATA_PLUG_TYPE;
	case ppBuffer_DX10:
		return PNT_ID3D10BUFFER_PLUG_TYPE;
	case pDesc_DX11:
		return PNT_D3D11_BUFFER_DESC_PLUG_TYPE;
	case pInitialData_DX11:
		return PNT_D3D11_SUBRESOURCE_DATA_PLUG_TYPE;
	case ppBuffer_DX11:
		return PPNT_ID3D11BUFFER_PLUG_TYPE;
	case StartSlot_VertexShader:
		return UINT_PLUG_TYPE;
	case NumBuffers_VertexShader:
		return UINT_PLUG_TYPE;
	case ppConstantBuffers_DX10_VertexShader:
		return PPNT_ID3D10BUFFER_PLUG_TYPE;
	case ppConstantBuffers_DX11_VertexShader:
		return PPNT_ID3D11BUFFER_PLUG_TYPE;
	case StartSlot_PixelShader:
		return UINT_PLUG_TYPE;
	case NumBuffers_PixelShader:
		return UINT_PLUG_TYPE;
	case ppConstantBuffers_DX10_PixelShader:
		return PPNT_ID3D10BUFFER_PLUG_TYPE;
	case ppConstantBuffers_DX11_PixelShader:
		return PPNT_ID3D11BUFFER_PLUG_TYPE;
	case pDstResource_DX10:
		return PNT_ID3D10RESOURCE_PLUG_TYPE;
	case pDstResource_DX11:
		return PNT_ID3D11RESOURCE_PLUG_TYPE;
	case DstSubresource:
		return UINT_PLUG_TYPE;
	case pDstBox_DX10:
		return PNT_D3D10_BOX_PLUG_TYPE; 
	case pDstBox_DX11:
		return PNT_D3D11_BOX_PLUG_TYPE; 
	case pSrcData:
		return PNT_VOID_PLUG_TYPE; 
	case SrcRowPitch:
		return UINT_PLUG_TYPE; 
	case SrcDepthPitch:
		return UINT_PLUG_TYPE;
	case pDstResource_DX10_Copy:
		return PNT_ID3D10RESOURCE_PLUG_TYPE;
	case pSrcResource_DX10_Copy:
		return PNT_ID3D10RESOURCE_PLUG_TYPE;
	case pDstResource_DX11_Copy:
		return PNT_ID3D11RESOURCE_PLUG_TYPE;
	case pSrcResource_DX11_Copy:
		return PNT_ID3D11RESOURCE_PLUG_TYPE;
	case pDstResource_DX10_CopySub:
		return PNT_ID3D10RESOURCE_PLUG_TYPE;
	case pDstResource_DX11_CopySub:
		return PNT_ID3D11RESOURCE_PLUG_TYPE;
	case DstSubresource_CopySub:
		return UINT_PLUG_TYPE;
	case DstX:
		return UINT_PLUG_TYPE;
	case DstY:
		return UINT_PLUG_TYPE;
	case DstZ:
		return UINT_PLUG_TYPE;
	case pSrcResource_DX10_CopySub:
		return PNT_ID3D10RESOURCE_PLUG_TYPE;
	case pSrcResource_DX11_CopySub:
		return PNT_ID3D11RESOURCE_PLUG_TYPE;
	case SrcSubresource:
		return UINT_PLUG_TYPE;
	case pSrcBox_DX10:
		return PNT_D3D10_BOX_PLUG_TYPE;
	case pSrcBox_DX11:
		return PNT_D3D10_BOX_PLUG_TYPE;
	default:
		break;
	}
#elif defined(VIREIO_D3D9)
	switch ((STS_Decommanders)dwDecommanderIndex)
	{
	case pShader_Vertex:
		return PNT_IDIRECT3DVERTEXSHADER9_PLUG_TYPE;
	case pShader_Pixel:
		return PNT_IDIRECT3DPIXELSHADER9_PLUG_TYPE;
	case State:
		return D3DTRANSFORMSTATETYPE_PLUG_TYPE;
	case pMatrix:
		return PNT_D3DMATRIX_PLUG_TYPE;
	case State_Multiply:
		return D3DTRANSFORMSTATETYPE_PLUG_TYPE;
	case pMatrix_Multiply:
		return PNT_D3DMATRIX_PLUG_TYPE;
	case StartRegister_VertexShader:
		return UINT_PLUG_TYPE;
	case pConstantData_VertexShader:
		return PNT_FLOAT_PLUG_TYPE;
	case Vector4fCount_VertexShader:
		return UINT_PLUG_TYPE;
	case StartRegister_PixelShader:
		return UINT_PLUG_TYPE;
	case pConstantData_PixelShader:
		return PNT_FLOAT_PLUG_TYPE;
	case Vector4fCount_PixelShader:
		return UINT_PLUG_TYPE;
	}
#endif
	return 0;
}

/**
* Provides the output pointer for the requested commander.
***/
void* MatrixModifier::GetOutputPointer(DWORD dwCommanderIndex)
{
#if defined(VIREIO_D3D11) || defined(VIREIO_D3D10)
	switch ((STS_Commanders)dwCommanderIndex)
	{
	case ppActiveConstantBuffers_DX10_VertexShader:
		break;
	case ppActiveConstantBuffers_DX11_VertexShader:
		return (void*)&m_pvOutput[STS_Commanders::ppActiveConstantBuffers_DX10_VertexShader];//m_apcActiveConstantBuffers11[0];
	case ppActiveConstantBuffers_DX10_PixelShader:
		break;
	case ppActiveConstantBuffers_DX11_PixelShader:
		break;
	default:
		break;
	}
#elif defined(VIREIO_D3D9)
#endif

	return nullptr;
}

/**
* Sets the input pointer for the requested decommander.
***/
void MatrixModifier::SetInputPointer(DWORD dwDecommanderIndex, void* pData)
{
#if defined(VIREIO_D3D11) || defined(VIREIO_D3D10)
	switch ((STS_Decommanders)dwDecommanderIndex)
	{
	case pShaderBytecode_VertexShader:
		m_ppvShaderBytecode_VertexShader = (void**)pData;
		break;
	case BytecodeLength_VertexShader:
		m_pnBytecodeLength_VertexShader = (SIZE_T*)pData;
		break;
	case pClassLinkage_VertexShader:
		m_ppcClassLinkage_VertexShader = (ID3D11ClassLinkage**)pData;
		break;
	case ppVertexShader_DX10:
		m_pppcVertexShader_DX10 = (ID3D10VertexShader***)pData;
		break;
	case ppVertexShader_DX11:
		m_pppcVertexShader_DX11 = (ID3D11VertexShader***)pData;
		break;
	case pShaderBytecode_PixelShader:
		m_ppvShaderBytecode_PixelShader = (void**)pData;
		break;
	case BytecodeLength_PixelShader:
		m_pnBytecodeLength_PixelShader = (SIZE_T*)pData;
		break;
	case pClassLinkage_PixelShader:
		m_ppcClassLinkage_PixelShader = (ID3D11ClassLinkage**)pData;
		break;
	case ppPixelShader_DX10:
		m_pppcPixelShader_DX10 = (ID3D10PixelShader***)pData;
		break;
	case ppPixelShader_DX11:
		m_pppcPixelShader_DX11 = (ID3D11PixelShader***)pData;
		break;
	case pVertexShader_10:
		m_ppcVertexShader_10 = (ID3D10VertexShader**)pData;
		break;
	case pVertexShader_11:
		m_ppcVertexShader_11 = (ID3D11VertexShader**)pData;
		break;
	case pPixelShader_10:
		m_ppcPixelShader_10 = (ID3D10PixelShader**)pData;
		break;
	case pPixelShader_11:
		m_ppcPixelShader_11 = (ID3D11VertexShader**)pData;
		break;
	case pDesc_DX10:
		m_ppsDesc_DX10 = (D3D10_BUFFER_DESC**)pData;
		break;
	case pInitialData_DX10:
		m_ppsInitialData_DX10 = (D3D10_SUBRESOURCE_DATA**)pData;
		break;
	case ppBuffer_DX10:
		m_pppcBuffer_DX10 = (ID3D10Buffer***)pData;
		break;
	case pDesc_DX11:
		m_ppsDesc_DX11 = (D3D11_BUFFER_DESC**)pData;
		break;
	case pInitialData_DX11:
		m_ppsInitialData_DX11 = (D3D11_SUBRESOURCE_DATA**)pData;
		break;
	case ppBuffer_DX11:
		m_pppcBuffer_DX11 = (ID3D11Buffer***)pData;
		break;
	case StartSlot_VertexShader:
		m_pdwStartSlot_VertexShader = (UINT*)pData;
		break;
	case NumBuffers_VertexShader:
		m_pdwNumBuffers_VertexShader = (UINT*)pData;
		break;
	case ppConstantBuffers_DX10_VertexShader:
		m_pppcConstantBuffers_DX10_VertexShader = (ID3D10Buffer***)pData;
		break;
	case ppConstantBuffers_DX11_VertexShader:
		m_pppcConstantBuffers_DX11_VertexShader = (ID3D11Buffer***)pData;
		break;
	case StartSlot_PixelShader:
		m_pdwStartSlot_PixelShader = (UINT*)pData;
		break;
	case NumBuffers_PixelShader:
		m_pdwNumBuffers_PixelShader = (UINT*)pData;
		break;
	case ppConstantBuffers_DX10_PixelShader:
		m_pppcConstantBuffers_DX10_PixelShader = (ID3D10Buffer***)pData;
		break;
	case ppConstantBuffers_DX11_PixelShader:
		m_pppcConstantBuffers_DX11_PixelShader = (ID3D11Buffer***)pData;
		break;
	case pDstResource_DX10:
		m_ppcDstResource_DX10 = (ID3D10Resource**)pData;
		break;
	case pDstResource_DX11:
		m_ppcDstResource_DX11 = (ID3D11Resource**)pData;
		break;
	case DstSubresource:
		m_pdwDstSubresource = (UINT*)pData;
		break;
	case pDstBox_DX10:
		m_ppsDstBox_DX10 = (D3D10_BOX**)pData; 
		break;
	case pDstBox_DX11:
		m_ppsDstBox_DX11 = (D3D11_BOX**)pData; 
		break;
	case pSrcData:
		m_ppvSrcData = (void**)pData; 
		break;
	case SrcRowPitch:
		m_pdwSrcRowPitch = (UINT*)pData; 
		break;
	case SrcDepthPitch:
		m_pdwSrcDepthPitch = (UINT*)pData;
		break;
	case pDstResource_DX10_Copy:
		m_ppcDstResource_DX10_Copy = (ID3D10Resource**)pData;
		break;
	case pSrcResource_DX10_Copy:
		m_ppcSrcResource_DX10_Copy = (ID3D10Resource**)pData;
		break;
	case pDstResource_DX11_Copy:
		m_ppcDstResource_DX11_Copy = (ID3D11Resource**)pData;
		break;
	case pSrcResource_DX11_Copy:
		m_ppcSrcResource_DX11_Copy = (ID3D11Resource**)pData;
		break;
	case pDstResource_DX10_CopySub:
		m_ppcDstResource_DX10_CopySub = (ID3D10Resource**)pData;
		break;
	case pDstResource_DX11_CopySub:
		m_ppcDstResource_DX11_CopySub = (ID3D11Resource**)pData;
		break;
	case DstSubresource_CopySub:
		m_pdwDstSubresource_CopySub = (UINT*)pData;
		break;
	case DstX:
		m_pdwDstX = (UINT*)pData;
		break;
	case DstY:
		m_pdwDstY = (UINT*)pData;
		break;
	case DstZ:
		m_pdwDstZ = (UINT*)pData;
		break;
	case pSrcResource_DX10_CopySub:
		m_ppcSrcResource_DX10_CopySub = (ID3D10Resource**)pData;
		break;
	case pSrcResource_DX11_CopySub:
		m_ppcSrcResource_DX11_CopySub = (ID3D11Resource**)pData;
		break;
	case SrcSubresource:
		m_pdwSrcSubresource = (UINT*)pData;
		break;
	case pSrcBox_DX10:
		m_ppsSrcBox_DX10 = (D3D10_BOX**)pData;
		break;
	case pSrcBox_DX11:
		m_ppsSrcBox_DX11 = (D3D10_BOX**)pData;
		break;
	default:
		break;
	}
#elif defined(VIREIO_D3D9)
	switch ((STS_Decommanders)dwDecommanderIndex)
	{
	case pShader_Vertex:
		m_ppcShader_Vertex = (IDirect3DVertexShader9**)pData;
		break;
	case pShader_Pixel:
		m_ppcShader_Pixel = (IDirect3DPixelShader9**)pData;
		break;
	case State:
		m_psState = (D3DTRANSFORMSTATETYPE*)pData;
		break;
	case pMatrix:
		m_ppsMatrix = (D3DMATRIX**)pData;
		break;
	case State_Multiply:
		m_psState_Multiply = (D3DTRANSFORMSTATETYPE*)pData;
		break;
	case pMatrix_Multiply:
		m_ppsMatrix_Multiply = (D3DMATRIX**)pData;
		break;
	case StartRegister_VertexShader:
		m_pdwStartRegister_VertexShader = (UINT*)pData;
		break;
	case pConstantData_VertexShader:
		m_ppfConstantData_VertexShader = (float**)pData;
		break;
	case Vector4fCount_VertexShader:
		m_pdwVector4fCount_VertexShader = (UINT*)pData;
		break;
	case StartRegister_PixelShader:
		m_pdwStartRegister_PixelShader = (UINT*)pData;
		break;
	case pConstantData_PixelShader:
		m_ppfConstantData_PixelShader = (float**)pData;
		break;
	case Vector4fCount_PixelShader:
		m_pdwVector4fCount_PixelShader = (UINT*)pData;
		break;
	}
#endif
}

/**
* Matrix Modifier supports various D3D10 + D3D11 calls.
***/
bool MatrixModifier::SupportsD3DMethod(int nD3DVersion, int nD3DInterface, int nD3DMethod)
{
#if defined(VIREIO_D3D11) || defined(VIREIO_D3D10)
	if ((nD3DVersion >= (int)AQU_DirectXVersion::DirectX_10) &&
		(nD3DVersion <= (int)AQU_DirectXVersion::DirectX_10_1))
	{
		if (nD3DInterface == INTERFACE_ID3D10DEVICE)
		{

		}
		else if (nD3DInterface == INTERFACE_IDXGISWAPCHAIN)
		{

		}
	}
	else if ((nD3DVersion >= (int)AQU_DirectXVersion::DirectX_11) &&
		(nD3DVersion <= (int)AQU_DirectXVersion::DirectX_11_2))
	{
		if (nD3DInterface == INTERFACE_ID3D11DEVICE)
		{
			if ((nD3DMethod == METHOD_ID3D11DEVICE_CREATEVERTEXSHADER) ||
				(nD3DMethod == METHOD_ID3D11DEVICE_CREATEBUFFER))
				return true;
		}
		else if (nD3DInterface == INTERFACE_ID3D11DEVICECONTEXT)
		{
			if ((nD3DMethod == METHOD_ID3D11DEVICECONTEXT_VSSETSHADER) ||
				(nD3DMethod == METHOD_ID3D11DEVICECONTEXT_VSSETCONSTANTBUFFERS))
				return true;
		}
		else if (nD3DInterface == INTERFACE_IDXGISWAPCHAIN)
		{

		}
	}
#elif defined(VIREIO_D3D9)
#endif
	return false;
}

/**
* Handle Stereo Render Targets (+Depth Buffers).
***/
void* MatrixModifier::Provoke(void* pThis, int eD3D, int eD3DInterface, int eD3DMethod, DWORD dwNumberConnected, int& nProvokerIndex)
{
#if defined(VIREIO_D3D11) || defined(VIREIO_D3D10)
	switch (eD3DInterface)
	{
	case INTERFACE_ID3D11DEVICE:
		switch (eD3DMethod)
		{
		case METHOD_ID3D11DEVICE_CREATEVERTEXSHADER:
			// CreateVertexShader(const void *pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage *pClassLinkage, ID3D11VertexShader **ppVertexShader);

			if (!pThis) return nullptr;
			if (!m_ppvShaderBytecode_VertexShader) return nullptr;
			if (!m_pnBytecodeLength_VertexShader) return nullptr;
			if (!m_ppcClassLinkage_VertexShader) return nullptr;
			if (!m_pppcVertexShader_DX11) return nullptr;

			{
				// create the shader
				*(HRESULT*)m_pvReturn = ((ID3D11Device*)pThis)->CreateVertexShader(*m_ppvShaderBytecode_VertexShader,
					*m_pnBytecodeLength_VertexShader,
					*m_ppcClassLinkage_VertexShader,
					*m_pppcVertexShader_DX11);

				// get the shader pointer
				ID3D11VertexShader* pcShader = nullptr;
				if (*m_pppcVertexShader_DX11)
					if (**m_pppcVertexShader_DX11)
						pcShader = **m_pppcVertexShader_DX11;
				if (pcShader)
				{
					// get the hash code
					DWORD dwHashCode = GetHashCode((BYTE*)*m_ppvShaderBytecode_VertexShader, (DWORD)*m_pnBytecodeLength_VertexShader);

					// is this shader already enumerated ?
					for (size_t nShaderDescIndex = 0; nShaderDescIndex < m_asShaders.size(); nShaderDescIndex++)
					{
						if (dwHashCode == m_asShaders[nShaderDescIndex].dwHashCode)
						{
							// create and set private shader data
							Vireio_Shader_Private_Data sPrivateData;
							sPrivateData.dwHash = dwHashCode;
							sPrivateData.dwIndex = (UINT)nShaderDescIndex;

							pcShader->SetPrivateData(PDID_ID3D11VertexShader_Vireio_Data, sizeof(sPrivateData), (void*)&sPrivateData);

							// method replaced, immediately return (= behavior -16)
							nProvokerIndex = -16;
							return m_pvReturn;
						}
					}

					// create reflection class
					ID3D11ShaderReflection* pcReflector = NULL;
					if (SUCCEEDED(D3DReflect(*m_ppvShaderBytecode_VertexShader,
						*m_pnBytecodeLength_VertexShader,
						IID_ID3D11ShaderReflection,
						(void**) &pcReflector)))
					{
						// get desc
						D3D11_SHADER_DESC sDesc;
						pcReflector->GetDesc( &sDesc );

						// fill shader data
						Vireio_D3D11_Shader sShaderData;
						sShaderData.dwConstantBuffers = sDesc.ConstantBuffers;
						sShaderData.dwVersion = sDesc.Version;
						sShaderData.dwBoundResources = sDesc.BoundResources;
						sShaderData.dwInputParameters = sDesc.InputParameters;
						sShaderData.dwOutputParameters = sDesc.OutputParameters; 
						sShaderData.dwHashCode = dwHashCode;

						// get name size, max to VIREIO_MAX_VARIABLE_NAME_LENGTH
						UINT dwLen = (UINT)strnlen_s(sDesc.Creator, VIREIO_MAX_VARIABLE_NAME_LENGTH - 1);
						CopyMemory(sShaderData.szCreator, sDesc.Creator, sizeof(CHAR)*dwLen);
						sShaderData.szCreator[dwLen] = 0;

						for (UINT dwIndex = 0; dwIndex < sDesc.ConstantBuffers; dwIndex++)
						{
							// get next constant buffer
							ID3D11ShaderReflectionConstantBuffer* pcConstantBuffer = pcReflector->GetConstantBufferByIndex(dwIndex);
							if (pcConstantBuffer)
							{
								// get desc
								D3D11_SHADER_BUFFER_DESC sDescBuffer;
								pcConstantBuffer->GetDesc(&sDescBuffer);

								// fill buffer data
								Vireio_D3D11_Constant_Buffer sBufferData;
								sBufferData.eType = sDescBuffer.Type;
								sBufferData.dwVariables = sDescBuffer.Variables;
								sBufferData.dwSize = sDescBuffer.Size;
								sBufferData.dwFlags = sDescBuffer.uFlags;

								// get name size, max to VIREIO_MAX_VARIABLE_NAME_LENGTH
								dwLen = (UINT)strnlen_s(sDescBuffer.Name, VIREIO_MAX_VARIABLE_NAME_LENGTH - 1);
								CopyMemory(sBufferData.szName, sDescBuffer.Name, sizeof(CHAR)*dwLen);
								sBufferData.szName[dwLen] = 0;

								// enumerate variables
								for (UINT dwIndexVariable = 0; dwIndexVariable < sDescBuffer.Variables; dwIndexVariable++)
								{
									ID3D11ShaderReflectionVariable* pcVariable = pcConstantBuffer->GetVariableByIndex(dwIndexVariable);
									if (pcVariable)
									{
										// get desc
										D3D11_SHADER_VARIABLE_DESC sDescVariable;
										pcVariable->GetDesc(&sDescVariable);

										// fill variable data
										Vireio_D3D11_Shader_Variable sVariableData;
										sVariableData.dwSize = sDescVariable.Size;
										sVariableData.dwStartOffset = sDescVariable.StartOffset;

										// get name size, max to VIREIO_MAX_VARIABLE_NAME_LENGTH
										dwLen = (UINT)strnlen_s(sDescVariable.Name, VIREIO_MAX_VARIABLE_NAME_LENGTH - 1);
										CopyMemory(sVariableData.szName, sDescVariable.Name, sizeof(CHAR)*dwLen);
										sVariableData.szName[dwLen] = 0;

										// TODO !! FILL DEFAULT VALUE sVariableData.pcDefaultValue

										// and add to buffer desc
										sBufferData.asVariables.push_back(sVariableData);
									}
								}

								// and add to shader desc
								sShaderData.asBuffers.push_back(sBufferData);
							}
						}

						// and add to shader vector
						m_asShaders.push_back(sShaderData);

						// create and set private shader data
						Vireio_Shader_Private_Data sPrivateData;
						sPrivateData.dwHash = dwHashCode;
						sPrivateData.dwIndex = (UINT)m_asShaders.size() - 1;

						pcShader->SetPrivateData(PDID_ID3D11VertexShader_Vireio_Data, sizeof(sPrivateData), (void*)&sPrivateData);

						pcReflector->Release();
					}
				}

				// method replaced, immediately return (= behavior -16)
				nProvokerIndex = -16;
				return m_pvReturn;
			}
		case METHOD_ID3D11DEVICE_CREATEBUFFER:
			if (!m_ppsDesc_DX11) return nullptr;
			if (!m_ppsInitialData_DX11) return nullptr;
			if (!m_pppcBuffer_DX11) return nullptr;

			// is this a constant buffer ?
			if (!*m_ppsDesc_DX11) return nullptr;
			if (((*m_ppsDesc_DX11)->BindFlags & D3D11_BIND_CONSTANT_BUFFER) == D3D11_BIND_CONSTANT_BUFFER)
			{
				// create the buffer
				*(HRESULT*)m_pvReturn = ((ID3D11Device*)pThis)->CreateBuffer(*m_ppsDesc_DX11,
					*m_ppsInitialData_DX11,
					*m_pppcBuffer_DX11);

				// succeeded ?
				if (SUCCEEDED(*(HRESULT*)m_pvReturn)) 
				{
					// create left buffer
					ID3D11Buffer* pcBufferLeft = nullptr;
					if (FAILED(((ID3D11Device*)pThis)->CreateBuffer(*m_ppsDesc_DX11,
						*m_ppsInitialData_DX11,
						&pcBufferLeft)))
						OutputDebugString(L"MatrixModifier: Failed to create left buffer!");

					// create right buffer
					ID3D11Buffer* pcBufferRight = nullptr;
					if (FAILED(((ID3D11Device*)pThis)->CreateBuffer(*m_ppsDesc_DX11,
						*m_ppsInitialData_DX11,
						&pcBufferRight)))
						OutputDebugString(L"MatrixModifier: Failed to create left buffer!");

					// set as private data interface to the main buffer
					(**m_pppcBuffer_DX11)->SetPrivateDataInterface(PDIID_ID3D11Buffer_Constant_Buffer_Left, pcBufferLeft);
					(**m_pppcBuffer_DX11)->SetPrivateDataInterface(PDIID_ID3D11Buffer_Constant_Buffer_Right, pcBufferRight);

					// reference counter must be 1 now (reference held by the main buffer)
					ULONG nRef = pcBufferLeft->Release();
					if (nRef != 1) OutputDebugString(L"MatrixModifier: Reference counter invalid !");
					nRef = pcBufferRight->Release();
					if (nRef != 1) OutputDebugString(L"MatrixModifier: Reference counter invalid !");
				}

				// method replaced, immediately return (= behavior -16)
				nProvokerIndex = -16;
				return m_pvReturn;
			}
			else return nullptr;
		}
		return nullptr;
	case INTERFACE_ID3D11DEVICECONTEXT:
		switch(eD3DMethod)
		{
		case METHOD_ID3D11DEVICECONTEXT_VSSETSHADER:
			if (!m_ppcVertexShader_11) return nullptr;
			if (!*m_ppcVertexShader_11) return nullptr;
			else
			{
				Vireio_Shader_Private_Data sPrivateData;
				UINT dwDataSize =  sizeof(sPrivateData);
				(*(m_ppcVertexShader_11))->GetPrivateData(PDID_ID3D11VertexShader_Vireio_Data, &dwDataSize, (void*)&sPrivateData);
				//OutputDebugStringA(m_asShaders[sPrivateData.dwIndex].szCreator);
			}
			return nullptr;
		case METHOD_ID3D11DEVICECONTEXT_VSSETCONSTANTBUFFERS:
			if (!m_pdwStartSlot_VertexShader) return nullptr;
			if (!m_pdwNumBuffers_VertexShader) return nullptr;
			if (!m_pppcConstantBuffers_DX11_VertexShader) return nullptr;
			if (!*m_pppcConstantBuffers_DX11_VertexShader) return nullptr;
			if (!**m_pppcConstantBuffers_DX11_VertexShader) return nullptr;

			// loop through the new buffers
			for (UINT dwIndex = 0; dwIndex < *m_pdwNumBuffers_VertexShader; dwIndex++)
			{
				// get internal index
				UINT dwInternalIndex = dwIndex+*m_pdwStartSlot_VertexShader;

				// in range ? set buffer internally 
				if (dwInternalIndex < D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT)
				{
					m_apcActiveConstantBuffers11[dwInternalIndex] = ((*m_pppcConstantBuffers_DX11_VertexShader)[dwIndex]);

					// has this buffer private data set ? test for private data
					if (m_apcActiveConstantBuffers11[dwInternalIndex])
					{
						Vireio_Shader_Private_Data sPrivateData;
						UINT dwDataSize =  sizeof(sPrivateData);
						m_apcActiveConstantBuffers11[dwInternalIndex]->GetPrivateData(PDID_ID3D11Buffer_Vireio_Data, &dwDataSize, (void*)&sPrivateData);
#ifdef _DEBUG
						if (!dwDataSize)
							OutputDebugString(L"MatrixModifier: Buffer has NO private data set !");
#endif
						if (dwDataSize)
						{
							// TODO !! GET THE CONSTANT TABLE DESC AND SET AS PRIVATE DATA
						}
					}
				}
			}
			return nullptr;
		}
		return nullptr;
	}
#elif defined(VIREIO_D3D9)
#endif

	return nullptr;
}