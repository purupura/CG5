
#include "KamataEngine.h"
#include <Windows.h>
#include <d3dcompiler.h>

using namespace KamataEngine;

//関数プロトタイプ宣言
ID3DBlob* CompileShader(const std::wstring& filePath, const std::string& shaderModel);

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// エンジンの初期化
	KamataEngine::Initialize(L"LE3D_05_カラサワ_ミクム_CG5");

	// DirectXCommonクラスが管理している、ウィンドウの幅と高さの値の取得
	int32_t w = dxCommon->GetBackBufferWidth();
	int32_t h = dxCommon->GetBackBufferHeight();
	DebugText::GetInstance()->ConsolePrintf(std::format("width: {}", "height: {}", w, h).c_str());

	// DirectXCommonクラスが管理している、コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();

	//RootSignature作成-------------------------------------------
	//構造体にデータを用意する
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = 
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	ID3D10Blob* signatureBlob = nullptr;
	ID3D10Blob* errorBlog = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1,&signatureBlob,&errorBlog);
	if (FAILED(hr)) {
		DebugText::GetInstance()->ConsolePrintf(reinterpret_cast<char*>(errorBlog->GetBufferPointer()));
		assert(false);
	}

	//バイナリをもとに生成
	ID3D12RootSignature* rootSignature = nullptr;
	hr = dxCommon->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	//InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[1] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	//BlendState ----------------------------今回は不透明
	D3D12_BLEND_DESC blendDesc {};
	//全ての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL; 

	//RasterizerState ------------------------
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	//裏面（反時計回り）をカリングする
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	//塗りつぶしモードをソリッドにする（ワイヤーフレームならD3D12＿FILL_MODE_WIREFRAME）
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	//頂点シェーダーの読み込みとコンパイル
	ID3DBlob* vsBlob = CompileShader(L"Resources/shaders/TestVS.hlsl", "vs_5_0");
	assert(vsBlob != nullptr);

	// ピクセルシェーダーの読み込みとコンパイル
	ID3DBlob* psBlob = CompileShader(L"Resources/shaders/TestPS.hlsl", "ps_5_0");
	assert(psBlob != nullptr);

	////コンパイル済みのShader..エラー時情報の格納場所の用意
	//ID3D10Blob* vsBlob = nullptr; //頂点シェーダオブジェクト
	//ID3D10Blob* psBlob = nullptr; //ピクセルシェーダオブジェクト
	//ID3D10Blob* errorBlob = nullptr; //エラーオブジェクト
	////頂点シェーダーの読み込むとコンパイル
	//std::wstring vsFile = L"Resources/shaders/TestVS.hlsl";
	//hr = D3DCompileFromFile(
	//	vsFile.c_str(),//シェーダーファイル名
	//	nullptr,
	//	D3D_COMPILE_STANDARD_FILE_INCLUDE, //インクルード可能にする
	//	"main", "vs_5_0",				   //エントリーポイント名、シェーダーモデル設定
	//	D3DCOMPILE_DEBUG|D3DCOMPILE_SKIP_OPTIMIZATION,//デッバグ用設定
	//	0,&vsBlob,&errorBlob);
	//if (FAILED(hr)) {
	//	DebugText::GetInstance()->ConsolePrintf(
	//		std::system_category().message(hr).c_str());
	//	if (errorBlob) {
	//		DebugText::GetInstance()->ConsolePrintf(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
	//	}
	//	assert(false);
	//}

	//ピクセルシェーダーの読み込みとコンパイル
	//std::wstring psFile = L"Resources/shaders/TestPS.hlsl";
	//hr = D3DCompileFromFile(
	//    psFile.c_str(), // シェーダーファイル名
	//    nullptr,
	//    D3D_COMPILE_STANDARD_FILE_INCLUDE,               // インクルード可能にする
	//    "main", "ps_5_0",                                // エントリーポイント名、シェーダーモデル設定
	//    D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デッバグ用設定
	//    0, &psBlob, &errorBlob);
	//if (FAILED(hr)) {
	//	DebugText::GetInstance()->ConsolePrintf(
	//		std::system_category().message(hr).c_str());
	//	if (errorBlob) {
	//		DebugText::GetInstance()->ConsolePrintf(
	//			reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
	//	}
	//	assert(false);
	//}

	//PSO(PipeLineStateObject)の生成
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature;  //RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;   //InputLayout
	graphicsPipelineStateDesc.VS = {vsBlob->GetBufferPointer(), vsBlob->GetBufferSize()};//vertexShader
	graphicsPipelineStateDesc.PS = {psBlob->GetBufferPointer(), psBlob->GetBufferSize()};//pixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc;		   //BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;//RasterizerState
	//書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;//1つのRTVに書き込む　*二つ同時に書き込むこともできる
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//利用するトポロジ（形状）のタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//どのように画面に色を打ち込むかの設定（今は気にしなくていい）
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	//準備は整った。PSOを生成する
	ID3D12PipelineState* graphicsPipeLineState = nullptr;
	hr = dxCommon->GetDevice()->CreateGraphicsPipelineState(
		&graphicsPipelineStateDesc,IID_PPV_ARGS(&graphicsPipeLineState));
	assert(SUCCEEDED(hr));

	//VertexResorceの生成---------------
	//頂点リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;//CPUから書き込むヒープ
	//頂点リソースの設定
	D3D12_RESOURCE_DESC vertexResourceDesc{};
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; //バッファ
	vertexResourceDesc.Width = sizeof(Vector4) * 3;					//リソースのサイズ。今回はVector4を3頂点分
	//バッファの場合はこれらを1にする決まり
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;
	//バッファの場合はこれにする決まり
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	//実際に頂点リソースを生成する
	ID3D12Resource* vertexResource = nullptr;
	hr = dxCommon->GetDevice()->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &vertexResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexResource));
	assert(SUCCEEDED(hr));//うまくいかなかったときは起動できない

	//VertexBufferViewを作成する----------------
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	//リソースの先頭アドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView.SizeInBytes = sizeof(Vector4) * 3;
	//1つの頂点サイズ
	vertexBufferView.StrideInBytes = sizeof(Vector4);
	//　頂点リソースにデータを書き込む-------------------
	Vector4* vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	vertexData[0] = {-0.5f, -0.5f, 0.0f, 1.0f};//左下
	vertexData[1] = { 0.0f,  0.5f, 0.0f, 1.0f};//上
	vertexData[2] =  {0.5f, -0.5f, 0.0f, 1.0f};//右下
	//頂点リソースのマップを解除する
	vertexResource->Unmap(0, nullptr);

	// メインループ
	while (true) {
		// エンジンの更新
		if (KamataEngine::Update()) {
			break;
		}


		// 描画開始
		dxCommon->PreDraw();


		//コマンドを積む
		commandList->SetGraphicsRootSignature(rootSignature);
		commandList->SetPipelineState(graphicsPipeLineState);
		commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
		//トポロジの設定
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//頂点数、インデックス数、インデックスの開始位置、インデックスのオフセット
		commandList->DrawInstanced(3, 1, 0, 0);

		// 描画終了
		dxCommon->PostDraw();
	}

	// 解放処理
	vertexResource->Release();
	graphicsPipeLineState->Release();
	vsBlob->Release();
	psBlob->Release();
	rootSignature->Release();
	signatureBlob->Release();
	// エラーオブジェクトの解放
	//if (errorBlob) {
	//	errorBlob->Release();
	//}
	KamataEngine::Finalize();
	return 0;
}

// シェーダーコンパイル関数
// filePath		: シェーダーファイルのパス		例　L"Resources/Shaders/TestVS.hlsl"
// shaderModel	: シェーダーモデル			例　"vs_5_0"
ID3DBlob* CompileShader(const std::wstring& filePath, const std::string& shaderModel) { 
	ID3DBlob* shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	HRESULT hr =
	    D3DCompileFromFile(
			filePath.c_str(),//シェーダーファイル名
			nullptr, 
			D3D_COMPILE_STANDARD_FILE_INCLUDE,				//インクルード可能にする
			"main", shaderModel.c_str(),					//エントリーポイント名、シェーダーモデル設定
			D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,//デバッグ用設定
			0, &shaderBlob, &errorBlob);

	//エラーが発生した場合、止める
	if (FAILED(hr)) {
		if (errorBlob) {
			OutputDebugStringA(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
			errorBlob->Release();
		}
		assert(false);
	}
	//生成したshaderBlobを返す
	return shaderBlob;
}