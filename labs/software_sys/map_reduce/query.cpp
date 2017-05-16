/******************
  query.cpp
  Written by Drew Nash
  CS 560 - Programming Assignment 2
  Drew Nash - anash4@utk.edu
  Mahendra Duwal Shrestha - mduwalsh@utk.edu

  This program processes search queries for the users and returns locations of text within the input file(s).
 *******************/

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <map>
#include <set>
#include <vector>
using namespace std;

/* Search class */
class Search {
	protected:
		map <string, vector<string> > index;
		map <string, vector<string> >::iterator index_it;
		map <string, int> files;
		map <string, int>::iterator files_it;
		set <string> stopwords;
		set <string>::iterator stopwords_it;
		map <string, vector<int> > results;
		map <string, vector<int> >::iterator results_it;
		vector <vector <string> > query;
		int notmode;
		int change;
		vector <string> words;
		vector <string> locations;
		map <string, string> tmp;
		map <string, string>::iterator tmp_it;
		map <string, vector <int> > resultstmp;
		map <string, vector <int> >::iterator resultstmp_it;
	public:
		void load_index(string&);
		void load_query(string&, char&);
		void load_stopwords(string&);
		void initialize_results();
		void clear_results();
		void clear_resultstmp();
		void run_query();
		void run_words();
		void combine_results();
		void print_results();
};

/* Load index into map */
void Search::load_index(string &infile){
	ifstream ifs;
	string buffer, key, value, file, lines;
	vector<string> values;
	int i, mode, line;

	ifs.open(infile.c_str());
	if (ifs.fail()){
		cerr << "Error: Could not open " << infile << endl;
		exit(1);
	}

	mode = 0;
	while(getline(ifs, buffer)){
		mode = 1;
		key.clear();
		value.clear();
		values.clear();
		for (i = 0; i < buffer.size(); i++){
			if (mode == 1 && buffer[i] != ' ' && buffer[i] != '\t'){
				if (buffer[i] >= 'a' && buffer[i] <= 'z'){
					buffer[i] -= 32;
				}
				key.push_back(buffer[i]);
			}
			else if ((mode == 1 || mode == 2) && (buffer[i] == ' ' || buffer[i] == '\t')){
				mode = 2;
				file.clear();
			}
			else if (mode == 2 && buffer[i] != '@'){
				value.push_back(buffer[i]);
				file.push_back(buffer[i]);
			}
			else if (mode == 2 && buffer[i] == '@'){
				value.push_back(buffer[i]);
				mode = 3;
				lines.clear();
			}
			else if (mode == 3 && buffer[i] != '@'){
				value.push_back(buffer[i]);
				lines.push_back(buffer[i]);
			}
			else if (mode == 3 && buffer[i] == '@'){
				value.push_back(buffer[i]);
				mode = 4;
				line = atoi(lines.c_str());
				files_it = files.find(file);
				if (files_it == files.end()){
					files.insert(make_pair(file, line));
				} else {
					if (files_it->second < line){
						files_it->second = line;
					}
				}
			}
			else if (mode == 4 && buffer[i] != ',' && i != (buffer.size() - 1)){
				value.push_back(buffer[i]);
			}
			else if (mode == 4 && buffer[i] == ','){
				values.push_back(value);
				value.clear();
				mode = 2;
				file.clear();
			}
			else if (mode == 4 && buffer[i] != ',' && i == (buffer.size() - 1)){
				value.push_back(buffer[i]);
				values.push_back(value);
				value.clear();
				mode = 5;
			}
		}
		
		index_it = index.find(key);
		if (index_it == index.end() && mode == 5){
			index.insert(make_pair(key, values));
		} else {
			cerr << "Error: Inverted index file not formatted correctly. Run MapReduce again." << endl;
			exit(1);
		}
		buffer.clear();
	}
	ifs.close();
	 
	cout << "Inverted index file loaded successfully." << endl;
	return;
}

/* Initialize results map to all false */
void Search::initialize_results(){
	string file;
	int size;
	vector <int> tmpv;

	change = 0;
	for (files_it = files.begin(); files_it != files.end(); files_it++){
		file = files_it->first;
		size = files_it->second;
		tmpv.assign(size, 0);
		results.insert(make_pair(file, tmpv));
		resultstmp.insert(make_pair(file, tmpv));
	}

	return;
}

