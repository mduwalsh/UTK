/******************
preprocess.cpp
Written by Drew Nash
CS 560 - PA 2
Drew Nash - anash4@utk.edu
Mahendra Duwal Shrestha - mduwalsh@utk.edu

This program prepares the text input files for use with MapReduce by converted the case, removing
symbols, and prepending line numbers.
*******************/

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdlib>
using namespace std;

int main(){
	string infile;
	string outfile;
	string buffer, buffer2;
	ifstream ifs;
	ofstream ofs;
	ostringstream oss, oss2;
	istringstream iss;
	int i, line, word, go;
	
	/* Allow user to choose file names */
	cout << "Enter name of input file for text processing:" << endl;
	cin >> infile;

	ifs.open(infile.c_str());
	if (ifs.fail()){
		cerr << "Error: Could not open " << infile << endl;
		exit(1);
	}

	go = 0;
	
	/* Ensure that user does not choose same name as input file */
	while(!go){
		cout << "Enter desired name for output file of processed text:" << endl;
		cin >> outfile;
		if (infile.compare(outfile) == 0){
			cerr << "Error: Input and output file names cannot be the same." << endl;
		} else {
			go = 1;
		}
	}

	ofs.open(outfile.c_str());
	if (ofs.fail()){
		cerr << "Error: Could not open " << outfile << endl;
		exit(1);
	}
	
	/* Parse through file, removing all punctuation. Punctuation in the middle of
	   a word is just removed, so "that's" becomes "thats" but hyphens become spaces
	   so "fare-well" becomes "fare well". Also, all letters are converted to upper
	   case */
	line = 0;
	buffer.clear();
	while(getline(ifs, buffer)){
		for (i = 0; i < buffer.size(); i++){
			if (buffer[i] >= 'A' && buffer[i] <= 'Z'){
				continue;
			}
			else if (buffer[i] >= 'a' && buffer[i] <= 'z'){
				buffer[i] -= 32;
			}
			else if (buffer[i] == '-'){
				buffer[i] = ' ';
			}
		}

		buffer2.clear();
		for (i = 0; i < buffer.size(); i++){
			if ((buffer[i] >= 'A' && buffer[i] <= 'Z') || (buffer[i] >= '0' && buffer[i] <= '9')){
				buffer2.push_back(buffer[i]);
			}
			else if (buffer[i] == ' '){
				buffer2.push_back(buffer[i]);
			}
		}
		
		/* Use stringstreams to print to file */
		line++;
		iss.clear();
		iss.str(buffer2);
		oss.clear();
		oss.str("");
		oss << line << " ";
		word = 0;
		buffer.clear();
		while(iss >> buffer){
			word++;
			oss2.clear();
			oss2.str("");
			oss2 << buffer << " ";
			buffer.clear();
			buffer = oss2.str();
			oss << buffer;
		}
		oss << endl;
		buffer.clear();
		buffer = oss.str();
		ofs << buffer;
	}
	ifs.close();
	ofs.close();
	return 0;
}
