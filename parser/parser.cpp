#include "parser.h"

// ssh
vector<string> parse_address(string s){
    vector<string> parsed_addr;
    int user_end = 0;
    int host_end;
    std::string user, host;
    string port = "22";
    // find where the user and host ends (user@host:port)
    for(int i = 0; i < s.length(); i++){
        if(s.at(i) == '@'){
            user_end = i;
        }
        else if(s.at(i) == ':'){
            host_end = i;
        }
    }
    if(user_end != 0){
        user = s.substr(0, user_end);
        host = s.substr(user_end + 1, host_end - user_end - 1);
    }
    else{
        host = s.substr(0, host_end);
    }

    if(s.length() > host_end + 1){      // assign port if one is declared (else 22)
        port = s.substr(host_end + 1);
    }

    parsed_addr.push_back(user);
    parsed_addr.push_back(host);
    parsed_addr.push_back(port);

    return parsed_addr;
}
