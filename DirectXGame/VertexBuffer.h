#pragma once

#include <d3dx12.h>

class VertexBuffer {
public:
	//vertexBuffer生成
	void Create(const UINT size, const UINT stride);

	//ゲッター
	ID3D12Resource* Get();
	D3D12_VERTEX_BUFFER_VIEW* GetView();

	//コンストラクタ
	VertexBuffer();
	//デストラクタ
	~VertexBuffer();

	private:
	ID3D12Resource* vertexBuffer_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
};
