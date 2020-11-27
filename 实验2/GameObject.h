//***************************************************************************************
// GameObject.h by X_Jun(MKXJun) (C) 2018-2019 All Rights Reserved.
// Licensed under the MIT License.
//
// ������Ϸ����
// Simple game object.
//***************************************************************************************

#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "Model.h"




class GameObject
{
public:
	template <class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;


	GameObject();

	// ��ȡλ��
	DirectX::XMFLOAT3 GetPosition() const;

	//
	// ��ȡ��Χ��
	//

	DirectX::BoundingBox GetLocalBoundingBox() const;
	DirectX::BoundingBox GetBoundingBox() const;
	DirectX::BoundingOrientedBox GetBoundingOrientedBox() const;

	//
	// ����ʵ��������
	//

	// ��ȡ������������ʵ������Ŀ
	size_t GetCapacity() const;
	// ��������ʵ��������������ʵ������Ŀ
	void ResizeBuffer(ComPtr<ID3D11Device> device, size_t count);
	// ��ȡʵ��������

	//
	// ����ģ��
	//

	void SetModel(Model&& model);
	void SetModel(const Model& model);

	//
	// ���þ���
	//

	void SetWorldMatrix(const DirectX::XMFLOAT4X4& world);
	void XM_CALLCONV SetWorldMatrix(DirectX::FXMMATRIX world);

	//
	// ����
	//
	
	// ���ƶ���
	void Draw(ComPtr<ID3D11DeviceContext> deviceContext, BasicEffect& effect);
	void Draw(ComPtr<ID3D11DeviceContext> deviceContext, ParticleEffect& effect);

	// ����ʵ��
	void DrawInstanced(ComPtr<ID3D11DeviceContext> deviceContext, BasicEffect & effect, const std::vector<DirectX::XMMATRIX>& data);

private:
	Model m_Model;												    // ģ��
	DirectX::XMFLOAT4X4 m_WorldMatrix;							    // �������

	ComPtr<ID3D11Buffer> m_pInstancedBuffer;						// ʵ��������
	size_t m_Capacity;											    // ����������
};




#endif
