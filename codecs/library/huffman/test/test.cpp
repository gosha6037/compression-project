#include <string>
#include <experimental/string_view>
#include <vector>
#include <iostream>
#include <library/huffman/huffman.h>


int main() {
    std::string test = "zdfxgcjvhbkl;,jkgdzsfdxgchvbnmlbjvhcfgxdfchvjbljjjjkjkjkjkjkjkljkljkgj";
    std::vector<std::string> sample;
    sample.push_back(test);

    Codecs::huffmancodec codec;
    codec.learn(sample);

    std::string encoded;
    codec.encode(encoded, test);
    std::string out;
    codec.decode(out, encoded);
    if (out == test) {
        std::cout << "Correct" << std::endl;
    } else {
        std::cout << "Failed" << std::endl;
    }

    return 0;
}