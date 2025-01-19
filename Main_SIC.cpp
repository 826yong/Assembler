
//   command line instructions:
//      g++ -o Main_SIC.exe Main_SIC.cpp
//      ./Main_SIC.exe Figure2.1.txt SIC

//   see the result at "symbolTab.txt", "output.txt"


#include<bits/stdc++.h>
using namespace std;

//loading the instruction 
struct Data{
	string operation;
	int type;
	int opcode;
};
vector<Data> dataList;
bool load(){
	ifstream inputFile("data.txt");
	if(!inputFile){
		cerr << "Failed to open file" << endl;
		return 0;  //fail
	}
	string line;
	while(getline(inputFile, line)){
		istringstream ss(line);
		Data data;
		ss >> data.operation >> data.type >> std::hex >> data.opcode;
		dataList.push_back(data);
	}	
	inputFile.close();
	return 1;  //success
}

struct Code{
	int location;
	string flag;
	string instruction;
	string operand;
	string opcode_f;
};



//be used to edit object program 
int find_opcode(const string& instruction){
	for(const auto& data : dataList){
		if(data.operation == instruction){
			return data.opcode;
		}
	}
	return -1;
}


//count the byte of instruction (format)
int count_byte(string& instruction, string& operand){
	if(instruction == "START" || instruction == "BASE"){
		return 0;
	}else if(instruction == "BYTE"){
		if(operand[0] == 'C' && operand[1] == '\''){
			return (operand.length()-3);
		}else if(operand[0] == 'X' && operand[1] == '\''){
			return (operand.length()-3)/2; 
		}
	}else if(instruction == "RESB"){
		return stoi(operand);
	}else if(instruction == "RESW"){
		return stoi(operand)*3;
	}else if(instruction[0] == '+'){
		return 4;  //format 4
	}else if(instruction[instruction.length() -1] == 'R'){
		return 2;  //format 2
	}else if(instruction == "FIX" || instruction == "FLOAT" || instruction == "NORM" || 
			 instruction == "HIO" || instruction == "SIO" || instruction == "TIO"){
		return 1;  //format 1
	}
	return 3;
}

//read the instruction
vector<Code> codeList;
bool read_inst(const string& filename){  //string reference
	ifstream inputFile(filename);
	if(!inputFile){
		cerr << "Failed to open file for reading" << endl;
		return 0;
	}
	string line;
	int loc = 0;
	bool isFirstLine = true;
	while(getline(inputFile, line)){
		//remove leading and trailing whitespace 
		line.erase(0, line.find_first_not_of(" \t")); 
        line.erase(line.find_last_not_of(" \t") + 1);

		//skip empty lines or command lines
		if (line.empty() || line[0]=='.') {
            continue;
        }

		istringstream ss(line);
		Code code;
		vector<string> parts;
		string part;
		while(ss >> part){
			parts.push_back(part);
		}

		if (isFirstLine && parts.size() >= 2 && parts[1] == "START") {
            isFirstLine = false;
            try {
                loc = stoi(parts[2], nullptr, 16); 
            } catch (...) {
                loc = 0; 
            }
            code.location = loc;
            code.flag = parts[0];
            code.instruction = parts[1];
            code.operand = parts[2];
            codeList.push_back(code);
            continue;
        }

		if(parts.size() == 3){
			code.location = loc;
			code.flag = parts[0];
			code.instruction = parts[1];
			code.operand = parts[2];
		}else if(parts.size() == 2){
			int exist = find_opcode(parts[0]);
			if(exist!=-1){
				code.location = loc;
				code.flag = "";
				code.instruction = parts[0];
				code.operand = parts[1];
			}else{
				code.location = loc;
				code.flag = parts[0];
				code.instruction = parts[1];
				code.operand = "";
			}
		}else if(parts.size() == 1){
			code.location = loc;
			code.flag = "";
			code.instruction = parts[0];
			code.operand = "";
		}else{
			cerr << " Invalid line: " << line << endl;
			continue;
		}
		codeList.push_back(code);
		loc += count_byte(code.instruction, code.operand);
	}
	inputFile.close();
	return 1;
}



//create symbol table (pass 1)
bool symTab(string filename){
	ofstream outputFile(filename);
	if(!outputFile){
		cerr << "Failed to open file for writing" << endl;
		return 0;
	}
	for (const auto& code : codeList) {
		if(!code.flag.empty()){
			outputFile << setfill('0') << setw(4) << hex << uppercase << code.location << "     ";
			outputFile << setfill(' ') << setw(6) << code.flag << endl;
		}
    }
	outputFile.close();
	return 1;
}



