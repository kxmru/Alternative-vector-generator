#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <array>
#include <utility>

#define LETTER_AMOUNT 52

struct AltVectorElement {
	char symb;
	int alternative;
	int keyword;
	AltVectorElement(char s) : symb(s), alternative(-1), keyword(-1) {}
	AltVectorElement(char s, int ind) : symb(s), alternative(-1), keyword(ind) {}
};

void AddWord(std::vector<AltVectorElement>& alt_vec, std::string::iterator start, std::string::iterator end, int word_ind)
{
	end--;
	while(start != end)	alt_vec.push_back(AltVectorElement(*start++));

	alt_vec.push_back(AltVectorElement(*start, word_ind));
}

std::pair<std::array<int, LETTER_AMOUNT>, std::vector<AltVectorElement>> GenerateVectors(std::vector<std::string>& keywords)
{
	std::vector<AltVectorElement> alt_vec;
	std::array<int, 52> indexes;
	indexes.fill(-1);

	char start;
	for (int i = 0, keyword_amount = static_cast<int>(keywords.size()); i != keyword_amount; i++) {
		start = keywords[i][0] - (isupper(keywords[i][0]) ? 'A' : 'a' - 26);

		if (indexes[start] == -1) {
			indexes[start] = static_cast<int>(alt_vec.size());
			AddWord(alt_vec, keywords[i].begin() + 1, keywords[i].end(), i);
			continue;
		}

		int pos = indexes[start] ;
		int matches = 1;
		for (int size = alt_vec.size(); keywords[i][matches] == alt_vec[pos].symb && pos != size; matches++, pos++) {
			if (alt_vec[pos].keyword != -1) {
				std::cout << "Keyword can't begin with another keyword\n";
				return { };
			}
		}

		while(alt_vec[pos].alternative != -1)
			pos = alt_vec[pos].alternative;
		alt_vec[pos].alternative = static_cast<int>(alt_vec.size());
		AddWord(alt_vec, keywords[i].begin() + matches, keywords[i].end(), i);
	}
	return { indexes, alt_vec };
}

std::vector<std::string> ReadFile(std::string& path)
{
	std::vector<std::string> keywords;

	std::ifstream stream(path);
	if (!stream.is_open()) {
		std::cerr << "File open error";
		return std::vector<std::string>(0);
	}

	std::string buffer;
	while (stream >> buffer) {
		keywords.push_back(buffer);
	}
	stream.close();
	return keywords;
}

void PrintVectors(std::array<int, LETTER_AMOUNT>& ind, std::vector<AltVectorElement>& alt, std::vector<std::string>& keywords)
{
	char letter = 'A';
	for (int i = 0; i != LETTER_AMOUNT; i++) {
		if (letter == 'Z' + 1)
			letter = 'a';

		std::cout << letter++;
		if (ind[i] != -1)
			std::cout << ' ' << ind[i];
		std::cout << '\n';
	}
	std::cout << '\n';

	for (int i = 0, size = alt.size(); i != size; i++) {
		std::cout << i << '\t';
		std::cout << alt[i].symb;
		if (alt[i].alternative != -1)
			std::cout << '\t' << alt[i].alternative;
		if (alt[i].keyword != -1) {
			if (alt[i].alternative != -1)
				std::cout << '\t' << alt[i].keyword;
			std::cout << '\t' << '\t' << keywords[alt[i].keyword];
		}
		std::cout << '\n';
	}
}

bool WriteIndVec(const std::string& path, const std::array<int, LETTER_AMOUNT>& arr)
{
	std::ofstream stream(path, std::ios::binary);
	if (!stream.is_open()) {
		std::cerr << "Index vector file open error\n";
		return false;
	}

	stream.write(reinterpret_cast<const char*>(arr.data()), arr.size() * sizeof(int));
	return true;
}

bool WriteAltVec(const std::string& path, const std::vector<AltVectorElement>& vec)
{
	std::ofstream stream(path, std::ios::binary);
	if (!stream.is_open()) {
		std::cerr << "Index vector file open error\n";
		return false;
	}

	stream.write(reinterpret_cast<const char*>(vec.data()), vec.size() * sizeof(AltVectorElement));
	return true;
}

int main(int argc, char* argv[])
{
	std::string key_path;
	std::string ind_path;
	std::string vec_path;

	bool print = true;
	bool write = true;

	switch (argc) {
	case 4:
		ind_path = argv[2];
		vec_path = argv[3];
		[[fallthrough]];
	case 2:
		key_path = argv[1];
		break;
	default:
		std::cout << "Usage: <keywords file> <ind vect.bin> <alt vec.bin>" << std::endl;
		return 0;
	}

	std::vector<std::string> keywords = ReadFile(key_path);
	auto[ind_vec, alt_vec] = GenerateVectors(keywords);

	if (alt_vec.size() == 0)
		return 0;

	if (print) {
		PrintVectors(ind_vec, alt_vec, keywords);
	}

	WriteIndVec("ind_vec.txt", ind_vec);
	WriteAltVec("alt_vec.txt", alt_vec);

	return 0;
}