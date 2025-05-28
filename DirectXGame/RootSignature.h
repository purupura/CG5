#pragma once

#include <d3d12.h>
#include <cassert>
class RootSignature {
public:
	//生成
	void Create();

	//ゲッター
	ID3D12RootSignature* Get();

	//コンストラクタ
	RootSignature();
	//デストラクタ
	~RootSignature();

private:
	ID3D12RootSignature* rootSignature_ = nullptr;

};
