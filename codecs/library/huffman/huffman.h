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
            } else {
                std::vector<bool> new_code = code;
                new_code.push_back(false);
                DFS(tree, out, new_code, tree[pos.left]);
                *(new_code.end() - 1) = true;
                DFS(tree, out, new_code, tree[pos.right]);
            }
        }

        std::vector<bool> find_wrong_code(const std::vector<node> &tree, node pos, std::vector<bool> code, long long layer) const {
            if (layer < 0 || (!pos.left && !pos.right)) {
                return std::vector<bool>();
            }
            if (!layer) {
                return code;
            } else {
                std::vector<bool> out;
                std::vector<bool> new_code = code;
                new_code.push_back(false);
                out = find_wrong_code(tree, tree[pos.left], new_code, layer - 1);
                if (!out.size()) {
                    *(new_code.end() - 1) = true;
                    out = find_wrong_code(tree, tree[pos.right], new_code, layer - 1);
                }
                return out;
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

            unsigned push_back(const std::vector<bool> &code) {
                for (bool bit : code) {
                    push_back(bit);
                }
                return bits;
            }

            std::string getvalue() const {
                return s;
            }
        };

    private:
        friend class BitStr;

    public:

        void encode(string &encoded, const string &raw) const {
            BitStr out;
            for (size_t i = 0; i < raw.size(); ++i) {
                unsigned bits = out.push_back(codes[static_cast<unsigned char>(raw[i])]);

                if (i + 1 == raw.size() && bits) {
                    std::vector<bool> fake_code = find_wrong_code(tree, tree[0], std::vector<bool>(), bits);
                    out.push_back(fake_code);
                }

            }
            encoded = out.getvalue();
        }

        void decode(string &raw, const string &encoded) const {
            node position = tree[0];
            std::vector<bool> code(8);
            for (char c : encoded) {
                auto symbol = static_cast<unsigned char>(c);
                for (int j = 7; j >= 0; --j) {
                    code[j] = symbol % 2;
                    symbol /= 2;
                }
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
            return std::string();
        }

        void load(const string &) { }

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
                for (auto It_s = It->begin(); It_s != It->end(); ++It_s) {
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
                new_node.pos = tree.size();
                tree.push_back(new_node);
                q.push(new_node);
            }
            tree[0] = q.top();
            codes = std::vector<std::vector<bool>>(256, std::vector<bool>());
            DFS(tree, codes, std::vector<bool>(), tree[0]);
        }

        void reset() {}
    };
}
