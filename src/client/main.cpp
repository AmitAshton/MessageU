#include "MessageUClient.h"
#include <iostream>

int main()
{
	try
	{
		MessageUClient client;
		client.run();    
	}
	catch (const std::exception& e)
	{
		std::cerr << "Fatal Error: " << e.what() << std::endl;
		std::cerr << "Press Enter to exit." << std::endl;
		std::cin.get();
		return 1;
	}
	catch (...)
	{
		std::cerr << "An unknown fatal error occurred." << std::endl;
		std::cerr << "Press Enter to exit." << std::endl;
		std::cin.get();
		return 2;
	}

	return 0;
}