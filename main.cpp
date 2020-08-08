#include <iostream>

#include <cstdlib>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Exception.hpp>
#include <curlpp/Infos.hpp>
#include <fstream>
#include <sstream>
#include "json.hpp"
#include <filesystem>

using json = nlohmann::json;

using namespace std;

string post_request(const string& url ,const string& path2file)
{
    const string field_divider="&";
    stringstream result;
    try
    {
        // This block responsible for reading in the fastest way media file
        //      and prepare it for sending on API server
        ifstream is(path2file);
        is.seekg(0, ios_base::end);
        size_t size=is.tellg();
        is.seekg(0, ios_base::beg);
        vector<char> v(size/sizeof(char));
        is.read((char*) &v[0], size);
        is.close();
        string body(v.begin(), v.end());

        // Initialization
        curlpp::Cleanup cleaner;
        curlpp::Easy request;
        list< string > headers;
        headers.push_back("Content-Type: application/json");
        headers.push_back("User-Agent: curl/7.77.7");

        using namespace curlpp::Options;

        request.setOpt(new Verbose(true));
        request.setOpt(new HttpHeader(headers));
        request.setOpt(new Url(url));
        request.setOpt(new PostFields(body));
        request.setOpt(new PostFieldSize(body.length()));
        request.setOpt(new curlpp::options::SslEngineDefault());
        request.setOpt(WriteStream(&result));
        request.perform();
    }
    catch ( curlpp::LogicError & e )
    {
        cout << e.what() << endl;
    }
    catch ( curlpp::RuntimeError & e )
    {
        cout << e.what() << endl;
    }

    return (result.str());

}

json get_request(const string& url)
{
    try
    {
        curlpp::Cleanup cleaner;
        curlpp::Easy request;

        json resp;

        using namespace curlpp::Options;

        request.setOpt(Verbose(true));
        request.setOpt(Url(url));

        std::ostringstream json ;
        request.setOpt(new curlpp::options::WriteStream(&json));

        request.perform();

        resp = json::parse(json.str());

        std::string effURL;


        curlpp::infos::EffectiveUrl::get(request, effURL);


        std::cout << "Effective URL: " << effURL << std::endl;

        return resp;
    }

    catch ( curlpp::LogicError & e ) {
        std::cout << e.what() << std::endl;
    }
    catch ( curlpp::RuntimeError & e ) {
        std::cout << e.what() << std::endl;
    }

}

json getLastCommit(string file){

    std::ifstream repo("repositorio.json");
    json r;
    r << repo;
    repo.close();

    return get_request("http://127.0.0.1:5000/" + (string)r["nameRepo"] + "/" + file + "/lastcommit");

}

int verificarArchivo(const string& file){

    std::ifstream repo("repositorio.json");
    json r;
    r << repo;
    repo.close();

    string mensaje = "EXISTS(SELECT 1 FROM " + (string)r["nameRepo"] +"_versiones WHERE archivo = '" + file + "')";

    return (get_request("http://127.0.0.1:5000/" + (string)r["nameRepo"] + "/" + file + "/exist"))[0][mensaje];

}

bool verificarVersion(const string& file){
    int versionGlobal = getLastCommit(file)[0]["MAX(id)"];

    std::ifstream repo("repositorio.json");
    json r;
    r << repo;
    repo.close();

    int versionLocal = 0;
    for (auto &it : r["version"]){
        if (file == it["archivo"]){
            versionLocal = it["version"];
        }
    }
    if((versionLocal != 0) && (versionLocal == versionGlobal)){
        return 1;
    }else{
        return 0;
    }
}

void add (std::string namefile){

    string url = std::filesystem::current_path();
    std::ifstream add("add.json");
    json ad;
    ad << add;
    json temp;
    temp["name"] = namefile;
    temp["url"] = url + "/" + namefile;
    ad["Lista"] += temp;
    add.close();

    std::ofstream add2("add.json");
    add2 << ad;
    add2.close();

}

