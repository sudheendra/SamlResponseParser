
#include "SamlParser.hpp"

int main(int argc, char* argv[])
{
    string s = "D:\\work\\gitsaml\\saml-tester\\SamlTester\\sample_saml.xml";
	//string s = "D:\\Work\\cpp-xmltooling\\xmltoolingtest\\data\\SimpleXMLObjectWithAttribute.xml";
    SamlParser st(s);
	char *fileContent = NULL;
	int fileLen = 0;
	FILE *fp = fopen(s.c_str(), "rb");
	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		fileLen = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		printf("file len: %d", fileLen);
		fileContent = (char *) malloc (fileLen);
		memset(fileContent, 0, fileLen);
		fread(fileContent, 1, fileLen, fp);
		fclose(fp);
	}

	string data(fileContent);
	if (!data.empty())
	{
		int ret = st.ParseSaml(data);
		if (ret)
		{
			string status = st.getStatusCode();
			if (status.empty())
				cout << "no value for status node" << endl;
			else
				cout << "Status is: " << status << endl;
			string val = "unique-id";
			string id = st.getAttributeValue(val);
			if (id.empty())
				cout << "no value for attr name:  " <<  val.c_str() << endl;
			else
				cout << "unique id is : "<< id.c_str() << endl;
			std::map<string, string> assertionMap;
			ret = st.fillAttributeMap(assertionMap);
		}
		else
			cout << "Failed to parse" << endl;
	}
    return 0;
}