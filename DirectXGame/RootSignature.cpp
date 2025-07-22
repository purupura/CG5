#include "RootSignature.h"
#include "KamataEngine.h" //DirectXCommon

using namespace KamataEngine;

//RootSignatureを生成する
void RootSignature::Create() {
	//既にインスタンスがあるなら解放する　　//Createメンバ関数が2度実行されたときの対処
	if (rootSignature_) {
		rootSignature_->Release();
		rootSignature_ = nullptr;
	}
	// クラス内で取得するために追加
	DirectXCommon* dxCommmon = DirectXCommon::GetInstance();

	//RootSignature作成 ---------------------------
	//　構造体にデータを用意する

	D3D12_ROOT_SIGNATURE_DESC descriptorRootSignature{};
	descriptorRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	
	//デスクリプタレンジ
	D3D12_DESCRIPTOR_RANGE srvDescRange[1]{};
	//t0 レジスタを利用可能にする
	srvDescRange[0].BaseShaderRegister = 0; // t0 レジスタを利用
	srvDescRange[0].NumDescriptors = 1;     // 1つのSRVを使用
	srvDescRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRVのタイプ
	srvDescRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // テーブルの先頭からオフセットなし

	//　RootParameterの用意　※PixelShaderに読ませるために必要　★00_09　追加
	//　複数設定できるので配列の構造をしている。今回は　1つだけなので、長さ1の配列として用意する
	D3D12_ROOT_PARAMETER rootParameters[1]{};

	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // デスクリプタテーブルを使用
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;           // ピクセルシェーダーで使用
	rootParameters[0].DescriptorTable.pDescriptorRanges = srvDescRange;           // デスクリプタレンジを設定
	rootParameters[0].DescriptorTable.NumDescriptorRanges = _countof(srvDescRange); // デスクリプタレンジの数を設定

	descriptorRootSignature.pParameters = rootParameters; // RootParameterを設定
	descriptorRootSignature.NumParameters = _countof(rootParameters); // RootParameterの数を設定

	// Samplerの設定　★00_09　追加
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // 線形フィルタリング
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // U座標のアドレスモード
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // V座標のアドレスモード
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // W座標のアドレスモード
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // 比較関数
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;                   // 最大LOD
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // ピクセルシェーダーで使用

	descriptorRootSignature.pStaticSamplers = staticSamplers; // StaticSamplerを設定
	descriptorRootSignature.NumStaticSamplers = _countof(staticSamplers); // StaticSamplerの数を設定

	ID3DBlob* signatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&descriptorRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		DebugText::GetInstance()->ConsolePrintf(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	//バイナリをもとに生成
	ID3D12RootSignature* rootSignature = nullptr;
	hr = dxCommmon->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	//SignatureBlobはRootSignatureの生成後解放してもいい
	signatureBlob->Release();

	//生成したRootSignatureをとっておく
	rootSignature_ = rootSignature;



}

//生成したRootSignatureを返す
ID3D12RootSignature* RootSignature::Get() { 
	return rootSignature_;
}

//コンストラクタ
RootSignature::RootSignature() {
}

//デストラクタ
RootSignature::~RootSignature() {
	if (rootSignature_) {
		rootSignature_->Release();
		rootSignature_ = nullptr;
	}
}
