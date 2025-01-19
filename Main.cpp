
//   command line instructions:
//      "g++ -o Main.exe Main.cpp"
//      "./Main.exe Figure2._.txt SIC_" 

//   see the result at "symbolTab.txt", "output.txt", and "record.txt"


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
			// if (parts[1] == "EQU") {
			// 	if (parts[2] != "*") {
					
			// 	} 

			code.location = loc;
			code.flag = parts[0];
			code.instruction = parts[1];
			code.operand = parts[2];
		}else if(parts.size() == 2){

			if (parts[0] == "BASE") {
				code.location = loc;
				code.flag = "";
				code.instruction = parts[0];
				code.operand = parts[1];
				codeList.push_back(code);
				continue;  // Skip location update for BASE
			}

			if (parts[0] == "END") {
				code.location = loc;
				code.flag = "";
				code.instruction = parts[0];
				code.operand = parts[1];
				codeList.push_back(code);
				continue;  // Skip location update for END
			}

			if (parts[0] == "*") {
				code.location = loc;
				code.flag = parts[0];
				code.instruction = parts[1];
				code.operand = "";
				codeList.push_back(code);
				continue; 
			}

			string newPart = "";
			if (parts[0].substr(0, 1) == "+"){

				// if it's format 4 instruction
				newPart = parts[0].substr(1);

			} else {
				newPart = parts[0];
			}

			int exist = find_opcode(newPart);
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


// count the target address
int compute_target_address(const string& operand) {
	try {
		if (!operand.empty()) {

			// immediate addressing mode
			if (operand[0] == '#') { 
				string value = operand.substr(1);
				try {
					return stoi(value); // return decimal
				} catch (...) {
					string resolved = find_symbol(value);
					if (!resolved.empty()) return stoi(resolved, nullptr, 16);
					cerr << "Invalid immediate value: " << operand << endl;
					return -1;	
				}	
			}

			// indirect addressing mode
			if (operand[0] == '@') { 
				string symbol = operand.substr(1);
				string sym_add = find_symbol(symbol);
				if (!sym_add.empty()) {
					return stoi(sym_add, nullptr, 16);
				} else {
					cerr << "Undefind symbol for indirect addressing: " << symbol << endl;
					return -1;
				}
			}

			//index addressing mode
			if (operand.substr(operand.size() - 2) == ",X") {
				string symbol = operand.substr(0, operand.size() - 2);
				string sym_add = find_symbol(symbol);
				if (!sym_add.empty()) {
					return stoi(sym_add, nullptr, 16);
				} else {
					cerr << "Undefined symbol for indexed addressing: " << symbol << endl;
					return -1;
				}
			}

			// others, just return address
			string sym_add = find_symbol(operand);
			if (!sym_add.empty()) {
				return stoi(sym_add, nullptr, 16);
			} else {
				cerr << "Undefined symbol: " << operand << endl;
				return -1;
			}
		}
	} catch (...) {
		cerr << "Error computing target address for operand: " << operand << endl;
		return -1;
	}

	return 1;
}


map<string , int> reg_map = {
	{"A", 0}, {"X", 1}, {"L", 2}, {"B", 3}, {"S", 4}, {"T", 5}, {"F", 6}, {"PC", 8}, {"SW", 9} 
};

int find_register(const string& name) {
	if (reg_map.find(name) != reg_map.end()) {
		return reg_map[name];
	}
	return -1;
}


//edit opcode program (pass 2) for SIC/XE
bool object_code_XE(){
	int BASE = -1;
	for(auto& code : codeList){
		int format = count_byte(code.instruction, code.operand);
		int opcode = 0;

		if (format == 4) {
			string newInstruction = code.instruction.substr(1);
			opcode = find_opcode(newInstruction);
		} else {
			opcode = find_opcode(code.instruction);
		}

		int TA, PC = code.location + format;
		stringstream ss;

		if (code.instruction == "BASE") {
			BASE = stoi(find_symbol(code.operand), nullptr, 16);
		}
		
		if (opcode != -1) {
			if(!code.operand.empty()){
				switch (format){

					case 1:  // format 1
						ss << setfill('0') << setw(2) << right << hex << uppercase << opcode;
						code.opcode_f = ss.str();
						break;

					case 2: {  // format 2
						string reg1, reg2;
						istringstream iss(code.operand);
						getline(iss, reg1, ',');
						getline(iss, reg2, ',');

						int reg1_code = find_register(reg1);
						int reg2_code = reg2.empty() ? 0 : find_register(reg2);

						if (reg1_code != -1 && reg2_code != -1) {
							ss << setfill('0') << setw(2) << right << hex << uppercase << opcode;
							ss << setw(1) << hex << uppercase << reg1_code << setw(1) << hex << uppercase <<reg2_code;
							code.opcode_f = ss.str(); 
						}
						break;
					}

					case 3:
					case 4: {  // format 3 and 4
						int e = (format == 4) ? 1 : 0;
						int n = 1, i = 1, x = 0, b = 0, p = 0;
						int disp = 0;
						TA = compute_target_address(code.operand);

						if (TA != -1) {
							if (e == 1) {
								disp = TA;
							} else {
								disp = TA - PC;
								if (abs(disp - 0) < 0x1000) {
									p = 1;
									b = 0;
								} else {
									disp = TA - BASE;
									b = 1;
									p = 0;
								}  
							}
				
							if (code.operand.substr(code.operand.size() - 2) == ",X") {
								x = 1;
							} else if (code.operand[0] == '@') {
								n = 1; 
								i = 0;
							} else if (code.operand[0] == '#') {
								n = 0; 
								i = 1;
								b = 0;
								p = 0;

								int digit = 1;
								for (char c : code.operand.substr(1)) {
									if (!isdigit(c)) {
										digit = 0;
									}
								}
								if (digit) {
									disp = TA;
								}
							} else {
								n = 1;
								i = 1;
							}

							int ni = (n << 1) | i;
							int xbpe = (x << 3) | (b << 2) | (p << 1) | e;
							int opcode_with_ni = (opcode & 0xFC ) | ni;

							ss << setfill('0') << setw(2) << right << hex << uppercase << opcode_with_ni;
							ss << setw(1) << right << hex << uppercase << xbpe;

							if (e) {
								ss << setfill('0') << setw(5) << (disp & 0xFFFFF);
							} else {
								ss << setfill('0') << setw(3) << (disp & 0xFFF);
							}

							code.opcode_f = ss.str();
						}
						break;
					}
				}

			} else {

				if (format == 1) {
                    ss << setfill('0') << setw(2) << hex << uppercase << opcode;
                } else if (format == 2) {
                    ss << setfill('0') << setw(2) << hex << uppercase << opcode << "00";
                } else if (format == 3) {
					int ni = 11;
					int opcode_with_ni = (opcode & 0xFC ) | ni;
                    ss << setfill('0') << setw(2) << hex << uppercase << opcode_with_ni << "0000";
                } else if (format == 4) {
					ss << setfill('0') << setw(2) << hex << uppercase << opcode << "000000";
				}
                code.opcode_f = ss.str();
			}

		}else{

			if(code.instruction == "BYTE"){
				if(code.operand[0] == 'X'){
					code.opcode_f = code.operand.substr(2, code.operand.length()-3);
				}else if(code.operand[0] == 'C'){
					for(int i=2; i< code.operand.length()-1; i++){
						ss << hex << uppercase << static_cast<int>(code.operand[i]);
					}
					code.opcode_f = ss.str();
				}

			}else if(code.instruction == "WORD"){
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



//translate into record form
bool record(string filename_O){
	ofstream outputFile(filename_O);
	if(!outputFile){
		cerr << "Failed to open record file for writing" << endl;
		return 0;
	}

	int size = 0;
	int preLocate = symnbolList[0].location;
	stringstream total_size;
	vector<string> temp;

	
	int start = codeList.front().location;
	int end = codeList.back().location;
	int Size = end - start;
	//head record
	outputFile << "H^" << setw(6) << left << symnbolList[0].flag  
			   << "^" << setfill('0') << setw(6) << hex << uppercase << right << start 
			   << "^" << setfill('0') << setw(6) << right << Size << endl;
	
	//text record
	for(size_t i=0; i<codeList.size(); ++i){
		const auto& code = codeList[i];

		if(!code.opcode_f.empty()){
			temp.push_back(code.opcode_f);
			size += code.opcode_f.length() / 2;
		}
		int next_size = codeList[i+1].opcode_f.length() /2;

		if((size + next_size)>0x1E || codeList[i+1].location-code.location >= 0x1000 ){
			total_size.str("");
			total_size << setfill('0') << setw(2) << hex << uppercase << size;
			outputFile <<  "T^" << setfill('0') << setw(6) << right << preLocate << "^" << total_size.str();
			
			for(const auto& opcode : temp){
				outputFile << "^" << opcode;
			}
			outputFile << endl;
	
			
			temp.clear();
			size = 0;
			preLocate = codeList[i+1].location;
		}
	}

	if(!temp.empty()){
		total_size.str("");
		total_size << setfill('0') << setw(2) << hex << uppercase << size;
		outputFile << "T^" << setfill('0') << setw(6) << hex << uppercase << right << preLocate << "^" << total_size.str();
		for(const auto& opcode : temp){
			outputFile << "^" << opcode;
		}
		outputFile << endl;
	}

	//modification record
	for (const auto& code : codeList) {
		if (code.instruction[0] == '+') {
			if (code.operand[0] != '#') {
				outputFile << "M^" << setfill('0') << setw(6) << hex << uppercase << right << (code.location + 1) << "^05" << endl;
			}
		}
	}
	
	//end record
	outputFile << "E^" << setfill('0') << setw(6) << hex << uppercase << right << symnbolList.front().location << endl;

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
	}else if(type == "SIC/XE"){
		cout << "Object code generated successfully? " << object_code_XE() << endl;
	}else{
		int t = 0;
		for(const auto& code : codeList){
			if(code.instruction == "BASE"){
				t = 1;
				break;
			}
		}
		if(t==0){
			cout << "Object code generated successfully? " << object_code_SIC() << endl;
		}else{
			cout << "Object code generated successfully? " << object_code_XE() << endl;
		}	
	}

	cout << "Output file successfully update? " << write_inst("output.txt") << endl;
	cout << "Record created successfully? " << record("record.txt") << endl;

	return 0;
} 
