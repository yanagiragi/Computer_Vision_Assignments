#include "processor.h"

Mat processor::getWeye(int rowindex, int colindex)
{
	Mat tmp = Mat::eye(m_originalLightSrc.size(), m_originalLightSrc.size(), CV_64F);
	for(int i = 0; i < m_originalLightSrc.size(); ++i){
		tmp.at<double>(i, i) = static_cast<double>(pow(getWeight(m_originalImg[i+1].at<uchar>(rowindex, colindex)),2));
	}
	return tmp;
}

uchar processor::getWeight(uchar pixel)
{
	return (pixel > 1 && pixel < 255) ? 1 : 0;
}

void processor::calculateNormalsWithWeights()
{

	// split b to normals and albedo
	Mat normal(m_originalImg.begin()->second.rows, m_originalImg.begin()->second.cols, CV_64FC3);
	Mat normalR(m_originalImg.begin()->second.rows, m_originalImg.begin()->second.cols, CV_64FC3);
	Mat normalG(m_originalImg.begin()->second.rows, m_originalImg.begin()->second.cols, CV_64FC3);
	Mat normalB(m_originalImg.begin()->second.rows, m_originalImg.begin()->second.cols, CV_64FC3);
	Mat albedo(m_originalImg.begin()->second.rows, m_originalImg.begin()->second.cols, CV_64F);

	// First, Solve color = light * b(x,y) , which b(x,y) stands for Kd * normal(x,y)
	Mat src = foldImgMatrix();
	Mat light = foldLightVector();

	// pseudo inverse using least square sum with weight	
	for(int i = 0; i < m_originalImg.begin()->second.rows; ++i){
		for(int j = 0; j < m_originalImg.begin()->second.cols; ++j){

			Mat color(m_originalLightSrc.size(), 1, CV_64F);
			for(int lightindex = 0; lightindex < m_originalLightSrc.size(); ++lightindex)
				color.at<double>(lightindex, 1) = src.at<double>(lightindex, i * m_originalImg.begin()->second.cols + j);

			//Mat w = Mat::eye(light.rows, light.rows, CV_64F) * getWeye(i,j); // ** 2
			Mat w = getWeye(i,j); // ** 2
			Mat b = (light.t() * w * light).inv() * light.t() * w * color;
			
			Vec3d tmpBGR(b.at<double>(0, 0), b.at<double>(0, 1), b.at<double>(0, 2));
			double norm = sqrt(
					pow(b.at<double>(0, 0), 2) +
					pow(b.at<double>(0, 1), 2) +
					pow(b.at<double>(0, 2), 2)
				);

			if(norm == 0){
				tmpBGR.val[0] = tmpBGR.val[1] = 0;
				tmpBGR.val[2] = 1;
			}
			else{
				tmpBGR /= norm;
			}

			normal.at<Vec3d>(i, j) = tmpBGR;
			albedo.at<double>(i, j) = norm;

			//cout << b << endl;			
		}	
	}
	
	m_normal = normal;
	m_normalB = normalB;
	m_normalG = normalG;
	m_normalR = normalR;
	m_albedo = albedo;

	return;
	
}

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
			if(i == 0){
				cout << i << " " << j << " " << tmp.val[2] << " 0 0 255" << endl;
			}
			else if(m_albedo.at<double>(i,j) == 0.0){
				cout << i << " " << j << " " << tmp.val[2] << " 255 0 0" << endl;
			} 
			else if(tmp.val[2] != 0){
				cout << i << " " << j << " " << tmp.val[2] << " 255 255 255" << endl;
			}
		}
	}
}

