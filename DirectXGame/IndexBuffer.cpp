#include "IndexBuffer.h"
#include "kamataEngine.h"

#include<d3dx12.h>
#include<cassert>

using namespace KamataEngine;

void IndexBuffer::Create(const UINT size, const UINT stride) { 

	assert(stride==2||stride==4);
	DXGI_FORMAT format = (stride == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	// IndexResorceの生成---------------
	//インデックスリソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD; // CPUから書き込むヒープ
	// インデックスリソースの設定
	D3D12_RESOURCE_DESC indexResourceDesc{};
	indexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; // バッファ
	indexResourceDesc.Width = size;                                // リソースのサイズ。今回はVector4を3頂点分
	// バッファの場合はこれらを1にする決まり
	indexResourceDesc.Height = 1;
	indexResourceDesc.DepthOrArraySize = 1;
	indexResourceDesc.MipLevels = 1;
	indexResourceDesc.SampleDesc.Count = 1;
	// バッファの場合はこれにする決まり
	indexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// 実際に頂点リソースを生成する
	ID3D12Resource* indexResource = nullptr;
	HRESULT hr =
	    dxCommon->GetDevice()->CreateCommittedResource(
			&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,&indexResourceDesc, 
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&indexResource));
	assert(SUCCEEDED(hr)); // うまくいかなかったときは起動できない

	indexBuffer_ = indexResource;

	// IndexBufferViewを作成する----------------
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};
	// リソースの先頭アドレスから使う
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	// 使用するインデックスデータの全サイズ
	indexBufferView.SizeInBytes = size;
	//インデックスのフォーマット
	indexBufferView.Format = format;

	//IndexBufferViewをとっておく
	indexBufferView_ = indexBufferView;

}

ID3D12Resource* IndexBuffer::Get() { return indexBuffer_; }

D3D12_INDEX_BUFFER_VIEW* IndexBuffer::GetDevice() { return &indexBufferView_; }

IndexBuffer::IndexBuffer() {}

IndexBuffer::~IndexBuffer() { 
	if (indexBuffer_) {
	
	indexBuffer_->Release();
		indexBuffer_ = nullptr;
	}


}
