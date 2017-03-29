#include "processor.h"

int main(int argc, char **argv) {

	std::string pathname = "test/star/"; // Remeber to add slash at bottom

	if(argc != 1)
		pathname = std::string(argv[1]);

	processor p = processor(pathname);

	p.imgPreprocessing();

	//std::cout << "Finished Reading LightSource.txt and Src Pictures in directory: " << pathname << std::endl;
	
	p.calculateNormals();

	//std::cout << "Finished Calculate Normals" << std::endl;

	p.previewNormals();

	//p.constructSurfaceCH();
	p.constructSurfaceHV();

	p.dumpPly();
	
	return 0;
}
