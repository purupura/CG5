
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

void SetupPipelineState(PipelineState& pipelineState,RootSignature& rs,Shader& vs,Shader& ps);

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
	};

	//頂点データの準備　★00_07　追加

VertexData vertices[] = {
	    {{-1.0f, 1.0f, 0.0f, 1.0f}},  // 左上
	    {{1.0f, 1.0f, 0.0f, 1.0f}},   // 右上
	    {{-1.0f, -1.0f, 0.0f, 1.0f}}, // 左下
	    {{1.0f, -1.0f, 0.0f, 1.0f}},  // 右下
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

	for (int i = 0; i < _countof(indices); ++i) {
		pGpuIndices[i] = indices[i];
	}

	////　頂点リソースにデータを書き込む-------------------
	//Vector4* vertexData = nullptr;
	//vb.Get()->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	//vertexData[0] = {-0.5f, -0.5f, 0.0f, 1.0f};//左下
	//vertexData[1] = { 0.0f,  0.5f, 0.0f, 1.0f};//上
	//vertexData[2] =  {0.5f, -0.5f, 0.0f, 1.0f};//右下


	// メインループ
	while (true) {
		// エンジンの更新
		if (KamataEngine::Update()) {
			break;
		}


		// 描画開始
		dxCommon->PreDraw();


		//コマンドを積む
		commandList->SetGraphicsRootSignature(rs.Get());
		commandList->SetPipelineState(pipelineState.Get());
		commandList->IASetVertexBuffers(0, 1, vb.GetView());
		commandList->IASetIndexBuffer(ib.GetDevice());
		//トポロジの設定
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//頂点数、インデックス数、インデックスの開始位置、インデックスのオフセット
		//commandList->DrawInstanced(3, 1, 0, 0);
		commandList->DrawIndexedInstanced(_countof(indices), 1, 0, 0, 0);

		// 描画終了
		dxCommon->PostDraw();
	}

	// 解放処理

	KamataEngine::Finalize();
	return 0;
}

//インプットレイアウト、ブレンドステート、ラスタライザステート
//引数として、空のPipelineState,RootSignature,頂点シェーダーvs,ピクセルシェーダーpsを参照で受け取る
void SetupPipelineState(PipelineState& pipelineState, RootSignature& rs, Shader& vs, Shader& ps) {

	//InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[1] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

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
