#pragma once

#include <library/common/codec.h>
#include <bitset>
#include <queue>

namespace Codecs {

    class huffmancodec : public CodecIFace {
    public:
        struct node {
            size_t left, right, pos;
            unsigned long long frequency;
            unsigned char value;

            bool operator<(const node &other) const {
                return frequency < other.frequency;
            }

            bool operator>(const node &other) const {
                return frequency > other.frequency;
            }
        };

    private:
        vector<node> tree;
        vector<std::vector<bool>> codes;

        void DFS(const std::vector<node> &tree, vector<std::vector<bool>> &out, std::vector<bool> code, node pos) {
            if (!pos.left && !pos.right) {
                out[pos.value] = code;
                std::cout << pos.value << '\n';
                for (bool bit : code) {
                    std::cout << bit;
                }
                std::cout << "\n\n";
            } else {
                std::vector<bool> new_code = code;
                new_code.push_back(false);
                DFS(tree, out, new_code, tree[pos.left]);
                *(new_code.end() - 1) = true;
                DFS(tree, out, new_code, tree[pos.right]);
            }
        }

    public:
        class BitStr {
        private:
            std::string s;
            unsigned bits;
        public:
            BitStr() {
                bits = 0;
            }

            BitStr(const std::string &str) {
                s = str;
                bits = 0;
            }

            BitStr(const BitStr &x) {
                s = x.s;
                bits = x.bits;
            }

            void push_back(bool bit) {
                if (bits) {
                    --bits;
                    unsigned char last = static_cast<unsigned char>(*(s.end() - 1));
                    unsigned char mask = (bit ? 1 : 0) << bits;
                    *(s.end() - 1) = last | mask;
                } else {
                    bits = 8;
                    s.push_back(0);
                    push_back(bit);
                }
            }

            void push_back(unsigned char symbol) {
                if (bits) {
                    *(s.end() - 1) |= symbol >> bits;
                    s.push_back(symbol << (8 - bits));
                } else {
                    s.push_back(symbol);
                }
            }

            void push_back(const std::vector<bool> &code) {
                for (bool bit : code) {
                    push_back(bit);
                }
            }

            std::string getvalue() const {
                return s;
            }
        };

        void encode(string &encoded, const string &raw) const {
            BitStr out;
            for (char c : raw) {
                out.push_back(codes[static_cast<unsigned char>(c)]);
                //std::cout << codes[static_cast<unsigned char>(c)][0] << '\n';
            }
            encoded = out.getvalue();
        }

        void decode(string &raw, const string &encoded) const {
            node position = tree[0];
            std::bitset<8> code;
            for (unsigned char c : encoded) {
                code = c;
                for (unsigned i = 0; i < 8; ++i) {
                    if (code[i]) {
                        position = tree[position.right];
                    } else {
                        position = tree[position.left];
                    }
                    if (!position.left && !position.right) {
                        raw.push_back(position.value);
                        position = tree[0];
                    }
                }
            }
        }

        string save() const {
            return string();
        }

        void load(const string&) {}

        size_t sample_size(size_t) const {
            return 10000;
        };

        void learn(const std::vector<std::string> &samplevector) {
            tree.resize(257);
            for (size_t i = 1; i <= 256; ++i) {
                node a;
                a.pos = i;
                a.left = 0;
                a.right = 0;
                a.frequency = 0;
                a.value = i - 1;
                tree[i] = a;
            }

            for (auto It = samplevector.begin(); It != samplevector.end(); ++It) {
                for (auto It_s = It -> begin(); It_s != It -> end(); ++It_s) {
                    unsigned char letter = static_cast<unsigned char>(*It_s);
                    tree[letter + 1].frequency += 1;
                }
            }

            std::priority_queue<node, std::vector<node>, std::greater<node>> q(tree.begin() + 1, tree.end());

            tree.reserve(512);
            while (q.size() > 1) {
                auto first_node = q.top();
                q.pop();
                auto second_node = q.top();
                q.pop();

                node new_node;
                new_node.left = first_node.pos;
                new_node.right = second_node.pos;
                new_node.frequency = first_node.frequency + second_node.frequency;
                std::cout << new_node.frequency << ' ' << new_node.left << ' ' << new_node.right << '\n';
                tree.push_back(new_node);
            }
            tree[0] = q.top();
            std::cout << tree[0].left << ' ' << tree[0].right << ' ' << tree[0].value << '\n';
            codes = std::vector<std::vector<bool>>(256, std::vector<bool>());
            DFS(tree, codes, std::vector<bool>(), tree[0]);
            for (std::vector<bool> code : codes) {
                if (code.size()) {
                    for (auto bit : code) {
                        std::cout << bit;
                    }
                    std::cout << '\n';
                }
            }
        }

        void reset() {}
    };

}
