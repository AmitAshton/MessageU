#include "Base64Wrapper.h"


std::string Base64Wrapper::encode(const std::string& str)
{
	std::string encoded;
	CryptoPP::StringSource ss(str, true,
		// Disable line breaks so the output is a single continuous Base64 string
		new CryptoPP::Base64Encoder(
			new CryptoPP::StringSink(encoded),
			false // insertLineBreaks = false
		) // Base64Encoder
	); // StringSource

	return encoded;
}

std::string Base64Wrapper::encode(const uint8_t* data, size_t len)
{
	std::string encoded;
	CryptoPP::StringSource ss(data, len, true,
		new CryptoPP::Base64Encoder(
			new CryptoPP::StringSink(encoded),
			false // single-line output
		)
	);
	return encoded;
}

std::string Base64Wrapper::decode(const std::string& str)
{
	std::string decoded;
	CryptoPP::StringSource ss(str, true,
		new CryptoPP::Base64Decoder(
			new CryptoPP::StringSink(decoded)
		) // Base64Decoder
	); // StringSource

	return decoded;
}