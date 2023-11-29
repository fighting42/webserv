#include "../includes/libft.hpp"

vector<std::string> split(string input, char delimiter) {
    vector<std::string> ret;
    stringstream ss(input);
    string temp;
 
    while (getline(ss, temp, delimiter)) {
        ret.push_back(temp);
    }
 
    return ret;
}
