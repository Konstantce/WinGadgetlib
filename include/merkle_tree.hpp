#ifndef MERKLE_TREE_HPP_
#define MERKLE_TREE_HPP_

#include <vector>
#include <math.h>

namespace merkle_tree
{
	template<typename HASHER, typename LONGINT>
	class MerkleTree
	{
	public:
		using hash_type = typename HASHER::HASH_DIGEST_TYPE;
	private:
		struct MerkleBranch
		{
			hash_type hash_;
			MerkleBranch* left_;
			MerkleBranch* right_;
			MerkleBranch(const hash_type& hash, MerkleBranch* left, MerkleBranch* right) :
				left_(left), right_(right), hash_(hash) {}
		};
	
		std::vector<MerkleBranch*> branches_;	
		std::vector<LONGINT> leaves_;
		LONGINT height_;
		MerkleBranch* root_;

		LONGINT calc_leaf_index(LONGINT address)
		{
			MerkleBranch* temp_branch = root_;
			LONGINT num = address;
			LONGINT modulus = static_cast<LONGINT>(pow(2, height_ - 1));
			LONGINT result = 0;
			for (unsigned i = 0; i < height_; i++)
			{
				LONGINT flag = (num % 2);
				result += flag * modulus;
				modulus /= 2;
				num /= 2;
			}
			return result;
		}

		void calculate_tree()
		{
			for (auto& leaf : leaves_)
			{
				hash_type hex_digest = HASHER::hash_leaf(leaf);
				branches_.emplace_back(new MerkleBranch{ hex_digest, nullptr, nullptr });
			}
			unsigned leaf_choice = 0;
			while (leaf_choice + 1 < branches_.size())
			{
				auto left = branches_[leaf_choice++];
				auto right = branches_[leaf_choice++];
				hash_type hex_digest = HASHER::hash_branch(left->hash_, right->hash_);
				branches_.emplace_back(new MerkleBranch{ hex_digest, left, right });
			}
			root_ = branches_.back();
		}
			
	public:
		std::vector<hash_type> get_proof(LONGINT address)
		{
			std::vector<hash_type> result;
			MerkleBranch* temp_branch = root_;
			LONGINT num = address;
			for (LONGINT i = 0; i < height_; i++)
			{
				bool flag = (num % 2);
				num /= 2;
				if (flag)
				{
					result.emplace_back(temp_branch->left_->hash_);
					temp_branch = temp_branch->right_;
				}
				else
				{
					result.emplace_back(temp_branch->right_->hash_);
					temp_branch = temp_branch->left_;
				}
			}
			std::reverse(result.begin(), result.end());
			return result;
		}

		MerkleTree(const std::vector<LONGINT>& leaves) : leaves_(leaves)
		{
			height_ = static_cast<LONGINT>(log2(leaves.size()));
			calculate_tree();
		}

		~MerkleTree()
		{
			for (auto branch : branches_)
				delete branch;
		}

		void update(LONGINT from, LONGINT to, LONGINT amount)
		{
			for (auto branch : branches_)
				delete branch;
			branches_.clear();
			leaves_[calc_leaf_index(from)] -= amount;
			leaves_[calc_leaf_index(to)] += amount;
			calculate_tree();
		}

		hash_type get_root()
		{
			return root_->hash_;
		}
		
		LONGINT get_leaf_at_address(LONGINT address)
		{
			return leaves_[calc_leaf_index(address)];
		}

		LONGINT height()
		{
			return height_;
		}
	};
}

#endif




