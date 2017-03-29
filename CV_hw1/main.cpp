#include "processor.h"

int main(int argc, char **argv) {

	int chooseX = 0, chooseY = 0, chooseZ = 0;
	std::string pathname = "test/bunny/"; // Remeber to add slash at bottom

	if(argv[1][0] == '-'){
		//return 0;
	}

	if(argc >= 1 && argv[1][0] == '-'){
		std::cout << "\nusage: ./cvhw1 $PATH/ $OPTIONS1 $OPTIONS2 $OPTIONS3\n\nPATH: remember to add slash at last character\n\nOPTIONS:" << std::endl;
		
		std::cout <<
		"OPTIONS1: 0 for not processing noise filter, 1 for processing" << endl <<
		"OPTIONS2: 0 for not calculate normals without weight, 1 for processing" << endl <<
		"OPTIONS3: 3 kinds of integrate methods\n" << endl;
		
		return 0;
	}
	
	if(argc >= 1)
		pathname = std::string(argv[1]);
	
	if(argc == 5){
		chooseX = argv[2][0] - '0' + 0;
		chooseY = argv[3][0] - '0' + 0;
		chooseZ = argv[4][0] - '0' + 0;
	}
	else{
		std::cout << "error input. check --help for help" << std::endl;
		return 0;
	}

	processor p = processor(pathname);

	if(chooseX)
		p.imgPreprocessing();
	
	if(chooseY)
		p.calculateNormalsWithWeights();
	else
		p.calculateNormals();

	//p.previewNormals();

	if(chooseZ % 3 == 0)
		p.constructSurfaceCH();
	else if(chooseZ % 3 == 1)
		p.constructSurfaceH();
	else
		p.constructSurfaceHV();

	p.dumpPly();
	
	return 0;
}