//using vector to store labels
struct label{
	int location;
	string flag;
};
vector<label> symnbolList;
bool read_sym(){
	ifstream inputfile("symbolTab.txt");
	if(!inputfile){
		cerr << "Failed to read symbol table..." << endl;
		return 0;
	}
	string line;
	while(getline(inputfile, line)){
		istringstream ss(line);
		label lab;
		ss >> hex >> lab.location >> lab.flag;
		symnbolList.push_back(lab);
	}
	inputfile.close();
	return 1;
} 



//find the location
string find_symbol(const string& operand){
	for(const auto& symbol : symnbolList){
		if(symbol.flag == operand){
			stringstream ss;
			ss << setfill('0') << setw(4) << hex << uppercase << symbol.location;
			return ss.str();
		}
	}
	return "";
}




//edit opcode program (pass 2) for SIC
bool object_code_SIC(){
	string s ="";
	for(auto& code : codeList){
		int opcode = find_opcode(code.instruction);
		if(opcode != -1){
			if(!code.operand.empty()){

				// index
				if(code.operand[code.operand.length()-1] == 'X' && code.operand[code.operand.length()-2] == ','){
					string operand = code.operand.substr(0, code.operand.size()-2);
					string loc = find_symbol(operand);
					if(!loc.empty()){
						int loc_int = stoi(loc, nullptr, 16);
						loc_int |= 0x8000; //set the high bit for indexed addressing
						stringstream ss;
						ss << setfill('0') << hex << uppercase << setw(2) << right << opcode << setw(4) << loc_int;
						code.opcode_f = ss.str();
					}

				}else{
					string loc = find_symbol(code.operand);
					if(!loc.empty()){
						stringstream ss;
						ss << setfill('0') << hex << uppercase << setw(2) << opcode << setw(4) << loc;
						code.opcode_f = ss.str();
					}else{
						stringstream ss;
						ss << setfill('0') << hex << uppercase << setw(2) << opcode << "0000";
						code.opcode_f = ss.str();
					}
				}
			}else{
				stringstream ss;
				ss << setfill('0') << hex << uppercase << setw(2) << opcode << "0000";
				code.opcode_f = ss.str();
			}
		}else{
			if(code.instruction == "BYTE"){
				if(code.operand[0] == 'X'){
					code.opcode_f = code.operand.substr(2, code.operand.length()-3);
				}if(code.operand[0] == 'C'){
					stringstream ss;
					for(int i=2; i< code.operand.length()-1; i++){
						ss << hex << uppercase << static_cast<int>(code.operand[i]);
					}
					code.opcode_f = ss.str();
				}
			}else if(code.instruction == "WORD"){
				stringstream ss;
				ss << setfill('0') << setw(6) << right << hex << uppercase << stoi(code.operand);
				code.opcode_f = ss.str();
			}
		}
	}
	return 1;
}

//write into output.txt
bool write_inst(string filename){
	ofstream outputFile(filename);
	if(!outputFile){
		cerr << "Failed to open output file for writing" << endl;
		return 0;
	}
	for (const auto& code : codeList) {
        outputFile << setfill('0') << setw(4) << hex << uppercase << code.location << "     ";
		outputFile << setfill(' ') << setw(6) << code.flag << "     ";
		outputFile << setfill(' ') << setw(6) << code.instruction << "     ";
		outputFile << setfill(' ') << setw(8) << code.operand << "     ";
		outputFile << setfill(' ') << setw(8) << code.opcode_f << endl;
    }
	outputFile.close();
	return 1;
}


int main(int argc, char* argv[]){
	if(argc < 3){
		cerr << "Usage: " << argv[0] << "<input file name>" << " type" << endl;
		return 1; //again
	}

	string inputFile = argv[1];
	string type = argv[2];

	cout << "Data loaded successfully? " << load() << endl;;
	cout << "Code loaded successfully? " << read_inst(inputFile) << endl;;
	cout << "Symbol table created successfully? " << symTab("symbolTab.txt") << endl;
	cout << "Symbol loaded successfully? " << read_sym() << endl;
	
	if(type == "SIC"){
		cout << "Object code generated successfully? " << object_code_SIC() << endl;
	}

	cout << "Output file successfully update? " << write_inst("output.txt") << endl;
	// cout << "Record created successfully? " << record("record.txt") << endl;

	return 0;
} 
