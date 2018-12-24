
#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>

using namespace std;

string url_encode(const string &value) {
    ostringstream escaped;
    escaped.fill('0');
    escaped << hex;

    for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
//        fprintf(stderr,"url_encode  \n");fflush(stderr);

//        string::value_type cc = (*i);
        unsigned char c = (unsigned char)(*i);


        // Keep alphanumeric and other accepted characters intact
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
//            fprintf(stderr,"url_encode 1.0  \n");fflush(stderr);
            escaped << c;
//            fprintf(stderr,"url_encode 1.1  \n");fflush(stderr);
            continue;
        }
//        fprintf(stderr,"url_encode 2  \n");fflush(stderr);

        // Any other characters are percent-encoded
        escaped << uppercase;
        escaped << '%' << setw(2) << int((unsigned char) c);
        escaped << nouppercase;
    }

    return escaped.str();
}

string urlDecode(string &SRC) {
    string ret;
    char ch;
    int i, ii;
    for (i=0; i<SRC.length(); i++) {
        if (int(SRC[i])==37) {
            sscanf(SRC.substr(i+1,2).c_str(), "%x", &ii);
            ch=static_cast<char>(ii);
            ret+=ch;
            i=i+2;
        } else {
            ret+=SRC[i];
        }
    }
    return (ret);
}