/* Clear results for next query */
void Search::clear_results(){
	change = 0;
	for (results_it = results.begin(); results_it != results.end(); results_it++){
		results_it->second.assign(results_it->second.size(), 0);
	}
	return;
}

/* Clear temporary results for further use */
void Search::clear_resultstmp(){
	for (resultstmp_it = resultstmp.begin(); resultstmp_it != resultstmp.end(); resultstmp_it++){
		resultstmp_it->second.assign(resultstmp_it->second.size(), 0);
	}
	return;
}

/* Load stop words into set */
void Search::load_stopwords(string &infile){
	ifstream ifs;
	string buffer, buffer2;
	int i;

	ifs.open(infile.c_str());
	if (ifs.fail()){
		cerr << "Error: Could not open " << infile << endl;
		exit(1);
	}
	while (getline(ifs, buffer)){
		buffer2.clear();
		for (i = 0; i < buffer.size(); i++){
			if (buffer[i] >= 'a' && buffer[i] <= 'z'){
				buffer[i] -= 32;
			}
		}
		for (i = 0; i < buffer.size(); i++){
			if (buffer[i] >= 'A' && buffer[i] <= 'Z'){
				buffer2.push_back(buffer[i]);
			}
		}
		stopwords_it = stopwords.find(buffer2);
		if (stopwords_it != stopwords.end()){
			cerr << "Error: Stopwords file not formatted correctly. Run MapReduce again." << endl;
			exit(1);
		} else {
			stopwords.insert(buffer2);
		}
	}
	ifs.close();
	cout << "Stopwords file loaded successfully." << endl;
	return;				
}

/* Load each query */
void Search::load_query(string &buffer, char &mode){
	istringstream iss;
	string word;
	vector <string> tmpv;

	query.clear();

	iss.clear();
	iss.str(buffer);

	tmp.clear();

	if (mode == 'y'){
		while (iss >> word){
			if (word.compare("AND") == 0){
				if (tmpv.size() > 1){
					query.push_back(tmpv);
				}
				else if (tmpv.size() == 1 && (tmpv[0].compare("OR") != 0) && (tmpv[0].compare("AND") != 0) && (tmpv[0].compare("NOT") != 0)){
					query.push_back(tmpv);
				}
				tmpv.clear();
			}
			else if (word.compare("NOT") == 0){
				if (tmpv.size() > 0 && (tmpv[tmpv.size()-1].compare("OR") == 0)){
					tmpv.push_back(word);
				}
				else if (tmpv.size() > 0 && (tmpv[tmpv.size()-1].compare("NOT") != 0)){
					query.push_back(tmpv);
					tmpv.clear();
					tmpv.push_back(word);
				}
				else if (tmpv.size() == 0){
					tmpv.push_back(word);
				}
			}
			else if (word.compare("OR") == 0){
				if (tmpv.size() > 0){
					tmpv.push_back(word);
				}
			} else {
				tmpv.push_back(word);
			}
		}

		if (tmpv.size() != 0){
			query.push_back(tmpv);
		}
	}
	else if (mode == 'n'){
		while (iss >> word){
			tmpv.push_back(word);
			query.push_back(tmpv);
			tmpv.clear();
		}
	}

	return;
}

