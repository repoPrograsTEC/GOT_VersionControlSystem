#include <iostream>
#include <cstdlib>
#include <sstream>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Exception.hpp>
#include <curlpp/Infos.hpp>
#include <fstream>
#include "json.hpp"
#include <filesystem>
#include <experimental/filesystem>
#include "sha256.h"
#include <unordered_map>
#include <queue>

using json = nlohmann::json;

using namespace std;

/*
 *
 *
 * Huffmann
 *
 *
 *
 */

///////// CHECKSUM /////////
const unsigned int SHA256::sha256_k[64] = //UL = uint32
        {0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
         0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
         0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
         0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
         0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
         0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
         0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
         0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
         0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
         0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
         0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
         0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
         0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
         0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
         0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
         0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

void SHA256::transform(const unsigned char *message, unsigned int block_nb)
{
    uint32 w[64];
    uint32 wv[8];
    uint32 t1, t2;
    const unsigned char *sub_block;
    int i;
    int j;
    for (i = 0; i < (int) block_nb; i++) {
        sub_block = message + (i << 6);
        for (j = 0; j < 16; j++) {
            SHA2_PACK32(&sub_block[j << 2], &w[j]);
        }
        for (j = 16; j < 64; j++) {
            w[j] =  SHA256_F4(w[j -  2]) + w[j -  7] + SHA256_F3(w[j - 15]) + w[j - 16];
        }
        for (j = 0; j < 8; j++) {
            wv[j] = m_h[j];
        }
        for (j = 0; j < 64; j++) {
            t1 = wv[7] + SHA256_F2(wv[4]) + SHA2_CH(wv[4], wv[5], wv[6])
                 + sha256_k[j] + w[j];
            t2 = SHA256_F1(wv[0]) + SHA2_MAJ(wv[0], wv[1], wv[2]);
            wv[7] = wv[6];
            wv[6] = wv[5];
            wv[5] = wv[4];
            wv[4] = wv[3] + t1;
            wv[3] = wv[2];
            wv[2] = wv[1];
            wv[1] = wv[0];
            wv[0] = t1 + t2;
        }
        for (j = 0; j < 8; j++) {
            m_h[j] += wv[j];
        }
    }
}

void SHA256::init()
{
    m_h[0] = 0x6a09e667;
    m_h[1] = 0xbb67ae85;
    m_h[2] = 0x3c6ef372;
    m_h[3] = 0xa54ff53a;
    m_h[4] = 0x510e527f;
    m_h[5] = 0x9b05688c;
    m_h[6] = 0x1f83d9ab;
    m_h[7] = 0x5be0cd19;
    m_len = 0;
    m_tot_len = 0;
}

void SHA256::update(const unsigned char *message, unsigned int len)
{
    unsigned int block_nb;
    unsigned int new_len, rem_len, tmp_len;
    const unsigned char *shifted_message;
    tmp_len = SHA224_256_BLOCK_SIZE - m_len;
    rem_len = len < tmp_len ? len : tmp_len;
    memcpy(&m_block[m_len], message, rem_len);
    if (m_len + len < SHA224_256_BLOCK_SIZE) {
        m_len += len;
        return;
    }
    new_len = len - rem_len;
    block_nb = new_len / SHA224_256_BLOCK_SIZE;
    shifted_message = message + rem_len;
    transform(m_block, 1);
    transform(shifted_message, block_nb);
    rem_len = new_len % SHA224_256_BLOCK_SIZE;
    memcpy(m_block, &shifted_message[block_nb << 6], rem_len);
    m_len = rem_len;
    m_tot_len += (block_nb + 1) << 6;
}

void SHA256::final(unsigned char *digest)
{
    unsigned int block_nb;
    unsigned int pm_len;
    unsigned int len_b;
    int i;
    block_nb = (1 + ((SHA224_256_BLOCK_SIZE - 9)
                     < (m_len % SHA224_256_BLOCK_SIZE)));
    len_b = (m_tot_len + m_len) << 3;
    pm_len = block_nb << 6;
    memset(m_block + m_len, 0, pm_len - m_len);
    m_block[m_len] = 0x80;
    SHA2_UNPACK32(len_b, m_block + pm_len - 4);
    transform(m_block, block_nb);
    for (i = 0 ; i < 8; i++) {
        SHA2_UNPACK32(m_h[i], &digest[i << 2]);
    }
}

std::string sha256(std::string input)
{
    unsigned char digest[SHA256::DIGEST_SIZE];
    memset(digest,0,SHA256::DIGEST_SIZE);

    SHA256 ctx = SHA256();
    ctx.init();
    ctx.update( (unsigned char*)input.c_str(), input.length());
    ctx.final(digest);

    char buf[2*SHA256::DIGEST_SIZE+1];
    buf[2*SHA256::DIGEST_SIZE] = 0;
    for (int i = 0; i < SHA256::DIGEST_SIZE; i++)
        sprintf(buf+i*2, "%02x", digest[i]);
    return std::string(buf);
}

struct Node
{
    char ch;
    int freq;
    Node *left, *right;
};

Node* getNode(char ch, int freq, Node* left, Node* right){
    Node* node = new Node();

    node->ch = ch;
    node->freq = freq;
    node->left = left;
    node->right = right;

    return node;
}

struct comp{
    bool operator()(Node* l, Node* r){
        // highest priority item has lowest frequency
        return l->freq > r->freq;
    }
};

void decode(Node* root, int &index, string str){
    if (root == nullptr) {
        return;
    }
    if (!root->left && !root->right){
        return;
    }

    index++;

    if (str[index] =='0') {
        decode(root->left, index, str);
    } else {
        decode(root->right, index, str);
    }
}

string decompressedData(unordered_map<char, string> huffmanCode, string text){

    string valor;
    string textoInicial;

    for(int i = 0; i < text.length() ; i++){

        valor += text[i];

        for (auto pair: huffmanCode) {
            if(valor == pair.second){
                textoInicial += pair.first;
                valor = "";
                break;
            }
        }
    }


    return textoInicial;
}

Node* crearArbolDeHuffman(string text){
    unordered_map<char, int> freq;
    for (char ch: text) {
        freq[ch]++;
    }

    priority_queue<Node*, vector<Node*>, comp> pq;

    for (auto pair: freq) {
        pq.push(getNode(pair.first, pair.second, nullptr, nullptr));
    }

    while (pq.size() != 1){

        Node *left = pq.top(); pq.pop();
        Node *right = pq.top();	pq.pop();

        int sum = left->freq + right->freq;
        pq.push(getNode('\0', sum, left, right));
    }

    Node* root = pq.top();

    return root;
}

unordered_map<char, string> crearTabla(Node* root, string str, unordered_map<char, string> &huffmanCode){
    if (root == nullptr) {
        return huffmanCode;
    }
    // found a leaf node
    if (!root->left && !root->right) {
        huffmanCode[root->ch] = str;
    }

    crearTabla(root->left, str + "0", huffmanCode);
    crearTabla(root->right, str + "1", huffmanCode);

    return huffmanCode;
}

string comprimirTabla(unordered_map<char, string> huffmanCode, string text){
    string str = "";
    for (char ch: text) {
        str += huffmanCode[ch];
    }
    return str;
}

string descomprimirTabla(unordered_map<string, string> huffmanCode, string text){

    string valor;
    string textoInicial;

    for(int i = 0; i < text.length() ; i++){

        valor += text[i];

        for (auto pair: huffmanCode) {
            if(valor == pair.second){
                textoInicial += pair.first;
                valor = "";
                break;
            }
        }
    }

    return textoInicial;
}

json tablaHaciaJson(unordered_map<char, string> huffmanTable){
    json j;
    string s;

    for (auto pair: huffmanTable) {
        s.push_back(pair.first);
        j[s] = pair.second;
        s = "";
    }

    return j;
}

unordered_map<string, string> jsonHaciaTabla(json j){
    unordered_map<string, string> huffmanTable;

    std::vector<string> myvectorKey;
    std::vector<string> myvectorValue;

    for (json::iterator it = j.begin(); it != j.end(); ++it) {
        myvectorKey.push_back(it.key());
        myvectorValue.push_back(it.value());
    }

    for (int i = 0; i < j.size(); i++) {
        huffmanTable[myvectorKey.at(i)] = myvectorValue.at(i);
    }

    return huffmanTable;
}

/*
 *
 *
 * Huffmann
 *
 *
 *
 */




/*
 *
 *
 * API
 *
 *
 */

std::string getText(const std::string& url){

    std::ifstream file;
    file.open (url);

    std::string text;
    std::string temp;

    while (std::getline(file, temp)){
        text += temp;
    }

    file.close();

    return text;

}

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

    std::ifstream add("add.json");
    json ad;
    ad << add;
    json temp;
    temp["name"] = namefile;
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

                    Node* HuffmanTree = crearArbolDeHuffman(getText(it["name"]));
                    //TABLA DEL ÁRBOL
                    unordered_map<char, string> huffmanCode;
                    unordered_map<char, string> huffmanTable = crearTabla(HuffmanTree, "", huffmanCode);
                    //COMPRIMIDO
                    string textoComprimido = comprimirTabla(huffmanTable, getText(it["name"]));
                    //tabla a json
                    json t = tablaHaciaJson(huffmanTable);

                    jl["contenido"] = t.dump();
                    jl["comprimido"] = textoComprimido;

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

                Node* HuffmanTree = crearArbolDeHuffman(getText(it["name"]));
                //TABLA DEL ÁRBOL
                unordered_map<char, string> huffmanCode;
                unordered_map<char, string> huffmanTable = crearTabla(HuffmanTree, "", huffmanCode);
                //COMPRIMIDO
                string textoComprimido = comprimirTabla(huffmanTable, getText(it["name"]));
                //tabla a json
                json t = tablaHaciaJson(huffmanTable);

                jk["contenido"] = t.dump();
                jk["comprimido"] = textoComprimido;

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

void rollback (const string& commit, const string& file){

    std::ifstream repo("repositorio.json");
    json r;
    r << repo;
    repo.close();

    string get = "http://127.0.0.1:5000/" + (string)r["nameRepo"] + "/" + file + "/" + commit;

    json commite = get_request(get);
    int id = commite[0]["id"];

    json rest = get_request("http://127.0.0.1:5000/" + (string)r["nameRepo"] + "/" + to_string(id) + "/getData")[0];

    std::ofstream outfile (rest["archivo"]);

    string sTabla = rest["contenido"];

    json tabla = json::parse(sTabla);

    unordered_map<string, string> mapaDesdeJson = jsonHaciaTabla(tabla);

    string texto = descomprimirTabla(mapaDesdeJson, rest["comprimido"]);

    outfile << texto;

    outfile.close();
}

json getID(int id){

    std::ifstream repo("repositorio.json");
    json r;
    r << repo;
    repo.close();

    return get_request("http://127.0.0.1:5000/" + (string)r["nameRepo"] + "/" + to_string(id));

}

void reset(const string& file){

    std::ifstream repo("repositorio.json");
    json r;
    r << repo;
    repo.close();

    json lastCommitVersionFile = getID(getLastCommit(file)[0]["MAX(id)"]);

    int id = lastCommitVersionFile[0]["id"];

    json rest = get_request("http://127.0.0.1:5000/" + (string)r["nameRepo"] + "/" + to_string(id) + "/getData")[0];

    std::ofstream outfile (rest["archivo"]);

    string sTabla = rest["contenido"];

    json tabla = json::parse(sTabla);

    unordered_map<string, string> mapaDesdeJson = jsonHaciaTabla(tabla);

    string texto = descomprimirTabla(mapaDesdeJson, rest["comprimido"]);

    outfile << texto;

    outfile.close();

}

void status(){

    std::ifstream repo("repositorio.json");
    json r;
    r << repo;
    repo.close();

    json lastID = getID(get_request("http://127.0.0.1:5000/" + (string)r["nameRepo"] + "/lastID")[0]["MAX(id)"]);
    string commit = lastID[0]["commit"];

    cout << get_request("http://127.0.0.1:5000/" + (string)r["nameRepo"] + "/" + commit + "/status") << "\n";


}

void status(string file){

    std::ifstream repo("repositorio.json");
    json r;
    r << repo;
    repo.close();

    cout << get_request("http://127.0.0.1:5000/" + (string)r["nameRepo"] + "/" + file + "/statusFile") << "\n";

}

void sync (const string& file){

    std::ifstream repo("repositorio.json");
    json r;
    r << repo;
    repo.close();

    string id = to_string(getLastCommit(file)[0]["MAX(id)"]);

    json rest = get_request("http://127.0.0.1:5000/" + (string)r["nameRepo"] + "/" + id + "/getData")[0];

    std::ofstream outfile (rest["archivo"]);

    string sTabla = rest["contenido"];

    json tabla = json::parse(sTabla);

    unordered_map<string, string> mapaDesdeJson = jsonHaciaTabla(tabla);

    string texto = descomprimirTabla(mapaDesdeJson, rest["comprimido"]);

    outfile << texto;

    outfile.close();
}

/*
 *
 *
 * API
 *
 *
 */









int main(int argc, char *argv[])
{

    // Array de comandos
    string comandos[6] = { "init", "add", "commit", "status", "reset", "sync"};

    if (argc == 1){
        cout << "Introduzca un comando válido #1 \n\n"
                "Si desea consultar la ayuda sobre los comandos ingrese ./got help\n" << endl;
    }

    if (argc == 2){
        string comando = argv[1];
        if (comando == "help"){
            cout << "\n***    Información sobre el uso de los comandos de Got!   ***" << endl;
            const string str = R"(
* got init <name>:
        Instancia un nuevo repositorio en el servidor y lo identifica con el nombre indicado por <name>.

* got add [-A] [name]:
        Permite agregar todos los archivos que no estén registrados o que tengan nuevos cambios al repositorio.
        Ignora los archivos que estén configurados en .gotignore.
        El usuario puede indicar cada archivo por agregar, o puede usar el flag -A para agregar todos los archivos relevantes.

* got commit <mensaje>:
        Envía los archivos agregados y pendientes de commit al server.
        Se debe especificar un mensaje a la hora de hacer el commit.

* got status <file>:
        Este comando nos va a mostrar cuales archivos han sido cambiados, agregados o eliminados de acuerdo al commit anterior.

* got rollback <file> <commit>
        Permite regresar un archivo en el tiempo a un commit específico.

* got reset <file>
        Deshace cambios locales para un archivo y lo regresa al último commit.

* got sync <file>
        Recupera los cambios para un archivo en el server y lo sincroniza con el archivo en el cliente.
        Si hay cambios locales, debe permitirle de alguna forma, que el usuario haga merge de los cambios interactivamente.
)";

            cout << str << '\n' ;
        }

        if (comando == "status") {
            cout << "\n***    Estado del repositorio al último commit!   ***\n" << endl;
            //INSERTAR AQUÍ COMANDOS DEL MÉTODO status
            status();

        }

        if (comando != "help" && comando != "status"){
            cout << "Introduzca un comando válido #2 \n\n"
                    "Si desea consultar la ayuda sobre los comandos ingrese ./got help\n" << endl;
        }
    }

    if (argc == 3) {
        string comandoIngresado = argv[1];
        int indiceComando = -1;

        for (int i = 0; i < 6; i++){
            if (comandoIngresado == comandos[i]) {
                indiceComando = i;
                break;
            }
        }


        switch (indiceComando) {
            //MÉTODO INIT
            case 0:
                cout << "\n***    Inicializando repositorio con el nombre: " + (string)argv[2] + "   ***" << endl;
                // INGRESAR AQUÍ COMANDOS DE MÉTODO init
                init((string)argv[2]);

                break;

                //MÉTODO ADD
            case 1:
                cout << "\n***    Añadiendo archivo: " + (string)argv[2] + "   ***" << endl;
                // INGRESAR AQUÍ COMANDOS DE MÉTODO add
                add((string)argv[2]);

                break;

                //MÉTODO COMMIT
            case 2:
                cout << "\n***    Realizando commit: " + (string)argv[2] + "   ***" << endl;
                // INGRESAR AQUÍ COMANDOS DE MÉTODO commit
                commit((string)argv[2]);

                break;

                //MÉTODO STATUS
            case 3:
                cout << "\n***    Estado al último commit del archivo: " + (string)argv[2] + "   ***" << endl;
                // INGRESAR AQUÍ COMANDOS DE MÉTODO status
                status((string)argv[2]);

                break;

                //MÉTODO RESET
            case 4:
                cout << "\n***    Regresando el archivo: " + (string)argv[2] + " al último commit   ***" << endl;
                // INGRESAR AQUÍ COMANDOS DE MÉTODO reset
                reset((string)argv[2]);

                break;

                //MÉTODO SYNC
            case 5:
                cout << "\n***    Sincronizando el archivo: " + (string)argv[2] + " con el servidor   ***" << endl;
                // INGRESAR AQUÍ COMANDOS DE MÉTODO sync
                sync((string)argv[2]);

                break;

            default:
                cout << "Introduzca un comando válido #3 \n\n"
                        "Si desea consultar la ayuda sobre los comandos ingrese ./got help\n" << endl;
        }
    }

    if(argc == 4){
        string comando = argv[1];
        if (comando == "rollback") {
            cout << "\n***    Regresando el archivo " + (string)argv[2] + " al commit " + (string)argv[3] + "   ***\n" << endl;
            //INSERTAR AQUÍ COMANDOS DEL MÉTODO rollback
            rollback((string)argv[3], (string)argv[2]);

        } if (comando == "add" && (string)argv[2] == "[-A]"){
            cout << "\n***    Añadiendo archivos!   ***" << endl;
            // INGRESAR AQUÍ COMANDOS DE MÉTODO add [-A]

        }if (comando != "rollback" && comando != "add"){
            cout << "Introduzca un comando válido #4 \n\n"
                    "Si desea consultar la ayuda sobre los comandos ingrese ./got help\n" << endl;
        }
    }


    return 0;
}