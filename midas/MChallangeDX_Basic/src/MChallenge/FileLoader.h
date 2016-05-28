#pragma once

#include <iostream>
#include <string>
#include <cassert>

// obj 모델 로드 라이브러리
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
// 이미지(텍스쳐) 로드 라이브러리
#include <opencv2/opencv.hpp>

class CFileLoader
{
public:
	CFileLoader()
	{

	}
	~CFileLoader()
	{

	}
	bool ImportModel( const std::string& path, const aiScene* pScene );
	//void MatToRawData( const cv::Mat& image );
};