/* Run each query part separately */
void Search::run_query(){
	int i, j, k, mode, edit;

	edit = 0;
	for (i = 0; i < query.size(); i++){
		notmode = 0;
		edit = 0;
		tmp.clear();
		words.clear();
		locations.clear();
		mode = 0;
		clear_resultstmp();
		for (j = 0; j < query[i].size(); j++){
			if (query[i][j].compare("NOT") != 0 && query[i][j].compare("OR") != 0){
				stopwords_it = stopwords.find(query[i][j]);
				if (stopwords_it == stopwords.end()){
					words.push_back(query[i][j]);
					index_it = index.find(query[i][j]);
					if (index_it != index.end()){
						for (k = 0; k < index_it->second.size(); k++){
							if (mode == 0){
								locations.push_back(index_it->second[k]);
							} else {
								tmp_it = tmp.find(index_it->second[k]);
								if (tmp_it == tmp.end()){
									tmp.insert(make_pair(index_it->second[k], query[i][j]));
								}
							}
						}
					}
					mode = 1;
				}
				else if (mode == 1){
					words.push_back("");
				}
			}
			else if (query[i][j].compare("NOT") == 0){
				notmode = 1;
			}
			else if (query[i][j].compare("OR") == 0){
				if (mode == 1){
					run_words();
					edit = 1;
				}
				notmode = 0;
				tmp.clear();
				words.clear();
				locations.clear();
				mode = 0;
			}
		}
		if (mode == 1){
			run_words();
			edit = 1;
		}
		if (edit == 1){
			combine_results();
		}
	}

	print_results();
	return;
}

/* Run the given words in the query */
void Search::run_words(){
	int i, j, k, mode;
	string s, s2, s3;
	string file;
	int line, tmpi;
	vector <int> tmpv;
	map <string, vector <int> > notresults;
	map <string, vector <int> >::iterator notresults_it;
	istringstream iss;
	ostringstream oss;

	if (notmode == 1){
		for (files_it = files.begin(); files_it != files.end(); files_it++){
			file = files_it->first;
			line = files_it->second;
			tmpv.assign(line, 1);
			notresults.insert(make_pair(file, tmpv));
		}
	}

	for (i = 0; i < locations.size(); i++){
		s = locations[i];
		j = 0;
		for (j = 1; j < words.size(); j++){
			if (words[j].size() > 0){
				s = locations[i];
				mode = 0;
				s2.clear();
				s3.clear();
				for (k = 0; k < s.size(); k++){
					if (mode == 0 && s[k] == '@'){
						mode = 1;
						s3.push_back(s[k]);
					}
					else if (mode == 1 && s[k] == '@'){
						mode = 2;
						s3.push_back(s[k]);
					}
					else if (mode == 2){
						s2.push_back(s[k]);
					} else {
						s3.push_back(s[k]);
					}
				}
				iss.clear();
				iss.str(s2);
				iss >> tmpi;
				tmpi += j;
				oss.clear();
				oss.str("");
				oss << tmpi;
				s2 = oss.str();
				s3 += s2;
				tmp_it = tmp.find(s3);
				if (tmp_it == tmp.end()){
					locations[i] = "";
				}
				else if (tmp_it->second != words[j]){
					locations[i] = "";
				}
			}
		}
	}

	for (i = 0; i < locations.size(); i++){
		mode = 0;
		file.clear();
		s.clear();
		if (locations[i].size() > 0){
			for (j = 0; j < locations[i].size(); j++){
				if (mode == 0 && locations[i][j] != '@'){
					file.push_back(locations[i][j]);
				}
				else if (mode == 0 && locations[i][j] == '@'){
					mode = 1;
				}
				else if (mode == 1 && locations[i][j] != '@'){
					s.push_back(locations[i][j]);
				}
				else if (mode == 1 && locations[i][j] == '@'){
					break;
				}
			}
		}
		line = atoi(s.c_str());
		if (notmode == 1){
			notresults_it = notresults.find(file);
			if (notresults_it != notresults.end()){
				notresults_it->second[line-1] += 1;
				notresults_it->second[line-1] %= 2;
			}
		} else {
			resultstmp_it = resultstmp.find(file);
			if (resultstmp_it != resultstmp.end()){
				if (resultstmp_it->second[line-1] != 1){
					resultstmp_it->second[line-1] = 1;
				}
			}
		}
	}

	if (notmode == 1){
		resultstmp_it = resultstmp.begin();
		for (notresults_it = notresults.begin(); notresults_it != notresults.end(); notresults_it++){
			for (i = 0; i < notresults_it->second.size(); i++){
				if (notresults_it->second[i] == 1){
					resultstmp_it->second[i] = 1;
				}
			}
			resultstmp_it++;
		}
	}
	return;
}

