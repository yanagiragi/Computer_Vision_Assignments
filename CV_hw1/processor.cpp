#include "processor.h"

void processor::calculateNormals()
{
	// First, Solve color = light * b(x,y) , which b(x,y) stands for Kd * normal(x,y)
	Mat src = foldImgMatrix();
	Mat light = foldLightVector();

	// pseudo inverse
	Mat normal = (light.t() * light).inv() * light.t() * src;

	m_normal = normal;

	return;
}

Mat processor::foldImgMatrix()
{
	Mat src = Mat(m_originalImg.size(), m_originalImg.begin()->second.total(), CV_64F);
	
	for(int i = 0; i < m_originalImg.size(); ++i){
		for(int rowindex = 0; rowindex < m_originalImg[i+1].rows; ++rowindex){
			for(int colindex = 0; colindex < m_originalImg[i+1].cols; ++colindex){
				 src.at<double>(i, rowindex * m_originalImg[i+1].cols + colindex) = static_cast<double>(m_originalImg[i+1].at<uchar>(rowindex, colindex)) / 255.0;
			}
		}
	}
    
    return src;
}

Mat processor::foldLightVector()
{
	Mat light = Mat(m_originalLightSrc.size(), 3, CV_64F);
	
	for(int i = 0; i < m_originalLightSrc.size(); ++i){
		
		// Normalize to 0.0 ~ 1.0 (By dividing 255.0)
		Point3_<double> tmp = Point3_<double>(static_cast<double>(m_originalLightSrc[i+1].x), static_cast<double>(m_originalLightSrc[i+1].y), static_cast<double>(m_originalLightSrc[i+1].z));
		double sum = sqrt(tmp.x * tmp.x + tmp.y * tmp.y + tmp.z * tmp.z);
		
		light.at<double>(i,0) = static_cast<double>(m_originalLightSrc[i+1].x) / sum;
		light.at<double>(i,1) = static_cast<double>(m_originalLightSrc[i+1].y) / sum;
		light.at<double>(i,2) = static_cast<double>(m_originalLightSrc[i+1].z) / sum;
	}

	return light;
}
