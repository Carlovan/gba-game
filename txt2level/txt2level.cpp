#include <cstdlib>
#include <cstdio>
#include <iostream>

using namespace std;

int main(){
	FILE *in = fopen("input.txt", "r");
	FILE *out = fopen("level.h", "w");

	string output= "";

	int count = 0, tot = 0;
	char tmp;
	while((tmp = getc(in)) != EOF) {
		tot++;
		if(tmp >= '0' && tmp <= '9'){
			output += tmp;
			output += ",\t";
			count++;
		}
		else if(tmp == '\n'){
			output += "},\n\t{";
		}
		else{
			output += "-1,\t";
		}
	}
	output += "}};\n";

	fprintf(out, "int level[][%d] = {\n\t{", tot/10);
	fprintf(out, "%s", output.c_str());

	fprintf(out, "\nconst int blocksCount = %d;\n", count);
	fprintf(out, "\nconst int levWidth = %d;\n", tot/10);

	fclose(in);
	fclose(out);
	return 0;
}