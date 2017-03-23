#include "processor.h"

void processor::calculateNormals()
{
	/*Mat src = Mat(m_originalImg.size(), m_originalImg.begin()->second.total(), CV_8U);
	cout << static_cast<int>(m_originalImg.size()) << endl;
	
	for(int i = 0; i < m_originalImg.size(); ++i){
		for(int rowindex = 0; rowindex < m_originalImg[i+1].rows; ++rowindex){
			for(int colindex = 0; colindex < m_originalImg[i+1].cols; ++colindex){
				 src.at<uchar>(i, rowindex * m_originalImg[i+1].cols + colindex) = m_originalImg[i+1].at<uchar>(rowindex, colindex);
			}
		}
	}*/
	
	Mat src = foldImgMatrix();
	Mat light = foldLightVector();


	cout << light << endl;
	

}

Mat processor::foldImgMatrix()
{

	Mat src = Mat(m_originalImg.size(), m_originalImg.begin()->second.total(), CV_8U);
	
	for(int i = 0; i < m_originalImg.size(); ++i){
		for(int rowindex = 0; rowindex < m_originalImg[i+1].rows; ++rowindex){
			for(int colindex = 0; colindex < m_originalImg[i+1].cols; ++colindex){
				 src.at<uchar>(i, rowindex * m_originalImg[i+1].cols + colindex) = m_originalImg[i+1].at<uchar>(rowindex, colindex);
			}
		}
	}
	cout << static_cast<int>(m_originalImg.size()) << endl;
	/*
	cout << src.rows << "," <<  src.cols << endl;
	for(int rowindex = 0; rowindex < src.rows; ++rowindex){
		for(int colindex = 0; colindex < src.cols; ++colindex){
			cout << static_cast<int>(src.at<uchar>(rowindex, colindex)) << " ";
		}
	}
	*/

    return src;
}

Mat processor::foldLightVector()
{
	Mat light = Mat(m_originalLightSrc.size(), 3, CV_64F);
	for(int i = 0; i < m_originalLightSrc.size(); ++i){
		cout << static_cast<double>(m_originalLightSrc[i+1].x) << ", " 
		<< static_cast<double>(m_originalLightSrc[i+1].y) << ", "
		<< static_cast<double>(m_originalLightSrc[i+1].z) << endl;

		light.at<double>(i,0) = static_cast<double>(m_originalLightSrc[i+1].x);
		light.at<double>(i,1) = static_cast<double>(m_originalLightSrc[i+1].y);
		light.at<double>(i,2) = static_cast<double>(m_originalLightSrc[i+1].z);
	}

	return light;
}