/* Combine results for and */
void Search::combine_results(){
	int i = 0;

	if (change == 0){
		results_it = results.begin();
		for (resultstmp_it = resultstmp.begin(); resultstmp_it != resultstmp.end(); resultstmp_it++){
			for (i = 0; i < results_it->second.size(); i++){
				results_it->second[i] = resultstmp_it->second[i];
			}
			results_it++;
		}
		change = 1;
	} else {
		results_it = results.begin();
		for (resultstmp_it = resultstmp.begin(); resultstmp_it != resultstmp.end(); resultstmp_it++){
			for (i = 0; i < resultstmp_it->second.size(); i++){
				if (results_it->second[i] == 1 && resultstmp_it->second[i] == 1){
					results_it->second[i] = 1;
				} else {
					results_it->second[i] = 0;
				}
			}
			results_it++;
		}
	}

	clear_resultstmp();
	return;
}

/* Print the contents of the results map */
void Search::print_results(){
	int i = 0;
	int printed = 0;

	cout << "Query was found in the following locations:" << endl;
	for (results_it = results.begin(); results_it != results.end(); results_it++){
		printed = 0;
		cout << "File: " << results_it->first << endl;
		cout << "  Lines: ";
		for (i = 0; i < results_it->second.size(); i++){
			if (results_it->second[i] == 1){
				if (printed){ cout << ", "; }
				cout << i+1;
				printed = 1;
			}
		}
		if (!printed){
			cout << "None";
		}
		cout << endl;
	}
	cout << endl;
	return;
}

/* Collect prompts from user */
int main(){
	string infile;
	string input, buffer, buffer2;
	int i, go;
	char mode, details;

	Search keywords;

	cout << "Enter name of the inverted index file generated by MapReduce:" << endl;
	cin >> infile;
	keywords.load_index(infile);

	cout << endl << "Enter name of the stopwords file generated by MapReduce:" << endl;
	cin >> infile;
	keywords.load_stopwords(infile);

	keywords.initialize_results();

	go = 0;
	while (!go){
		cout << endl << "Extra credit mode? Enter 'y' or 'n':" << endl;
		cin >> mode;
		if (mode == 'y' || mode == ('y' - 32)){
			mode = 'y';
			cout << "EC mode on: Order of words in query matters and use of 'and', 'or', 'not' is supported." << endl;
			go = 1;
		}
		else if (mode == 'n' || mode == ('n' - 32)){
			mode = 'n';
			cout << "EC mode off: Order of words in query is ignored and use of 'and', 'or', 'not' is NOT supported." << endl;
			go = 1;
		} else {
			cout << "Error: Invalid mode entered." << endl;
		}
	}

	go = 0;
	while (!go){
		cout << endl << "Show query details? Enter 'y' or 'n':" << endl;
		cin >> details;
		if (details == 'y' || details == ('y' - 32)){
			details = 'y';
			cout << "Query details mode on." << endl;
			go = 1;
		}
		else if (details == 'n' || details == ('n' - 32)){
			details = 'n';
			cout << "Query details mode off." << endl;
			go = 1;
		} else {
			cout << "Error: Invalid mode entered." << endl;
		}
	}

	if (mode == 'n'){
		cout << endl << "To make a query, type one or more words, separated by spaces." << endl;
	}
	else if (mode == 'y'){
		cout << endl << "To make a query, type one or more phrases, separated by ' and ', ' or ', ' not '." << endl;
		cout << "The query should be in Conjunctive Normal Form, with ' not ' meaning \"and not\"." << endl;
	}

	cout << "Note that hyphens are treated as blank spaces, and other punctuation is ignored." << endl;
	cout << "See README for details. Send CTRL+D signal to exit." << endl;
	cout << "Enter queries:" << endl;

	buffer.clear();
	input.clear();
	cin.ignore();
	while(getline(cin, input)){
		buffer = input;
		for (i = 0; i < buffer.size(); i++){
			if ((buffer[i] >= 'A' && buffer[i] <= 'Z') || (buffer[i] >= '0' && buffer[i] <= '9')){
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

		if (details == 'y'){
			cout << "Query: " << input << endl;
			cout << "Conversion: " << buffer2 << endl;
		}
	
		keywords.clear_results();
		keywords.load_query(buffer2, mode);
		keywords.run_query();
	}
	return 0;
} 
