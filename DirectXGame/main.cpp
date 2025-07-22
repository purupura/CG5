
#include "KamataEngine.h"
#include "Shader.h"
#include <Windows.h>
//#include <d3dcompiler.h>
#include <cassert>
#include "RootSignature.h"
#include "PipelineState.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

using namespace KamataEngine;

// pipelineStateObjectno生成
void SetupPipelineState(PipelineState& pipelineState,RootSignature& rs,Shader& vs,Shader& ps);
//RenderTargetResourceの生成
ID3D12Resource* CreateRenderTargetResource(ID3D12Device* device, int32_t width,
	uint32_t height, DXGI_FORMAT format,const FLOAT* clearColor);
ID3D12Resource* CreateRenderTextureResource(ID3D12Device* device, 
	int32_t width, uint32_t height, DXGI_FORMAT format, const FLOAT* clearColor);
// DepthStencilTextureResourceの生成
ID3D12Resource* CreateDepthStencilTextureResource(ID3D12Device* device, 
	int32_t width, uint32_t height);

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
	RootSignature rs;
	rs.Create();

	//頂点シェーダーの読み込みとコンパイル
	Shader vs;
	vs.LoadDxc(L"Resources/shaders/TestVS.hlsl", L"vs_6_0");
	assert(vs.GetDxcBlob() != nullptr);

	// ピクセルシェーダーの読み込みとコンパイル
	Shader ps;
	ps.LoadDxc(L"Resources/shaders/TestPS.hlsl", L"ps_6_0");
	assert(ps.GetDxcBlob() != nullptr);

	////PSO(PipeLineStateObject)の生成
	PipelineState pipelineState;
	SetupPipelineState(pipelineState, rs, vs, ps);

	//リソースの確保含め、頂点情報を柔軟に対応できるようにVertexData構造体を新たに作成する
	//Vertex4 =>VertexDataに変更して利用する
	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
	};

	//頂点データの準備　★00_07　追加

