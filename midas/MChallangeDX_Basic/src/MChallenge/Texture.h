#include <d3d11.h>

class CTexture
{
public:
	CTexture();
	~CTexture();

	bool Initialize(ID3D11Device*, WCHAR*);
	void Release();
	ID3D11ShaderResourceView* GetTexture();

	ID3D11ShaderResourceView* GetShaderResourceView();
	bool BindTexture( ID3D11DeviceContext*, int nSlotNum );
private:
	ID3D11ShaderResourceView* m_pShaderResourceView;
};

