#include "processor.h"

void processor::calculateNormals()
{
	// First, Solve color = light * b(x,y) , which b(x,y) stands for Kd * normal(x,y)
	Mat src = foldImgMatrix();
	Mat light = foldLightVector();

	// pseudo inverse
	Mat b = (light.t() * light).inv() * light.t() * src;

	// split b to normals and albedo
	Mat normal(m_originalImg.begin()->second.rows, m_originalImg.begin()->second.cols, CV_64FC3);
	Mat normalR(m_originalImg.begin()->second.rows, m_originalImg.begin()->second.cols, CV_64FC3);
	Mat normalG(m_originalImg.begin()->second.rows, m_originalImg.begin()->second.cols, CV_64FC3);
	Mat normalB(m_originalImg.begin()->second.rows, m_originalImg.begin()->second.cols, CV_64FC3);
	Mat albedo(m_originalImg.begin()->second.rows, m_originalImg.begin()->second.cols, CV_64F);

	for(int j = 0; j < b.cols; ++j){
		Vec3d tmpBGR (b.at<double>(0, j), b.at<double>(1,j), b.at<double>(2,j));
		
		double sum = sqrt ( tmpBGR.val[0] * tmpBGR.val[0] + tmpBGR.val[1] * tmpBGR.val[1] + tmpBGR.val[2] * tmpBGR.val[2] );

		if(sum != 0){ // or it will becone nan
			tmpBGR.val[0] /= sum;
			tmpBGR.val[1] /= sum;
			tmpBGR.val[2] /= sum;
		}

		normal.at<Vec3d>(j / m_originalImg.begin()->second.cols, j % m_originalImg.begin()->second.cols) = tmpBGR;
		normalB.at<Vec3d>(j / m_originalImg.begin()->second.cols, j % m_originalImg.begin()->second.cols) = Vec3d(tmpBGR.val[0], 0, 0);
		normalG.at<Vec3d>(j / m_originalImg.begin()->second.cols, j % m_originalImg.begin()->second.cols) = Vec3d(0, tmpBGR.val[1], 0);
		normalR.at<Vec3d>(j / m_originalImg.begin()->second.cols, j % m_originalImg.begin()->second.cols) = Vec3d(0, 0, tmpBGR.val[2]);

		// There is no need to multiply 255 
		// It is because we use double, it will automaticlly times 255.0 when imshow
		// ref: http://docs.opencv.org/3.0-beta/modules/highgui/doc/user_interface.html#imshow


		albedo.at<double>(j / m_originalImg.begin()->second.cols, j % m_originalImg.begin()->second.cols) = sum * 255.0;

	}

	m_normal = normal;
	m_normalB = normalB;
	m_normalG = normalG;
	m_normalR = normalR;
	m_albedo = albedo;

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
		
		Point3_<double> tmp = Point3_<double>(static_cast<double>(m_originalLightSrc[i+1].x), static_cast<double>(m_originalLightSrc[i+1].y), static_cast<double>(m_originalLightSrc[i+1].z));
		
		// Normalize to 0.0 ~ 1.0 (By dividing 255.0)
		double sum = sqrt(tmp.x * tmp.x + tmp.y * tmp.y + tmp.z * tmp.z);
		
		if(sum != 0){
			light.at<double>(i,0) = static_cast<double>(m_originalLightSrc[i+1].x) / sum;
			light.at<double>(i,1) = static_cast<double>(m_originalLightSrc[i+1].y) / sum;
			light.at<double>(i,2) = static_cast<double>(m_originalLightSrc[i+1].z) / sum;
		}
		else{
			light.at<double>(i,0) = static_cast<double>(m_originalLightSrc[i+1].x);
			light.at<double>(i,1) = static_cast<double>(m_originalLightSrc[i+1].y);
			light.at<double>(i,2) = static_cast<double>(m_originalLightSrc[i+1].z);
		}
	}

	return light;
}

void processor::previewNormals()
{
	// cout << m_normalR << endl;
	imshow("albedo", m_albedo);

	imshow("normal", m_normal);
	imshow("normalB", m_normalB);
	imshow("normalG", m_normalG);
	imshow("normalR", m_normalR);

	waitKey(0);
	
	return ;
}
