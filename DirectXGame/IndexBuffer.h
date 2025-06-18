#pragma once
#include <d3dx12.h> //ID3D12PipelineState

class IndexBuffer {
public:
	// 初期化
	void Create(const UINT size, const UINT stride);
	// ゲッター
	ID3D12Resource* Get();
	D3D12_INDEX_BUFFER_VIEW* GetDevice();

		// コンストラクタ
	IndexBuffer();
	// デストラクタ
	~IndexBuffer();

	private:
	ID3D12Resource* indexBuffer_ = nullptr;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};
};
