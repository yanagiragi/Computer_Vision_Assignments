#include "processor.h"

int main(int argc, char **argv) {

	std::string pathname = "test/bunny/"; // Remeber to add slash at bottom

	processor p = processor(pathname);

	std::cout << "Finished Reading LightSource.txt and Src Pictures in directory: " << pathname << std::endl;
	
	p.calculateNormals();

	std::cout << "Finished Calculate Normals" << std::endl;
	
	return 0;
}
