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
		// cout << sum << endl;
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

		albedo.at<double>(j / m_originalImg.begin()->second.cols, j % m_originalImg.begin()->second.cols) = sum;

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

void processor::dumpPly()
{
	string prefix = "ply\nformat ascii 1.0\ncomment alpha=1.0\nelement vertex " + to_string(m_normal.rows * m_normal.cols) + "\nproperty float x\nproperty float y\nproperty float z\nproperty uchar red\nproperty uchar green\nproperty uchar blue z\nend_header";

	cout << prefix << endl;

	for(int i = 0; i < m_originalImg.begin()->second.rows; ++i){
		for(int j = 0; j < m_originalImg.begin()->second.cols; ++j){
			Vec3d tmp = m_normal.at<Vec3d>(i, j);
			if(i == 0)
				cout << i << " " << j << " " << tmp.val[2] << " 0 0 255" << endl;
			else if(m_originalImg.begin()->second.at<uchar>(i,j) == 0) // may lead to an error, cause it should be sampled in six pictures
				cout << i << " " << j << " " << tmp.val[2] << " 255 0 0" << endl;
			else if(tmp.val[2] != 0)
				cout << i << " " << j << " " << tmp.val[2] << " 255 255 255" << endl;
		}
	}
}

void processor::constructSurfaceH()
{
	/*
		o o o 	o o o 	o o o
		x x x   o > > 	o o o
		x x x   o x x 	o > >
	*/
	double x = 0, y = 0;

	Mat surface(m_originalImg.begin()->second.rows, m_originalImg.begin()->second.cols, CV_64FC3);

	for(int i = 0; i < m_originalImg.begin()->second.rows; ++i){

		double na,nb,nc, partialX, partialY;
		Vec3d tmp = m_normal.at<Vec3d>(i,0);

		na = tmp.val[0]; nb = tmp.val[1]; nc = tmp.val[2];

		partialY = (nb == 0 || nc == 0) ? 0.0 : nb / nc;
		partialY = clamp(partialY, threshold);

		tmp.val[2] = partialY;		
		
		m_normal.at<Vec3d>(i,0) = tmp;
	}

	for(int i = 0; i < m_originalImg.begin()->second.rows; ++i){
		x = 0;
		for(int j = 1; j < m_originalImg.begin()->second.cols; ++j){

			double na,nb,nc, partialX, partialY;
			Vec3d tmp = m_normal.at<Vec3d>(i,j);
			Vec3d prev = m_normal.at<Vec3d>(i, j - 1);

			na = tmp.val[0]; nb = tmp.val[1]; nc = tmp.val[2];

			partialX = (na == 0 || nc == 0) ? 0.0 : na / nc;
			partialX = clamp(partialX, threshold);
			x += partialX;

			if(tmp.val[2] == 0){
				tmp.val[2] = 0;
			}
			else if(x <= 0){
				// avoid digging below the plane
				tmp.val[2] = prev.val[2] + (partialX);
			}
			else{
				tmp.val[2] = prev.val[2] - (partialX);
			}
			
			m_normal.at<Vec3d>(i,j) = tmp;
		}
	}
	
	return ;
}

void processor::constructSurfaceHV()
{
	/*
		o > >   o o o	o o o
		x x x   v v v   o o o
		x x x   x x x   v v v
	*/

	Mat surface(m_originalImg.begin()->second.rows, m_originalImg.begin()->second.cols, CV_64FC3);

	for(int i = 0; i < m_originalImg.begin()->second.rows; ++i){
		for(int j = 0; j < m_originalImg.begin()->second.cols; ++j){

			double na,nb,nc, partialX, partialY;
			Vec3d tmp = m_normal.at<Vec3d>(i,j);

			na = tmp.val[0]; nb = tmp.val[1]; nc = tmp.val[2];

			partialX = (na == 0 || nc == 0) ? 0.0 : na / nc;
			partialY = (nb == 0 || nc == 0) ? 0.0 : nb / nc;

			// clamp for special peeks
			partialX = clamp(partialX, threshold);
			partialY = clamp(partialY, threshold);

			if(i == 0){
				if(j != 0){
					Vec3d prev = m_normal.at<Vec3d>(i,j-1);
					tmp.val[2] = prev.val[2] - (partialX);
				}
				else{
					tmp.val[2] = partialX;
				}

			}
			else{
				Vec3d prev = m_normal.at<Vec3d>(i-1,j);

				if(j != 0){
					tmp.val[2] = prev.val[2] - (partialY);
				}
				else{
					tmp.val[2] = partialY;
				}
			}
			m_normal.at<Vec3d>(i,j) = tmp;
		}
	}

	return ;
}

void processor::previewNormals()
{
	imshow("albedo", m_albedo);

	Mat normalMap(m_normal.rows , m_normal.cols, CV_8UC3);

	for(int i = 0; i < m_normal.rows; ++i){
		for(int j = 0; j < m_normal.cols; ++j){
			Vec3d tmp = m_normal.at<Vec3d>(i,j);

			tmp.val[0] = tmp.val[0]; // B
			tmp.val[1] = tmp.val[1]; // G
			tmp.val[2] = tmp.val[2]; // R

			normalMap.at<Vec3b>(i,j) = Vec3b(tmp.val[2] * 127 + 128, tmp.val[1] * 127 + 128, tmp.val[0] * 127 + 128);
		}
	}

	//imshow("normal", m_normal);
	imshow("nmap", normalMap);
	imshow("normalB", m_normalB);
	imshow("normalG", m_normalG);
	imshow("normalR", m_normalR);

	waitKey(0);

	return ;
}

double processor::clamp(double src, double thres){
	return clamp(src, -1.0 * thres, 1.0 * thres);
}
double processor::clamp(double src, double min, double max){
	return (src <= min) ? min : ((src >= max) ? max : src);
}
