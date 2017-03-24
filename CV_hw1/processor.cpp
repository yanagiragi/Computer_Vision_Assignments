#include "processor.h"

void processor::calculateNormals()
{

	// First, Solve light = src * b(x,y) , which b(x,y) stands for Kd * normal(x,y)
	Mat src = foldImgMatrix();
	Mat light = foldLightVector();

	cout << src.rows << " x " << src.cols << endl;

	// pseudo inverse
	Mat srcTrans = src.t();
	
	cout << srcTrans.rows << " x " << srcTrans.cols << ", " << srcTrans.depth()<< endl;
	
	Mat normal = srcTrans * src;

	Mat Bpinv;
	invert(normal, Bpinv, DECOMP_SVD);
	
	cout << Bpinv.rows << " x " << Bpinv.cols << endl;
	
	/*normal = srcTrans * light;

	cout << normal.rows << " x " << normal.cols << endl;*/
	
	/*Mat normal = (srcTrans * src).inv() * srcTrans;

	cout << normal.rows << " x " << normal.cols << endl;*/

}

Mat processor::foldImgMatrix()
{
	Mat src = Mat(m_originalImg.size(), m_originalImg.begin()->second.total(), CV_32F);
	
	for(int i = 0; i < m_originalImg.size(); ++i){
		for(int rowindex = 0; rowindex < m_originalImg[i+1].rows; ++rowindex){
			for(int colindex = 0; colindex < m_originalImg[i+1].cols; ++colindex){
				 src.at<float>(i, rowindex * m_originalImg[i+1].cols + colindex) = static_cast<float>(m_originalImg[i+1].at<uchar>(rowindex, colindex));
			}
		}
	}
    
    return src;
}

Mat processor::foldLightVector()
{
	Mat light = Mat(m_originalLightSrc.size(), 3, CV_32F);
	
	for(int i = 0; i < m_originalLightSrc.size(); ++i){
		light.at<float>(i,0) = static_cast<float>(m_originalLightSrc[i+1].x);
		light.at<float>(i,1) = static_cast<float>(m_originalLightSrc[i+1].y);
		light.at<float>(i,2) = static_cast<float>(m_originalLightSrc[i+1].z);
	}

	return light;
}
