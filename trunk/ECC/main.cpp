#define _CRT_SECURE_NO_WARNINGS

#include <fstream>
#include <ctime>

#include "configsystem.h"
#include "crypto.h"
#include "2nfactory.h"
#include "helpers.h"

using namespace helpers;

// Stream variables
std::ifstream fin;
std::ifstream ftext;
std::ofstream fout;
std::ofstream fenc;

gf2n *field;
ecurve *curve;
crypto *transform;

// Opens all streams.
void open_files(void)
{
	fin.open(INPUT_FILE_NAME);
	ftext.open(TEXT_FILE_NAME, std::ios::binary | std::ios::in);
	fout.open(OUTPUT_FILE_NAME);
	fenc.open(CIPHER_FILE_NAME, std::ios::binary | std::ios::out);
}

// Closes all streams.
void close_files(void)
{
	fin.close();
	ftext.close();
	fout.close();
	fenc.close();
}

int main(void)
{
	srand((unsigned int)time(0));
	open_files();
	field = read_field(fin);
	field->set_output_mode(FIELD_OUTPUT_MODE);
	std::cout << "[+] Created field (degree " << field->get_deg() << ")." << std::endl;
	curve = read_curve(fin, *field);
	if (!curve->has_factor())
		if (ask_question("", "Should we try to calculate factorization?"))
			calc_factorization(*curve);
	if (!curve->has_factor())
	{
		std::cout << "[-] No factorization of curves order avaliable => exiting." << std::endl;
	}
	else
	{
		bint curveOrder;
		int message_length;
		curve->order(curveOrder);
		std::cout << "[+] Created elliptic curve (order " << curveOrder << ")." << std::endl;
		transform = new crypto(*curve);
		message_length = transform->get_message_length();
		if (message_length <= 0)
		{
			std::cout << "[-] Unable to encrypt data - field is too small." << std::endl;
		}
		else
		{
			std::cout << "[+] Created crypto object (sngle message length " << message_length << ")." << std::endl;
			bool aborted;
			epoint initial_point = choose_initial_point("", aborted, *curve);
			if (!aborted)
			{
				transform->set_G(initial_point);
				bint groupOrder = transform->get_group_order();
				bint keyA, keyB;
				if (!choose_key(keyA, groupOrder) || ! choose_key(keyB, groupOrder))
				{
					std::cout << "[-] Unable to encrypt data - unable to choose keys - group is too small." << std::endl;
				}
				else
				{
					transform->set_keyA(keyA);
					transform->set_keyB(keyB);
					if (!output_crack_data(fout, *transform))
						std::cout << "[-] Unable to output crack data (unknown error)." << std::endl;
					int text_length;
					unsigned char *str = read_till_end(ftext, text_length);
					int point_count;
					epoint *points = transform->encrypt(str, text_length + 1, point_count); // including the last zero byte, since we're encrypting ASCII text.
					if (points == 0)
					{
						std::cout << "    [-] Unable to encrypt data - unable to mark single message." << std::endl;
					}
					else
					{
						std::cout << "    [+] Encrypted text (" << point_count << " single messages)." << std::endl;
						std::cout << "        [i] Private key for Alice: " << transform->get_keyA() << std::endl;
						std::cout << "        [i] Private key for Bob: " << transform->get_keyB() << std::endl;
						if (!output_encrypted_data(fenc, points, point_count))
							std::cout << "        [-] Unable to output encrypted data." << std::endl;
						//int data_length;
						//cout << "    [i] Decrypted text: " << transform->decrypt(points, point_count, data_length) << endl;
					}
					delete[] points;
					delete[] str;
				}
			}
		}
	}
	close_files();
	std::cout << "[i] Terminating programme..." << std::endl;
	std::cin.get();
	return 0;
}
