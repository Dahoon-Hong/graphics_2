#pragma once

#include <iostream>
#include <string>
#include <cassert>

// obj �� �ε� ���̺귯��
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
// �̹���(�ؽ���) �ε� ���̺귯��
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


