#include "MessageUClient.h"
#include <iostream>

/**
 * @brief Main entry point for the client application.
 *
 * This function creates and runs the main client instance.
 * All application logic is handled within the MessageUClient class.
 */
int main()
{
	try
	{
		MessageUClient client; // The client object is created on the stack
		client.run();          // Run the main application loop
	}
	catch (const std::exception& e)
	{
		// Catch any critical initialization errors
		// --- THIS IS THE FIX ---
		// We now exit immediately if initialization fails.
		std::cerr << "Fatal Error: " << e.what() << std::endl;
		std::cerr << "Press Enter to exit." << std::endl;
		std::cin.get();
		return 1; // Exit with an error code
		// ---------------------
	}
	catch (...)
	{
		std::cerr << "An unknown fatal error occurred." << std::endl;
		std::cerr << "Press Enter to exit." << std::endl;
		std::cin.get();
		return 2;
	}

	return 0; // Exit successfully
}