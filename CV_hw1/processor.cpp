#include "processor.h"

void processor::calculateNormals()
{
    Mat allImage(
            m_originalImg.size(), 
            m_originalImg.begin()->second.rows * m_originalImg.begin()->second.cols,
            CV_8UC3
            );

    cout << allImage.rows << endl;

    for(auto i = m_originalImg.begin(); i != m_originalImg.end(); ++i){
        for(int rowindex = 0; rowindex < i->second.rows; ++rowindex){
            for(int colindex = 0; colindex < i->second.cols; ++colindex){
                allImage.at<uchar>(i->first, rowindex * i->second.cols + colindex) = (uchar)(255.0);
                cout << "(" << i->first << " , " <<  rowindex << " | "<<  colindex << ")" << endl;
            }
        }
    }
    
    /*for(auto i = m_originalImg.begin(); i != m_originalImg.end(); ++i){
        for(int rowindex = 0; rowindex < i->second.rows; ++rowindex){
            for(int colindex = 0; colindex < i->second.cols; ++colindex){
                //allImage.at<uchar>(i->first,rowindex * i->second.cols + colindex) = i->second.at<uchar>(rowindex, colindex);
                allImage.at<uchar>(i->first,rowindex * i->second.cols + colindex) = (uchar)(255.0);
            }
        }
    }*/

    //cout << m_originalImg.begin()->second.rows << " * " << m_originalImg.begin()->second.cols << endl;

    //cout << allImage.cols * allImage.rows << endl;

    //imshow("res", allImage);
    //waitKey(100);
    
    cout << "hi" << endl;
}

