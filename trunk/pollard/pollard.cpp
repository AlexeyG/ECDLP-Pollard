#include <fstream>
#include <ctime>

#include "configpollard.h"
#include "crack.h"
#include "../ecc/2nfactory.h"
#include "../ecc/crypto.h"
#include "../ecc/bint.h"
#include "../ecc/helpers.h"

using namespace helpers;

// Stream variables
std::ifstream fin;
std::ifstream fenc;
std::ofstream fout;

gf2n *field;
ecurve *curve;
crypto *transform;
crack *pollard;

void open_files(void)
{
	fin.open(INPUT_FILE_NAME);
	fenc.open(CIPHER_FILE_NAME, std::ios::binary | std::ios::in);
	fout.open(OUTPUT_FILE_NAME, std::ios::binary | std::ios::out);
}

void close_files(void)
{
	fin.close();
	fenc.close();
	fout.close();
}

int main(void)
{
	srand((unsigned int)time(0));
	open_files();
	field = read_field(fin);
	std::cout << "[+] Created field (degree " << field->get_deg() << ")." << std::endl;
	curve = read_curve(fin, *field);
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
		epoint G = read_next_point(fin, *curve);
		epoint aG = read_next_point(fin, *curve);
		epoint bG = read_next_point(fin, *curve);
		pollard = new crack(*curve);
		if (!G.check() || !aG.check() || !bG.check())
		{
			std::cout << "[-] Incorrect input data - points do not belong to curve => exiting." << std::endl;
		}
		else
		{
			pollard->set_G(G);
			pollard->set_xG(bG);
			bint solution;
			double work_time;
			if (!pollard->solve(solution, true, "    ", work_time))
			{
				std::cout << "[-] Unable to solve ECDLP." << std::endl;
			}
			else
			{
				char *time_string = new char[LINE_LEN];
				sprintf(time_string, "%.5lf", work_time);
				std::cout << "[+] Solved ECDLP (Bobs key is " << solution << ") in " << time_string << " seconds." << std::endl;
				delete[] time_string;
				transform = new crypto(*curve);
				message_length = transform->get_message_length();
				if (message_length <= 0)
				{
					std::cout << "    [-] Unable to decrypt data - field is too small." << std::endl;
				}
				else
				{
					transform->set_G(G);
					transform->set_keyB(solution);
					transform->override_aG(aG, true);
					int encrypted_data_length;
					unsigned char *encrypted_data = read_till_end(fenc, encrypted_data_length);
					int decrypted_data_length;
					unsigned char *decrypted_data = transform->decrypt(encrypted_data, encrypted_data_length, decrypted_data_length);
					output_decrypted_data(fout, decrypted_data, decrypted_data_length - 1); // Cut out the zero string termination char.
					delete[] encrypted_data;
					delete[] decrypted_data;
				}
			}
		}
	}
	close_files();
	std::cout << "[i] Terminating programme..." << std::endl;
	std::cin.get();
	return 0;
}