VertexData vertices[] = {
	    {{-1.0f, 1.0f, 0.0f, 1.0f}, {0.0f,0.0f}}, // 左上
	    {{1.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, // 右上
	    {{-1.0f, -1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // 左下
	    {{1.0f, -1.0f, 0.0f, 1.0f},  {1.0f, 1.0f}}, // 右下
	};


	VertexBuffer vb;
	//vb.Create(sizeof(Vector4) * 3, sizeof(Vector4));
	vb.Create(sizeof(vertices), sizeof(vertices[0]));

	//頂点リソースにデータを書き込む----------------- ★00_07　追加
	VertexData* pGpuVertices = nullptr;
	vb.Get()->Map(0, nullptr, reinterpret_cast<void**>(&pGpuVertices));

	for (int i = 0; i < _countof(vertices); ++i) {
		pGpuVertices[i] = vertices[i];
	}

	//頂点インデックスデータの準備--------★00_07 追加
	uint16_t indices[] = {
	    0, 1, 2, // 左上三角形
	    2, 1, 3  // 右下三角形
	};


	//IndexBuffer(IndexResource,IndexResouceView)の生成
	IndexBuffer ib;
	ib.Create(sizeof(indices), sizeof(indices[0]));

	//頂点インデックスリソースにデータを書き込む
	uint16_t* pGpuIndices = nullptr;
	ib.Get()->Map(0, nullptr, reinterpret_cast<void**>(&pGpuIndices));

	ID3D12Device* device = dxCommon->GetDevice();
	HRESULT hr;

	//=======================================================
	//RenderTexture関係 ★00_09　追加

	//-------------------------------------------------------
	//0.RenderTextureResourceの作成

	const FLOAT kRenderTargetClearColor[] = {1.0f, 0.0f, 0.0f, 1.0f};

	ID3D12Resource* renderTextureResource = CreateRenderTargetResource(device, WinApp::kWindowWidth, WinApp::kWindowHeight, 
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, kRenderTargetClearColor);

	//-------------------------------------------------------
	// 1.RTV用のDescriptorHeapを作成する
	ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;

	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc{};
	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // RenderTargetView用のヒープ
	rtvDescriptorHeapDesc.NumDescriptors = 1;                    // 1つのRTVを作成

	hr = device->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));
	assert(SUCCEEDED(hr)); // 成功したか確認

	// CPU側からみたHANDLEを取得しておく
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandleCPU = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	//-------------------------------------------------------
	// 2.RTV用のViewの生成

	device->CreateRenderTargetView(
	    renderTextureResource, // 作成するRenderTextureResource
	    nullptr,        
		// クリア値はなし
	    rtvHandleCPU              // 作成したRTVのハンドル
	);

	for (int i = 0; i < _countof(indices); ++i) {
		pGpuIndices[i] = indices[i];
	}

	//========================================================
	// DepthStencilTexture関係 ★00_09　追加

	//-------------------------------------------------------
	// 0.DepthStencilTextureResourceの作成
	ID3D12Resource* depthStencilResource = CreateDepthStencilTextureResource(
		device, WinApp::kWindowWidth, WinApp::kWindowHeight);

	//-------------------------------------------------------
	// 1.DSV用のDescriptorHeapの作成
	ID3D12DescriptorHeap* dsvDescriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc{};
	dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV; // DepthStencilView用のヒープ
	dsvDescriptorHeapDesc.NumDescriptors = 1;                    // 1つのDSVを作成
	dsvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // フラグはなし

	hr = device->CreateDescriptorHeap(&dsvDescriptorHeapDesc, IID_PPV_ARGS(&dsvDescriptorHeap));
	assert(SUCCEEDED(hr)); // 成功したか確認

	// CPU側からみたDSVのハンドルを取得しておく
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandleCPU = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	//-------------------------------------------------------
	// 2.DSV用のViewの生成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT; // 深度値のフォーマット
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; // 2次元テクスチャ用のDSV

	device->CreateDepthStencilView(
		depthStencilResource, // 作成するDepthStencilTextureResource
		&dsvDesc,             // DSVの設定
		dsvHandleCPU             // 作成したDSVのハンドル
	);

	//========================================================
	// SRV(SHADER Resource View)を準備する　※ PixelShaderと連携をとるようにするため ★00_09　追加

	//-------------------------------------------------------
	// 1.SRV用のDescriptorHeapの作成
	ID3D12DescriptorHeap* srvDescriptorHeap = nullptr;

	D3D12_DESCRIPTOR_HEAP_DESC srvDescriptorHeapDesc = {};
	srvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; // SRV用のヒープ
	srvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // シェーダーから見えるようにする
	srvDescriptorHeapDesc.NumDescriptors = 1;                            // 1つのSRVを作成

	hr = device->CreateDescriptorHeap(&srvDescriptorHeapDesc, IID_PPV_ARGS(&srvDescriptorHeap));
	assert(SUCCEEDED(hr)); // 成功したか確認

	// CPU側からみたHANDLE,GPU側からみたHANDLEを取得しておく
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU = srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

	//-------------------------------------------------------
	// 2.SRV(Shader Resource View)の作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // RenderTextureのフォーマット
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING; // シェーダーのコンポーネントマッピング
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2次元テクスチャ用のSRV
	srvDesc.Texture2D.MipLevels = 1;                                            // mipmapの数

	device->CreateShaderResourceView(
		renderTextureResource, // 作成するRenderTextureResource
		&srvDesc,              // SRVの設定
		srvHandleCPU           // 作成したSRVのCPUハンドル
	);

	////　頂点リソースにデータを書き込む-------------------
	//Vector4* vertexData = nullptr;
	//vb.Get()->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	//vertexData[0] = {-0.5f, -0.5f, 0.0f, 1.0f};//左下
	//vertexData[1] = { 0.0f,  0.5f, 0.0f, 1.0f};//上
	//vertexData[2] =  {0.5f, -0.5f, 0.0f, 1.0f};//右下

	//　アプリで利用する3Dモデル　==========================
	//　複写体の準備
	Model* model = Model::CreateFromOBJ("terrain");

	// メインループ
	while (true) {
		// エンジンの更新
		if (KamataEngine::Update()) {
			break;
		}


		// 描画開始
		//dxCommon->PreDraw();

		// TransitionBarrierを SRV=>RTVに設定する
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION; // トランジションバリア
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;      // フラグはなし
		barrier.Transition.pResource = renderTextureResource;  // 対象のリソース
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE; // 前の状態はSRV
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;          // 次の状態はRTV
		commandList->ResourceBarrier(1, &barrier);                                   // バリアをコマンドリストに積む

		// 描画先の RTV と DSV を設定する
		commandList->OMSetRenderTargets(1, &rtvHandleCPU, false, &dsvHandleCPU);

		// VIewportの設定
		D3D12_VIEWPORT viewport{};
		viewport.Width = WinApp::kWindowWidth;
		viewport.Height = WinApp::kWindowHeight;
		viewport.TopLeftX = 0.0f; // 左上のX座標
		viewport.TopLeftY = 0.0f; // 左上のY座標
		viewport.MinDepth = 0.0f; // 最小深度値
		viewport.MaxDepth = 1.0f; // 最大深度値

		commandList->RSSetViewports(1, &viewport); // Viewportをコマンドリストに設定

		// Scissorの設定
		D3D12_RECT scissorRect{};
		// 基本的にビューポートと同じ矩形が構成されるようにする
		scissorRect.left = 0; // 左端のX座標
		scissorRect.right = WinApp::kWindowWidth; // 右端のX座標
		scissorRect.top = 0;                      // 上端のY座標
		scissorRect.bottom = WinApp::kWindowHeight; // 下端のY座標

		commandList->RSSetScissorRects(1, &scissorRect); // Scissorをコマンドリストに設定

		//全画面クリア
		commandList->ClearRenderTargetView(rtvHandleCPU,kRenderTargetClearColor,0,nullptr);
		//指定した深度で画面全体をクリアする
		commandList->ClearDepthStencilView(dsvHandleCPU, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		//描画

		// TransitionBarrierを元に戻し、PixelShaerが扱えるようにする
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION; // トランジションバリア
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;      // フラグはなし
		barrier.Transition.pResource = renderTextureResource;  // 対象のリソース
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET; // 前の状態はRTV
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE; // 次の状態はSRV
		commandList->ResourceBarrier(1, &barrier);                                  // バリアをコマンドリストに積む

		dxCommon->PreDraw(); 
		//コマンドを積む
		commandList->SetGraphicsRootSignature(rs.Get());
		commandList->SetPipelineState(pipelineState.Get());
		commandList->IASetVertexBuffers(0, 1, vb.GetView());
		commandList->IASetIndexBuffer(ib.GetDevice());
		//トポロジの設定
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// 使用するディスクリプタヒープの設定　★00_09　追加
		commandList->SetDescriptorHeaps(srvDescriptorHeap->GetDesc().NumDescriptors, &srvDescriptorHeap);

		// SRVのDescriptorTableの先頭を設定　※ 0は rootParameter[0]である　★00_09 追加
		commandList->SetGraphicsRootDescriptorTable(0, srvHandleGPU);

		//頂点数、インデックス数、インデックスの開始位置、インデックスのオフセット
		//commandList->DrawInstanced(3, 1, 0, 0);
		commandList->DrawIndexedInstanced(_countof(indices), 1, 0, 0, 0);

		// 描画終了
		dxCommon->PostDraw();
	}

	//解放
	renderTextureResource->Release(); // RenderTextureResourceの解放
	srvDescriptorHeap->Release();     // SRV用のDescriptorHeapの解放
	rtvDescriptorHeap->Release();     // RTV用のDescriptorHeapの解放

	depthStencilResource->Release(); // DepthStencilTextureResourceの解放
	dsvDescriptorHeap->Release();    // DSV用のDescriptorHeapの解放

	// 解放処理

	KamataEngine::Finalize();
	return 0;
}

//インプットレイアウト、ブレンドステート、ラスタライザステート
//引数として、空のPipelineState,RootSignature,頂点シェーダーvs,ピクセルシェーダーpsを参照で受け取る
void SetupPipelineState(PipelineState& pipelineState, RootSignature& rs, Shader& vs, Shader& ps) {

	//InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	//BlendState-------------------今回は不透明
	D3D12_BLEND_DESC blendDesc{};
	//全ての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	//RasterrizerState----------------------
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	//裏面（反時計回り）をカリングする
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	//塗りつぶしモードをソリッドにする
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	//PSO(PipelineStateObject)の生成---------------------
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rs.Get();            //RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;        //InputLayout
	graphicsPipelineStateDesc.VS = 
	{vs.GetDxcBlob()->GetBufferPointer(), vs.GetDxcBlob()->GetBufferSize()}; //VertexShader
	graphicsPipelineStateDesc.PS = 
	{ps.GetDxcBlob()->GetBufferPointer(), ps.GetDxcBlob()->GetBufferSize()}; //PixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc;               //BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;     //RasterizerState


	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1; // 1つのRTVに書き込む　*二つ同時に書き込むこともできる
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジ（形状）のタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むかの設定（今は気にしなくていい）
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	//準備は整った。PSOを生成する
	pipelineState.Create(graphicsPipelineStateDesc);

}

ID3D12Resource* CreateRenderTargetResource(ID3D12Device* device, 
	int32_t width, uint32_t height, DXGI_FORMAT clearFormat, const FLOAT* clearColor) {
	
	//1.生成するRenderTextureのDescの設定
	D3D12_RESOURCE_DESC resourceDesc{}; 
	resourceDesc.Width = UINT(width);						// RenderTextureの幅
	resourceDesc.Height = UINT(height);						// TetTextureの高さ
	resourceDesc.MipLevels = 1;								//　mipmapの数
	resourceDesc.DepthOrArraySize = 1;						// 奥行 or 配列Textureの配列数
	resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;  // Textureのフォーマット
	resourceDesc.SampleDesc.Count = 1;						//サンプリングカウント　１固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // Textureの時限数。普段使っているのは　2次元
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET; // RenderTargetとして使う通知

	// 2.利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{}; 
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // VRAM上に作る

	// 3. CleraValueの用意
	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = clearFormat; 
	clearValue.Color[0] = clearColor[0]; 
	clearValue.Color[1] = clearColor[1];
	clearValue.Color[2] = clearColor[2];
	clearValue.Color[3] = clearColor[3];

	// 4. RenderTextureResourceの生成
	ID3D12Resource* resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties, // ヒープのプロパティ
		D3D12_HEAP_FLAG_NONE, // ヒープフラグ
		&resourceDesc, // リソースの記述
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, // 初期状態
		&clearValue, // クリア値
		IID_PPV_ARGS(&resource) // 作成したリソースのポインタを受け取る
	);
	assert(SUCCEEDED(hr)); 

	return resource; 
 }

ID3D12Resource* CreateRenderTextureResource(ID3D12Device* device, int32_t width, uint32_t height, DXGI_FORMAT clearFormat, const FLOAT* clearColor) {
	
	// 1. 生成するRenderTextureのDescの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(width); // 幅
	resourceDesc.Height = UINT(height); // 高さ
	resourceDesc.MipLevels = 1;         // mipmapの数
	resourceDesc.DepthOrArraySize = 1;  // 奥行き or 配列数
	resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;// フォーマット
	resourceDesc.SampleDesc.Count = 1;                     // サンプリングカウント
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // 2次元テクスチャ
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET; // RenderTargetとして使用
	
	// 2. 利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties {};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	//3.ClearValueの用意
	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = clearFormat;
	clearValue.Color[0] = clearColor[0]; // 赤成分
	clearValue.Color[1] = clearColor[1];
	clearValue.Color[2] = clearColor[2];
	clearValue.Color[3] = clearColor[3];

	// 4.ReanderTextureResourceの生成
	ID3D12Resource* resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties, // ヒープのプロパティ
		D3D12_HEAP_FLAG_NONE, // ヒープフラグ
		&resourceDesc, // リソースの記述
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, // 初期状態
		&clearValue, // クリア値
		IID_PPV_ARGS(&resource) // 作成したリソースのポインタを受け取る
	);
	assert(SUCCEEDED(hr)); // 成功したか確認
	
	
	return resource;
}

ID3D12Resource* CreateDepthStencilTextureResource(ID3D12Device* device, int32_t width, uint32_t height) {
	
	// 1.生成するDepthStencilTextureのDescの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width; // 幅
	resourceDesc.Height = height; // 高さ
	resourceDesc.MipLevels = 1;   // mipmapの数
	resourceDesc.DepthOrArraySize = 1; // 奥行き or 配列数
	resourceDesc.Format = DXGI_FORMAT_D32_FLOAT; // フォーマット

	resourceDesc.SampleDesc.Count = 1; // サンプリングカウント
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // 2次元テクスチャ
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // DepthStencilとして使用

	// 2.利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // VRAM上に作成

	// 深度値のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f; // 深度値のクリア値
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT; // フォーマット

	// 3. Resourceの生成
	ID3D12Resource* resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties, // ヒープのプロパティ
		D3D12_HEAP_FLAG_NONE, // ヒープフラグ
		&resourceDesc, // リソースの記述
		D3D12_RESOURCE_STATE_DEPTH_WRITE, // 初期状態
		&depthClearValue, // クリア値
		IID_PPV_ARGS(&resource) // 作成したリソースのポインタを受け取る
	);
	assert(SUCCEEDED(hr)); // 成功したか確認

	return resource; }
