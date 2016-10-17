#include "FileLoader.h"


bool CFileLoader::ImportModel( const std::string& path, const aiScene* pScene )
{
	if ( path.empty() )
	{
		puts( "Path is invalid.\n" );
		return false;
	}

	Assimp::Importer importer;

	// ���� �ε� �����͸� ���
	importer.FreeScene();

	// �ε�� Mesh ������ ���� �ɼ� : Texture coordinate, Normal, Tangent & Bitangent�� ��������
	unsigned int qualityFlag = aiProcessPreset_TargetRealtime_MaxQuality;   // �ӵ��� �߿��ϸ� ���� �ɼ� ��� : aiProcessPreset_TargetRealtime_Fast �Ǵ� aiProcessPreset_TargetRealtime_Quality
	// DirectX �� Mesh ������ ����
	//qualityFlag |= aiProcess_ConvertToLeftHanded;

	// �� �ε� �� Mesh ������ ����
	pScene = importer.ReadFile( path, qualityFlag );
	if ( pScene == nullptr )
	{
		puts( "File import failure.\n" );
		return false;
	}

	// ������ �� ����
	return true;
}

/*
void MatToRawData( const cv::Mat& image )
{
	// �̹���(�ؽ���) ������ ó�� ���
	// cv::Mat ������ ����
	//  - ������ ���� : Blue, Green, Red, Alpha ����(BGRA)�� �⺻
	//  - 1ä�� (���) : 1�� �ȼ��� unsigned char�� ����Ǿ� ����
	//  - 3ä�� (BGR) : 1�� �ȼ��� 3���� unsigned char (cv::Vec3b)�� ����Ǿ� ����
	//  - 4ä�� (BGRA) : 1�� �ȼ��� 4���� unsigned char (cv::Vec4b)�� ����Ǿ� ����
	// �޸� ����
	//  - cv::Mat�� ���������� ����Ʈ�����ͷ� �ڿ��� �����ǰ� ����
	//  - ���� �����ڴ� Deep Copy�� �ƴ� Shallow Copy�� ����
	//  - Deep Copy�� �ʿ��� ��� clone() �޼��� ���

	// �̹���(�ؽ���) �����Ͱ� ��ȿ���� üũ
	const bool notValid = image.empty();

	// �̹���(�ؽ���)�� ���� ä�� ���� ��������
	const int channel = image.channels();

	// �̹���(�ؽ���)�� ������ Ÿ�� ��������
	const int type = image.type();
	switch ( type )
	{
	case CV_8UC1:
		// 1ä�� (���)
		break;
	case CV_8UC3:
		// 3ä�� (BGR)
		break;
	case CV_8UC4:
		// 4ä�� (BGRA)
		break;
	}

	// �̹���(�ؽ���) ������ Ÿ�� ��ȯ
	cv::Mat convertedImage;
	cv::cvtColor( image, convertedImage, cv::COLOR_BGR2BGRA );  // BGR ���� BGRA �� ��ȯ
	cv::cvtColor( image, convertedImage, cv::COLOR_GRAY2BGRA );  // ��� ���� BGRA �� ��ȯ
	cv::cvtColor( image, convertedImage, cv::COLOR_BGR2RGBA );  // BGR ���� RGBA �� ��ȯ
	cv::cvtColor( image, convertedImage, cv::COLOR_BGRA2GRAY );  // BGRA ���� ��� ���� ��ȯ

	// �̹���(�ؽ���) ������ ���� (Shallow & Deep)
	cv::Mat shallowCopyImage = image;
	cv::Mat deepCopyImage = image.clone();

	// �̹���(�ؽ���) ������ ũ��
	const cv::Size imgSize = image.size();
	const int imgWidth = imgSize.width;
	const int imgHeight = imgSize.height;

	// �̹���(�ؽ���) ��Ʈ�� ������ ������ ��������
	const void* pImage = image.ptr< void >( 0 );                        // ���� �������� ���
	const unsigned char* pUCImage = image.ptr< unsigned char >( 0 );    // 1ä���� ���
	const cv::Vec3b* pUC3Image = image.ptr< cv::Vec3b >( 0 );           // 3ä���� ���
	const cv::Vec4b* pUC4Image = image.ptr< cv::Vec4b >( 0 );           // 4ä���� ���

	// �̹���(�ؽ���) ��Ʈ�� Ư�� ��ǥ�� �� �������� - y��(����) ��ġ�� �տ� �ִ°Ϳ� ����!!
	const int y = 0; const int x = 0;
	const unsigned char val = image.at< unsigned char >( y, x );        // 1ä���� ���
	const cv::Vec3b val3 = image.at< cv::Vec3b >( y, x );               // 3ä���� ���
	const cv::Vec4b val4 = image.at< cv::Vec4b >( y, x );               // 4ä���� ���

	// �̹���(�ؽ���)�� ������(â)�� ���� ���
	cv::imshow( "â�̸�", image );

	// �̹���(�ؽ���)�� ������(â)�� ���� ��� ���ߴ� ��� (Ű �̺�Ʈ �߻��� ASCII�� ��ȯ)
	const int delay_ms = 1;
	const int key = cv::waitKey( delay_ms );
}
*/