bool processor::isNoise(int rowindex, int colindex, int matindex){
	int size = m_originalImg.begin()->second.rows;
	if(rowindex >= 1 && colindex >= 1 && rowindex < size - 1 && colindex < size - 1){
		// not edge or corner
		return (
				pow(m_originalImg[matindex + 1].at<uchar>(rowindex, colindex) - m_originalImg[matindex + 1].at<uchar>(rowindex, colindex - 1), 2) +
				pow(m_originalImg[matindex + 1].at<uchar>(rowindex, colindex) - m_originalImg[matindex + 1].at<uchar>(rowindex, colindex + 1), 2) +
				pow(m_originalImg[matindex + 1].at<uchar>(rowindex, colindex) - m_originalImg[matindex + 1].at<uchar>(rowindex - 1, colindex), 2) + 
				pow(m_originalImg[matindex + 1].at<uchar>(rowindex, colindex) - m_originalImg[matindex + 1].at<uchar>(rowindex + 1, colindex), 2)
			 ) > noiseThreshold;
	}
	else if(rowindex == 0 || colindex == 0 || rowindex == size - 1|| colindex == size - 1) return true;
	return false;
}

double processor::noiseRecover(int rowindex, int colindex, int matindex){
	int size = m_originalImg.begin()->second.rows;
	
	return 0.0; // sometimes, brutal leads to better result

	// below is the old ways
	/*if(rowindex >= 1 && colindex >= 1 && rowindex < size - 1 && colindex < size - 1){
		// not edge or corner
		return (
				m_originalImg[matindex + 1].at<uchar>(rowindex, colindex - 1) +
				m_originalImg[matindex + 1].at<uchar>(rowindex, colindex + 1) +
				m_originalImg[matindex + 1].at<uchar>(rowindex - 1, colindex) +
				m_originalImg[matindex + 1].at<uchar>(rowindex + 1, colindex) 
			 ) / 4.0;
	}
	else if(colindex == 0){
		if(rowindex == 0){
			return (
				m_originalImg[matindex + 1].at<uchar>(rowindex, colindex + 1) +
				m_originalImg[matindex + 1].at<uchar>(rowindex + 1, colindex) 
			 ) / 2.0;
		}
		else if(rowindex == size - 1){
			return (
				m_originalImg[matindex + 1].at<uchar>(rowindex, colindex + 1) +
				m_originalImg[matindex + 1].at<uchar>(rowindex - 1, colindex) 
			 ) / 2.0;
		}
		else{
			return (
				m_originalImg[matindex + 1].at<uchar>(rowindex, colindex + 1) +
				m_originalImg[matindex + 1].at<uchar>(rowindex - 1, colindex) +
				m_originalImg[matindex + 1].at<uchar>(rowindex + 1, colindex) 
			 ) / 3.0;
		}
	}
	else if(rowindex == 0){
		if(colindex == size - 1){
			return (
				m_originalImg[matindex + 1].at<uchar>(rowindex, colindex - 1) +
				m_originalImg[matindex + 1].at<uchar>(rowindex + 1, colindex) 
			 ) / 2.0;
		}
		else{
			return (
				m_originalImg[matindex + 1].at<uchar>(rowindex, colindex - 1) +
				m_originalImg[matindex + 1].at<uchar>(rowindex, colindex + 1) +
				m_originalImg[matindex + 1].at<uchar>(rowindex + 1, colindex) 
			 ) / 3.0;
		}
	}
	else if(colindex == size - 1){
		if(rowindex == 0){
			return (
				m_originalImg[matindex + 1].at<uchar>(rowindex, colindex - 1) +
				m_originalImg[matindex + 1].at<uchar>(rowindex + 1, colindex) 
			 ) / 2.0;
		}
		else if(rowindex == size - 1){
			return (
				m_originalImg[matindex + 1].at<uchar>(rowindex, colindex - 1) +
				m_originalImg[matindex + 1].at<uchar>(rowindex - 1, colindex) 
			 ) / 2.0;
		}
		else{
			return (
				m_originalImg[matindex + 1].at<uchar>(rowindex, colindex - 1) +
				m_originalImg[matindex + 1].at<uchar>(rowindex - 1, colindex) +
				m_originalImg[matindex + 1].at<uchar>(rowindex + 1, colindex) 
			 ) / 3.0;
		}
	}
	else if(rowindex == size - 1){
		if(colindex == size - 1){
			return (
				m_originalImg[matindex + 1].at<uchar>(rowindex, colindex - 1) +
				m_originalImg[matindex + 1].at<uchar>(rowindex - 1, colindex) 
			 ) / 2.0;
		}
		else{
			return (
				m_originalImg[matindex + 1].at<uchar>(rowindex, colindex - 1) +
				m_originalImg[matindex + 1].at<uchar>(rowindex, colindex + 1) +
				m_originalImg[matindex + 1].at<uchar>(rowindex - 1, colindex) 
			 ) / 3.0;
		}
	}
	else
		return static_cast<double>(m_originalImg[matindex + 1].at<uchar>(rowindex, colindex));*/
}

