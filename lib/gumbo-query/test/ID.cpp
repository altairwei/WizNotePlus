/*
 * ID test.
 */

#include <iostream>
#include <fstream>

#include "Util.h"
#include "Document.h"
#include "Node.h"

 using std::string;

int main(int argc, char * argv[])
{
	string page(file_str("test_page.html"));

	CDocument doc;
	doc.parse(page.c_str());

	CSelection c = doc.find("#start-of-content");
	if(c.nodeNum() > 0)
		return 0;
	return 1;
}
