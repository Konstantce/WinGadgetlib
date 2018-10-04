#include "merkle_tree.hpp"
#include "sha256.hpp"

#include <math.h>       /* log */
#include <algorithm>


using namespace merkle_tree;

MerkleTree::MerkleTree(const std::vector<uint32_t>& leaves_) : leaves(leaves_)
{
	height = static_cast<uint32_t>(log(leaves.size()) / log(2));
	calculate_tree();
}

std::vector<unsigned char> arr2vec(const std::string& ptr, unsigned size)
{
	std::vector<unsigned char> result;
	result.assign(ptr.data(), ptr.data() + size);
	return result;
}

std::string hexlify(uint32_t num)
{
	std::string result = "xxxx";
	result[0] = (char)(num >> 24);
	result[1] = (char)(num >> 16);
	result[2] = (char)(num >> 8);
	result[3] = (char)(num);
	return result;
}

std::string hexlify(std::string hash)
{
	auto convert_ch = [](char c) -> char
	{
		if (c >= '0' && c <= '9')
			return (c - '0');
		if (c >= 'a' && c <= 'f')
			return (c - 'a' + 10);

	};
	
	std::string result(hash.size() / 2, 'x');
	assert(hash.size() % 2 == 0);
	auto j = 0;
	for (size_t i = 0; i < hash.size() / 2; i ++)
	{
		result[i] = convert_ch(hash[j++]) * 16;
		result[i] += convert_ch(hash[j++]);
	}
	return result;
}

std::vector<gadget> MerkleTree::get_proof(uint32_t address)
{
	std::vector<gadget> result;
	MerkleBranch* temp_branch = root;
	uint32_t num = address;
	for (unsigned i = 0; i < height; i++)
	{
		bool flag = (num % 2);
		num /= 2;
		if (flag)
		{
			gadget g(arr2vec(temp_branch->left_->hash_, HASH_DIGEST_SIZE), 
				HASH_DIGEST_SIZE * 8,
				false);
			result.emplace_back(g);
			temp_branch = temp_branch->right_;
		}
		else
		{
			gadget g(arr2vec(temp_branch->right_->hash_, HASH_DIGEST_SIZE), 
				HASH_DIGEST_SIZE * 8,
				false);
			result.emplace_back(g);
			temp_branch = temp_branch->left_;
		}
	}
	std::reverse(result.begin(), result.end());
	return result;
}

void MerkleTree::update(uint32_t from, uint32_t to, uint32_t amount)
{
	leaves[from] -= amount;
	leaves[to] += amount;
	calculate_tree();
}

gadget MerkleTree::get_root()
{
	return gadget(arr2vec(root->hash_, HASH_DIGEST_SIZE), HASH_DIGEST_SIZE * 8, true);
}

void MerkleTree::calculate_tree()
{
	//TODO: 
	std::vector<MerkleBranch*> branches;
	unsigned index = 0;
	std::string hex_digest;
	for (auto& leaf : leaves)
	{				
		picosha2::hash256_hex_string(hexlify(leaf), hex_digest); 
		branches.emplace_back(new MerkleBranch{ hexlify(hex_digest), nullptr, nullptr });
	}
	unsigned leaf_choice = 0;
	while (leaf_choice + 1 < branches.size())
	{
		auto left = branches[leaf_choice++];
		auto right = branches[leaf_choice++];
		picosha2::hash256_hex_string(left->hash_ + right->hash_, 
			hex_digest);
		branches.emplace_back(new MerkleBranch{ hexlify(hex_digest), left, right });
	}
	
	//assert(leaf_choice + 1 == branches.size());
	root = branches.back();
}

uint32_t MerkleTree::get_leaf_at_address(uint32_t address)
{
	MerkleBranch* temp_branch = root;
	uint32_t num = address;
	uint32_t modulus = static_cast<uint32_t>(pow(2, height-1));
	uint32_t result = 0;
	for (unsigned i = 0; i < height; i++)
	{
		uint32_t flag = (num % 2);
		result += flag * modulus;
		modulus /= 2;
		num /= 2;
	}
	return leaves[result];
}




