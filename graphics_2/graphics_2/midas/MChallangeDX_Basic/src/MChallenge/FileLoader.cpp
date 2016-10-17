#include "FileLoader.h"


bool CFileLoader::ImportModel( const std::string& path, const aiScene* pScene )
{
	if ( path.empty() )
	{
		puts( "Path is invalid.\n" );
		return false;
	}

	Assimp::Importer importer;

	// 기존 로드 데이터를 비움
	importer.FreeScene();

	// 로드시 Mesh 데이터 생성 옵션 : Texture coordinate, Normal, Tangent & Bitangent를 생성해줌
	unsigned int qualityFlag = aiProcessPreset_TargetRealtime_MaxQuality;   // 속도가 중요하면 다음 옵션 사용 : aiProcessPreset_TargetRealtime_Fast 또는 aiProcessPreset_TargetRealtime_Quality
	// DirectX 용 Mesh 데이터 생성
	//qualityFlag |= aiProcess_ConvertToLeftHanded;

	// 모델 로드 및 Mesh 데이터 생성
	pScene = importer.ReadFile( path, qualityFlag );
	if ( pScene == nullptr )
	{
		puts( "File import failure.\n" );
		return false;
	}

	// 렌더링 모델 생성
	return true;
}

/*
void MatToRawData( const cv::Mat& image )
{
	// 이미지(텍스쳐) 데이터 처리 방법
	// cv::Mat 데이터 형태
	//  - 데이터 순서 : Blue, Green, Red, Alpha 순서(BGRA)가 기본
	//  - 1채널 (흑백) : 1개 픽셀당 unsigned char로 저장되어 있음
	//  - 3채널 (BGR) : 1개 픽셀당 3개의 unsigned char (cv::Vec3b)로 저장되어 있음
	//  - 4채널 (BGRA) : 1개 픽셀당 4개의 unsigned char (cv::Vec4b)로 저장되어 있음
	// 메모리 관리
	//  - cv::Mat은 내부적으로 스마트포인터로 자원이 관리되고 있음
	//  - 대입 연산자는 Deep Copy가 아닌 Shallow Copy로 동작
	//  - Deep Copy가 필요한 경우 clone() 메서드 사용

	// 이미지(텍스쳐) 데이터가 유효한지 체크
	const bool notValid = image.empty();

	// 이미지(텍스쳐)의 색상 채널 개수 가져오기
	const int channel = image.channels();

	// 이미지(텍스쳐)의 데이터 타입 가져오기
	const int type = image.type();
	switch ( type )
	{
	case CV_8UC1:
		// 1채널 (흑백)
		break;
	case CV_8UC3:
		// 3채널 (BGR)
		break;
	case CV_8UC4:
		// 4채널 (BGRA)
		break;
	}

	// 이미지(텍스쳐) 데이터 타입 변환
	cv::Mat convertedImage;
	cv::cvtColor( image, convertedImage, cv::COLOR_BGR2BGRA );  // BGR 에서 BGRA 로 변환
	cv::cvtColor( image, convertedImage, cv::COLOR_GRAY2BGRA );  // 흑백 에서 BGRA 로 변환
	cv::cvtColor( image, convertedImage, cv::COLOR_BGR2RGBA );  // BGR 에서 RGBA 로 변환
	cv::cvtColor( image, convertedImage, cv::COLOR_BGRA2GRAY );  // BGRA 에서 흑백 으로 변환

	// 이미지(텍스쳐) 데이터 복사 (Shallow & Deep)
	cv::Mat shallowCopyImage = image;
	cv::Mat deepCopyImage = image.clone();

	// 이미지(텍스쳐) 데이터 크기
	const cv::Size imgSize = image.size();
	const int imgWidth = imgSize.width;
	const int imgHeight = imgSize.height;

	// 이미지(텍스쳐) 비트맵 데이터 포인터 가져오기
	const void* pImage = image.ptr< void >( 0 );                        // 순수 포인터의 경우
	const unsigned char* pUCImage = image.ptr< unsigned char >( 0 );    // 1채널인 경우
	const cv::Vec3b* pUC3Image = image.ptr< cv::Vec3b >( 0 );           // 3채널인 경우
	const cv::Vec4b* pUC4Image = image.ptr< cv::Vec4b >( 0 );           // 4채널인 경우

	// 이미지(텍스쳐) 비트맵 특정 좌표의 값 가져오기 - y축(높이) 위치를 앞에 넣는것에 주의!!
	const int y = 0; const int x = 0;
	const unsigned char val = image.at< unsigned char >( y, x );        // 1채널인 경우
	const cv::Vec3b val3 = image.at< cv::Vec3b >( y, x );               // 3채널인 경우
	const cv::Vec4b val4 = image.at< cv::Vec4b >( y, x );               // 4채널인 경우

	// 이미지(텍스쳐)를 윈도우(창)에 띄우는 방법
	cv::imshow( "창이름", image );

	// 이미지(텍스쳐)를 윈도우(창)에 띄우고 잠시 멈추는 방법 (키 이벤트 발생시 ASCII값 반환)
	const int delay_ms = 1;
	const int key = cv::waitKey( delay_ms );
}
*/