void processor::imgPreprocessing()
{
	for(int i = 0; i < m_originalImg.size(); ++i){
		for(int rowindex = 0; rowindex < m_originalImg[i+1].rows; ++rowindex){
			for(int colindex = 0; colindex < m_originalImg[i+1].cols; ++colindex){
				//src.at<double>(i, rowindex * m_originalImg[i+1].cols + colindex) = static_cast<double>(m_originalImg[i+1].at<uchar>(rowindex, colindex)) / 255.0;
				if(isNoise(rowindex, colindex, i)){
					//m_originalImg[i+1].at<uchar>(rowindex, colindex) = 0; // force black?
					m_originalImg[i+1].at<uchar>(rowindex, colindex) = static_cast<uchar>(noiseRecover(rowindex, colindex, i)); // force black?
				}
			}
		}
	}
	return ;
}

void processor::constructSurfaceCH() // from center, horiziontal integrate
{
	/*
		x o x 	< o > 	o o o
		x o x   x o x 	< o >
		x o x   x o x 	x o x
	*/
	
	Mat surface(m_originalImg.begin()->second.rows, m_originalImg.begin()->second.cols, CV_64FC3);

	// hold cols, calculate rows for the first time
	for(int i = m_originalImg.begin()->second.rows, j = m_originalImg.begin()->second.cols / 2; i < m_originalImg.begin()->second.rows; ++i){

		double na,nb,nc, partialX, partialY;
		Vec3d tmp = m_normal.at<Vec3d>(i,j);
		na = tmp.val[0]; nb = tmp.val[1]; nc = tmp.val[2];

		partialY = (nb == 0 || nc == 0) ? 0.0 : nb / nc;
		partialY = clamp(partialY, threshold);

		tmp.val[2] = (i == 0) ? partialY : m_normal.at<Vec3d>(i,j - 1).val[2] + partialY;
		
		m_normal.at<Vec3d>(i,0) = tmp;
	}

	for(int i = 0; i < m_originalImg.begin()->second.rows; ++i){
		for(int j = 1, centerC = m_originalImg.begin()->second.cols / 2; j < centerC; ++j){
			
			double na,nb,nc, partialX, partialY;

			if(centerC+ j < m_originalImg.begin()->second.cols){
				Vec3d tmp = m_normal.at<Vec3d>(i, centerC + j);
				Vec3d prev = m_normal.at<Vec3d>(i, centerC + j - 1);
				na = tmp.val[0]; nb = tmp.val[1]; nc = tmp.val[2];

				partialX = (na == 0 || nc == 0) ? 0.0 : na / nc;
				partialX = clamp(partialX, threshold);
				
				tmp.val[2] = prev.val[2] - (partialX);
				
				m_normal.at<Vec3d>(i,centerC + j) = tmp;	
			}

			if(centerC - j >= 0){
				Vec3d tmp = m_normal.at<Vec3d>(i, centerC - j);
				Vec3d prev = m_normal.at<Vec3d>(i, centerC - j + 1);
				na = tmp.val[0]; nb = tmp.val[1]; nc = tmp.val[2];

				partialX = (na == 0 || nc == 0) ? 0.0 : na / nc;
				partialX = clamp(partialX, threshold);
				
				tmp.val[2] = prev.val[2] + (partialX);
				
				m_normal.at<Vec3d>(i,centerC - j) = tmp;	
			}			
		}
	}
	
	return ;
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
