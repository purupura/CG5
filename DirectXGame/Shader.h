#pragma once

#include <string>	//wstring, string
#include <d3d12.h>	//ID3DBlob
#include "MiscUtility.h"
#include <string> 
#include <d3d12.h> 
#include <d3dcompiler.h> 
#include <dxcapi.h>

class Shader {
public:
	//シェーダーファイルの読み込み、コンパイル済みデータを生成する
	void Load(const std::wstring& filePath, const std::wstring& shaderModel);

	void LoadDxc(const std::wstring& filePath, const std::wstring& shaderModel);

	//生成したコンパイル済みデータを取得する
	ID3DBlob* GetBlob();
	IDxcBlob* GetDxcBlob();
	//コンストラクタ
	Shader();
	//デストラクタ
	~Shader();

private:
	ID3DBlob* blob_ = nullptr;
	IDxcBlob* dxcBlob_ = nullptr; 
	MiscUtility* miscUtility_ = nullptr;
};