void init(std::string name){

    std::ofstream outfile (".gotignore");
    outfile << "./gotignore" << std::endl;
    outfile.close();

    std::ofstream repo("repositorio.json");
    json info;
    info["nameRepo"] = name;
    info["version"] = {};
    repo << info;
    repo.close();

    std::ofstream add("add.json");
    json ad;
    ad["Lista"] = {};
    add << ad;
    add.close();


    std::ofstream file ("send.json");
    json j;
    j["name"] = name;
    file << j;
    file.close();
    post_request("http://127.0.0.1:5000/init" ,"send.json");

}

void commit (string commit){

        std::ifstream repo("repositorio.json");
        json r;
        r << repo;
        repo.close();

        std::ifstream add("add.json");
        json j;
        j << add;
        add.close();

        for (auto &it : j["Lista"]) {

            if (verificarArchivo(it["name"]) == 1){

                if(verificarVersion(it["name"])){

                    std::ofstream file("send.json");
                    json jl;
                    jl["name"] = it["name"];
                    jl["nameRepo"] = r["nameRepo"];
                    jl["commit"] = commit;
                    file << jl;
                    file.close();
                    post_request("http://127.0.0.1:5000/add", "send.json");

                    for (auto &i : r["version"]) {
                        if (it["name"] == i["name"]) {
                            i["version"] = getLastCommit(i["name"])[0]["MAX(id)"];
                        }
                    }

                }else{
                    cout << "El archivo " << it["name"] << " no está en su ultimo commit" << endl;
                }

            }
            else{

                std::ofstream file("send.json");
                json jk;
                jk["name"] = it["name"];
                jk["nameRepo"] = r["nameRepo"];
                jk["commit"] = commit;
                file << jk;
                file.close();
                post_request("http://127.0.0.1:5000/add", "send.json");

                r["version"] += {{"name" , it["name"]}, {"version", getLastCommit(it["name"])[0]["MAX(id)"]}};

            }

        }

        std::ofstream addi("add.json");
        json ad;
        ad["Lista"] = {};
        addi << ad;
        add.close();

        std::ofstream repos("repositorio.json");
        repos << r;
        repos.close();


}

json rollback (const string& commit, const string& file){

    std::ifstream repo("repositorio.json");
    json r;
    r << repo;
    repo.close();

    string get = "http://127.0.0.1:5000/" + (string)r["nameRepo"] + "/" + file + "/" + commit;

    return get_request(get);

}

json getID(int id){

    std::ifstream repo("repositorio.json");
    json r;
    r << repo;
    repo.close();

    return get_request("http://127.0.0.1:5000/" + (string)r["nameRepo"] + "/" + to_string(id));

}

void reset(string file){

    json lastCommitVersionFile = getID(getLastCommit(file)[0]["MAX(id)"]);
    cout << lastCommitVersionFile;

}

json status(){

    std::ifstream repo("repositorio.json");
    json r;
    r << repo;
    repo.close();

    json lastID = getID(get_request("http://127.0.0.1:5000/" + (string)r["nameRepo"] + "/lastID")[0]["MAX(id)"]);
    string commit = lastID[0]["commit"];

    return get_request("http://127.0.0.1:5000/" + (string)r["nameRepo"] + "/" + commit + "/status");


}

json sync (const string& file){

    cout << getID(getLastCommit(file)[0]["MAX(id)"]);

}

int main()
{

    sync("pene");

    //"/home/daniel/CLionProjects/prueba/data.json"
    //post_request("http://127.0.0.1:5000/add","/home/daniel/CLionProjects/prueba/data.json");
    //get_request("http://127.0.0.1:5000/emp");
    //init("dani");
    //add("Hola");
    //add("pene");
    //commit("prueba");

    /*
    json respuesta;

    respuesta = get_request("http://127.0.0.1:5000/getVersion/dani");

    auto hola = respuesta["MAX(id)"];
    cout << hola;
*/

    //cout << getCommit("Vagina", "Hola");

    //cout << getLastCommit("Hola");

    //cout << getID(1);

    //reset("pene");



    return 0;
}