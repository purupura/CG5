#pragma once

#include <string>	//wstring, string
#include <d3d12.h>	//ID3DBlob

class Shader {
public:
	//シェーダーファイルの読み込み、コンパイル済みデータを生成する
	void Load(const std::wstring& filePath, const std::string& shaderModel);

	//生成したコンパイル済みデータを取得する
	ID3DBlob* GetBlob();

	//コンストラクタ
	Shader();
	//デストラクタ
	~Shader();

private:
	ID3DBlob* blob_ = nullptr;
};
