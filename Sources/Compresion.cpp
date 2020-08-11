#include <iostream>
#include <string>
#include <queue>
#include <unordered_map>
using namespace std;

// A Tree node
struct Node
{
    char ch;
    int freq;
    Node *left, *right;
};

// Function to allocate a new tree node
Node* getNode(char ch, int freq, Node* left, Node* right){
    Node* node = new Node();

    node->ch = ch;
    node->freq = freq;
    node->left = left;
    node->right = right;

    return node;
}

// Comparison object to be used to order the heap
struct comp{
    bool operator()(Node* l, Node* r){
        // highest priority item has lowest frequency
        return l->freq > r->freq;
    }
};

// traverse the DataCompresion Tree and decode the encoded string
void decode(Node* root, int &index, string str){
    if (root == nullptr) {
        return;
    }
    // found a leaf node
    if (!root->left && !root->right){
        cout << root->ch;
        return;
    }

    index++;

    if (str[index] =='0') {
        decode(root->left, index, str);
    } else {
        decode(root->right, index, str);
    }
}

string compressedData(unordered_map<char, string> huffmanCode, string text){
    cout << "\nOriginal data: " << text << '\n' << '\n';

    cout << "Data compresion codes are: " << '\n';
    for (auto pair: huffmanCode) {
        cout << pair.first << " " << pair.second << '\n';
    }

    string str = "";
    for (char ch: text) {
        str += huffmanCode[ch];
    }

    cout << "\nEncoded string is :\n" << str << '\n';

    return str;
}

// traverse the DataCompresion Tree and store DataCompresion Codes
// in a map.
unordered_map<char, string> encode(Node* root, string str, unordered_map<char, string> &huffmanCode){
    if (root == nullptr) {
        return huffmanCode;
    }
    // found a leaf node
    if (!root->left && !root->right) {
        huffmanCode[root->ch] = str;
    }

    encode(root->left, str + "0", huffmanCode);
    encode(root->right, str + "1", huffmanCode);

    return huffmanCode;
}

// Builds DataCompresion Tree and decode given input text
Node* buildHuffmanTree(string text){
    // count frequency of appearance of each character
    // and store it in a map
    unordered_map<char, int> freq;
    for (char ch: text) {
        freq[ch]++;
    }

    // Create a priority queue to store live nodes of
    // DataCompresion tree;
    priority_queue<Node*, vector<Node*>, comp> pq;

    // Create a leaf node for each character and add it
    // to the priority queue.
    for (auto pair: freq) {
        pq.push(getNode(pair.first, pair.second, nullptr, nullptr));
    }

    // do till there is more than one node in the queue
    while (pq.size() != 1){
        // Remove the two nodes of highest priority
        // (lowest frequency) from the queue
        Node *left = pq.top(); pq.pop();
        Node *right = pq.top();	pq.pop();

        // Create a new internal node with these two nodes
        // as children and with frequency equal to the sum
        // of the two nodes' frequencies. Add the new node
        // to the priority queue.
        int sum = left->freq + right->freq;
        pq.push(getNode('\0', sum, left, right));
    }

    // root stores pointer to root of DataCompresion Tree
    Node* root = pq.top();

    // traverse the DataCompresion Tree and store DataCompresion Codes
    // in a map. Also prints them
    unordered_map<char, string> huffmanCode;
    encode(root, "", huffmanCode);

    return root;
}

/*
int main(){
    //TEXTO A COMPRIMIR
    string text = "abfabcaecedba";

    //ÁRBOL
    Node* HuffmanTree = buildHuffmanTree(text);

    //TABLA DEL ÁRBOL
    unordered_map<char, string> huffmanCode;
    unordered_map<char, string> HuffmanTable = encode(HuffmanTree, "", huffmanCode);

    //COMPRIMIDO
    string compressedDataCode = compressedData(HuffmanTable,text);

    return 0;
}
*/