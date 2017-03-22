#include "processor.h"

int main(int argc, char **argv) {

	std::string pathname = "test/bunny/"; // Remeber to add slash at bottom

	processor p = processor(pathname);

	p.calculateNormals();

	std::cout << "Hello" << std::endl;
	
	return 0;
}
