#include "Checksum.cpp"
#include "Compresion.cpp"

/*
int main(){
    //TEXTO A COMPRIMIR
    //string text = "abfabcaecedba";

    string file = "/home/esteban/Escritorio/DataCompresion/Sources/prueba.txt";
    string output1 = sha256(file);

    cout << "\nsha256('"<< file << "') :   " << output1 << "\n\n";

    //ÁRBOL
    Node* HuffmanTree = buildHuffmanTree(output1);

    //TABLA DEL ÁRBOL
    unordered_map<char, string> huffmanCode;
    unordered_map<char, string> HuffmanTable = encode(HuffmanTree, "", huffmanCode);

    //COMPRIMIDO
    string compressedDataCode = compressedData(HuffmanTable,file);

    return 0;
}
*/