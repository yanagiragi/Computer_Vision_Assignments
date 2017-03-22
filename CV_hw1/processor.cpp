#include "processor.h"

void processor::calculateNormals()
{
	Mat src = Mat(m_originalImg.size(), m_originalImg.begin()->second.total(), CV_8U);
	cout << static_cast<int>(m_originalImg.size()) << endl;
	
	for(int i = 0; i < m_originalImg.size(); ++i){
		for(int rowindex = 0; rowindex < m_originalImg[i+1].rows; ++rowindex){
			for(int colindex = 0; colindex < m_originalImg[i+1].cols; ++colindex){
				 src.at<uchar>(i, rowindex * m_originalImg[i+1].cols + colindex) = m_originalImg[i+1].at<uchar>(rowindex, colindex);
			}
		}
	}
	/*
	cout << src.rows << "," <<  src.cols << endl;
	for(int rowindex = 0; rowindex < src.rows; ++rowindex){
		for(int colindex = 0; colindex < src.cols; ++colindex){
			cout << static_cast<int>(src.at<uchar>(rowindex, colindex)) << " ";
		}
	}
	*/

}

void processor::foldMatrix()
{

}
