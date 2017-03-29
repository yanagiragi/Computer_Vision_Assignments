#ifndef _PROCESSOR_H
#define _PROCESSOR_H

#define FGETS_MAX_BUFFER 1024

#include <vector>
#include <map>
#include <iostream>
#include <cstring>
#include <cmath>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

#include <opencv2/highgui/highgui.hpp> 
//#include <opencv2/core/types.hpp>
#include <opencv2/core/operations.hpp>
#include <opencv2/core/core.hpp>

using namespace cv;
using namespace std;

class processor {

    private :
        const double threshold = 0.5;
        map<int, Mat> m_originalImg = map<int, Mat>();
        map<int, Point3_<int>> m_originalLightSrc = map<int, Point3_<int>>();
        Mat m_normal, m_albedo, m_normalB, m_normalG, m_normalR;

    public :
        processor(string srcPath)
        {
            DIR *d;
            struct dirent *DirEntry;
            FILE *fp; 
            char *str; 
            int i,x,y,z;
            
            str = (char *)malloc(sizeof(char) * FGETS_MAX_BUFFER);
            d = opendir(srcPath.c_str());

            while((DirEntry=readdir(d)))
            {
                // Picture
                if(strstr(DirEntry->d_name, "bmp")){
                    char* tmp = DirEntry->d_name;
                    while(!isdigit(*tmp)) ++tmp;
                    sscanf(tmp, "%d.bmp", &i);
                    m_originalImg.insert( pair<int, Mat>(i, imread( srcPath + string(DirEntry->d_name), CV_LOAD_IMAGE_GRAYSCALE )));
                }
                // Reading LightSource.txt
                else if(strstr(DirEntry->d_name, "LightSource.txt")){
                    fp = fopen((srcPath + string(DirEntry->d_name)).c_str(), "r");
                    while(fgets(str, FGETS_MAX_BUFFER, fp)){
                        if(*str == '\n') continue;
                        sscanf(str, "pic%d: (%d,%d,%d)\n", &i, &x, &y, &z);
                        m_originalLightSrc.insert( pair<int,Point3_<int>>(i, Point3_<int>(x,y,z)));
                    }
                    fclose(fp);
                }
            }
            free(str);

            // Tranverse
            /*for(auto i = m_originalImg.begin(); i != m_originalImg.end(); ++i){                
                if(i->second.empty()) continue;
                cout << "id = " << i->first << endl;
                cout << m_originalLightSrc.find(i->first)->second << endl;
                
                imshow("test" + to_string(i->first),i->second);
                waitKey(250);
            }

            cout << "Done" << endl;*/

        }

        ~processor() {}


        // Functions
        void calculateNormals();
    	Mat foldImgMatrix();
    	Mat foldLightVector();
    	void previewNormals();
    	void dumpPly();
    	void constructSurfaceCH();
    	void constructSurfaceH();
    	void constructSurfaceHV();

    	double clamp(double, double);
    	double clamp(double, double, double);
};

#endif
