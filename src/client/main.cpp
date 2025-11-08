#include <iostream>
#include <string>
#include <limits> // For std::numeric_limits

/**
 * @brief Clears any error flags on std::cin and discards the buffer.
 *
 * Used after a failed input operation to restore std::cin to a usable state.
 */
void clearCinBuffer()
{
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

/**
 * @brief Displays the main menu to the user.
 */
void displayMenu()
{
	std::cout << "\nMessageU client at your service." << std::endl;
	std::cout << "110) Register" << std::endl;
	std::cout << "120) Request for clients list" << std::endl;
	std::cout << "130) Request for public key" << std::endl;
	std::cout << "140) Request for waiting messages" << std::endl;
	std::cout << "150) Send a text message" << std::endl;
	std::cout << "151) Send a request for symmetric key" << std::endl;
	std::cout << "152) Send your symmetric key" << std::endl;
	std::cout << "0) Exit client" << std::endl;
	std::cout << "? ";
}

/**
 * @brief Main entry point for the client application.
 */
int main()
{
	int selection = -1;

	while (true)
	{
		displayMenu();

		// Read user selection
		if (!(std::cin >> selection))
		{
			std::cout << "Error: Invalid input. Please enter a number." << std::endl;
			std::cin.clear(); // Clear error state
			clearCinBuffer(); // Discard the bad input
			continue; // Skip to the next loop iteration
		}

		// Discard any trailing characters (like the newline)
		clearCinBuffer();

		switch (selection)
		{
		case 110:
			std::cout << "Selected: (110) Register" << std::endl;
			// TODO: Implement registration logic
			break;
		case 120:
			std::cout << "Selected: (120) Request for clients list" << std::endl;
			// TODO: Implement client list logic
			break;
		case 130:
			std::cout << "Selected: (130) Request for public key" << std::endl;
			// TODO: Implement public key logic
			break;
		case 140:
			std::cout << "Selected: (140) Request for waiting messages" << std::endl;
			// TODO: Implement pull messages logic
			break;
		case 150:
			std::cout << "Selected: (150) Send a text message" << std::endl;
			// TODO: Implement send text logic
			break;
		case 151:
			std::cout << "Selected: (151) Send a request for symmetric key" << std::endl;
			// TODO: Implement request key logic
			break;
		case 152:
			std::cout << "Selected: (152) Send your symmetric key" << std::endl;
			// TODO: Implement send key logic
			break;
		case 0:
			std::cout << "Exiting. Goodbye!" << std::endl;
			return 0; // Exit the program
		default:
			std::cout << "Error: Unknown option selected. Please try again." << std::endl;
			break;
		}
	}

	return 0; // Should never be reached
}