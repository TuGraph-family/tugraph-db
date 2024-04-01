
#include <vector>
#include <iostream>

void dump_p(const std::vector<std::pair<std::string, int>>& oi) {
    for (auto& p : oi) {
	auto cypher = p.first + ";";
        std::string temp;
        for (auto c : cypher) {
            if (c != '\n') {
                temp.push_back(c);
            } else {
                temp.push_back(' ');
            }
        }
        std::cout << temp << std::endl;
    }
}

void dump(const std::vector<std::string>& oi) {
    for (auto& p : oi) {
       std::string temp;
       for (auto c : p) {
           if (c != '\n') {
               temp.push_back(c);
           } else {
               temp.push_back(' ');
           }
       }
       temp += ";";
       std::cout << temp << std::endl;
    }
}

int main() {

    static const std::vector<std::pair<std::string, int>> script_check = {
        {"MATCH ()-[r]->() RETURN euid(r) /* 28 */", 28},
        {"MATCH ()-[r]->() RETURN id(r) /* 28 */", 28},
        // test rest api same support
        {"MATCH (n),(m) WHERE id(n)=0 and id(m)=1 CREATE (n)-[r:ACTED_IN {charactername: "
         "\"testaha\"}]->(m) RETURN r",
         1},
        {"MATCH (n),(m),(k) \n"
         "WHERE id(n)=0 and id(m)=2 and id(k)=3\n"
         "CREATE (n)-[r:ACTED_IN {charactername: \"testaha\"}]->(m),(n)-[q:MARRIED]->(k)\n"
         "RETURN r,q",
         1},
        {"MATCH (n)-[e]->() where id(n)=4 return euid(e)", 2},
        {"MATCH ()-[e]->(n) where id(n)=4 return euid(e)", 1},
        {"MATCH (n)-[e]-() where id(n)=4 return euid(e)", 3},
        {"MATCH ()-[e]->() where euid(e)=\"0_2_0_0_0\" return e,labels(e),properties(e)", 1},
        {"MATCH ()-[e]->() where euid(e)=\"4_17_5_0_0\" return properties(e)", 1},
        {"MATCH ()-[e]->() where euid(e)=\"4_17_5_0_0\" return e.charactername", 1},
        {"MATCH ()-[e]->() where euid(e)=\"8_13_2_0_0\" set e.weight=1223 return "
         "e,labels(e),properties(e)",
         1},
        {"MATCH ()-[e]->() where euid(e)=\"4_17_5_0_0\" delete e", 1},
    };


    dump_p(script_check);
    return 0;
}
