#include "WorldTransformEx.h"

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;

// Scale,Rotation,Translate 行列から　world行列を計算
//そして定数バッファへの転送も行う
void WorldTransformEx::UpdateMatrix() {
	// world変換行列を計算し、matWorld_に格納する
	matWorld_ = MakeAffineMatrix();
	// 定数バッファに転送する
	TransferMatrix();
}

KamataEngine::Matrix4x4 WorldTransformEx::MakeAffineMatrix() { 
	// Scale Matrix
	Matrix4x4 matScale = MakeScaleMatrix(scale_);

	// Rotation Matrix
	Matrix4x4 matRotX = MakeRotateXMatrix(rotation_.x);
	Matrix4x4 matRotY = MakeRotateYMatrix(rotation_.y);
	Matrix4x4 matRotZ = MakeRotateZMatrix(rotation_.z);
	Matrix4x4 matRot = matRotZ * matRotX * matRotY;

	// Translate Matrix
	Matrix4x4 matTrans = MakeTranslateMatrix(translation_);

	// World Matrix
	Matrix4x4 matWorld = matScale * matRot * matTrans;
	
	return matWorld_; }
