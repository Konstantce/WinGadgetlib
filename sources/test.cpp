#include "annealing.hpp"
#include "basic_gadgets.hpp"
#include "utils.hpp"
#include "Field.hpp"
#include "hasher.hpp"
#include "merkle_tree.hpp"

#include <algorithm>
#include <random>

using namespace gadgetlib;

struct very_large_test_field_impl
{
	static constexpr const char* characteristics
		= "2192857320389321840545175547229257073006128757787064809776291564948086256487001349549868962854716761977211";
	static constexpr size_t safe_bitsize = 349;
};

struct large_test_field_impl
{
	static constexpr const char* characteristics
		= "105732633691844908418600724518276605693537277189924299171828348124653624736205332238085309";
	static constexpr size_t safe_bitsize = 296;
};

//TBD: smaller modulus is error-prone due to inner bug (nit fixed yet)
struct small_test_field_impl
{
	static constexpr const char* characteristics
		= "3275661863067481859447961340841014559796679193854595675601036996505488982223";
	static constexpr size_t safe_bitsize = 251;
};

using inner_field_impl = very_large_test_field_impl;
using field = Field<inner_field_impl>;


void check(const gadget& gadget)
{
	auto pboard = protoboard<field>();
	auto annealing = engraver();
	annealing.incorporate_gadget(pboard, gadget);
	r1cs_example<field> example(pboard);
	std::cout << "Number of constraints: " << example.constraint_system.size() << std::endl;
	std::cout << "Satisfied: " << example.check_assignment() << std::endl;
	//example.dump();
}

void check_addition()
{
	gadget input(0x12345678, 32, false);
	gadget result(0xf0e21567, 32, true);
	gadget const_gadget(0xdeadbeef, 32);
	gadget comparison = ((input  + const_gadget) == result);

	check(comparison);
}


void check_addition_xor()
{
	gadget input(0x12345678, 32, false);
	gadget result(0xc1840208, 32, true);
	gadget const_gadget1(0xf0e21561, 32);
	gadget const_gadget2(0xdeadbeef, 32);
	gadget comparison = (((input ^ const_gadget1) + const_gadget2) == result);

	check(comparison);
}

void check_concat_extract()
{
	gadget input = gadget(0, 32) || gadget(0x1, 1) || gadget(0x0, 31) || gadget(0x2, 32) 
		|| gadget(3, 32) || gadget(4, 32) || gadget(5, 32) || gadget(6, 32) || 
		gadget(7, 32);
	gadget result(0x7, 32, true);
	gadget comparison = ((input)[{32 * 7, 32 * 7 +31}] == result);

	check(comparison);
}

void check_concat_extract2()
{
	gadget input = gadget(1, 16) || gadget(2, 16);
	gadget result = { 0x00010002, 32 };
	gadget comparison = (input == result);

	check(comparison);
}

void check_shr()
{
	gadget input(0x12345678, 32, false);
	gadget result(0x01234567, 32, true);
	gadget const_gadget(0x21436587, 32);
	gadget comparison = ((input >> 4)  == result);

	check(comparison);
}

void check_rotate()
{
	gadget input(0x12345678, 32, false);
	gadget result(0x81234567, 32, true);
	gadget const_gadget(0x21436587, 32);
	gadget comparison = ((input.rotate_right(32)) == input);

	check(comparison);
}

void check_and()
{
	gadget input(0x1122335E, 32, false);
	gadget result(0x1020324e, 32, true);
	gadget const_gadget(0xdeadbeef, 32);
	gadget comparison = ((input & const_gadget) == result);

	check(comparison);
}

void check_not()
{
	gadget input(0xffffffff, 32, false);
	gadget result(0x00000000, 32, true);
	gadget comparison = ((!input) == result);

	check(comparison);
}

void check_ITE()
{
	gadget first_var(0xdeadbeef, 32, false);
	gadget second_var(0x12345678, 32, false);
	gadget result(0x12345678, 32, true);
	gadget input(0, 1, true);

	gadget comparison = (ITE(input, first_var, second_var) == result);

	check(comparison);
}

void check_leq()
{
	gadget input1(1, 2, true);
	gadget input2(3, 2, true);
	gadget comparison = (ITE(input1 <= input2, gadget(1,1), gadget(0,1)) == gadget(1, 1));

	check(comparison);
}

void check_sha256()
{
	gadget input(0x33323138, 32, false);
	gadget result(0x9D21310B, 32, true);
	gadget comparison = ((sha256_gadget(input))[{224, 255}] == result);

	check(comparison);
}

#include "sha256.hpp"

void check_sha256v2()
{
	std::string input_str = "3218";
	std::string hex_digest;
	picosha2::hash256_hex_string(input_str, hex_digest);
	std::string result_str = hex_digest;

	gadget input(0x33323138, 32, true);
	gadget result(result_str, 256, true);
	gadget comparison = (sha256_gadget(input) == result);

	check(comparison);
}

void check_common_prefix_mask()
{
	gadget input1(11, 4, true);
	gadget input2(7, 4, true);
	gadget result = get_common_prefix_mask(input1, input2);
	gadget comparison = (result == gadget(3, 4));

	check(comparison);
}

void check_MimC()
{
	gadget input(0xdeadbeef, 32, true);
	using hasher = merkle_tree::MimcHash<inner_field_impl, uint32_t>;
	auto result = hasher::hash_leaf(0xdeadbeef);
	gadget result_gadget(std::string("d") + result.to_string(), false);
	gadget comparison = (result_gadget == MimcLeafHash(input));

	check(comparison);
}

template<typename FieldT>
std::vector<gadget> convert_proof_to_gadget(const std::vector<FieldT>& x)
{
	std::vector<gadget> result;
	for (auto& elem : x)
	{
		result.emplace_back(std::string("d") + elem.to_string(), false);
	}
	return result;
}

void check_merkle_proof()
{
	std::vector<uint32_t> leaves = { 0xdeadbeef, 0x112364e1, 3, 0x12345678};
	using hasher = merkle_tree::MimcHash<inner_field_impl, uint32_t>;
	merkle_tree::MerkleTree<hasher, uint32_t> tree(leaves);
	uint32_t raw_address = 1;
	gadget address(raw_address, tree.height(), true);
	gadget leaf(tree.get_leaf_at_address(raw_address), 32, false);
	std::vector<gadget> proof = convert_proof_to_gadget(tree.get_proof(raw_address));
	gadget merkle_root = gadget(std::string("d")+tree.get_root().to_string(), true);
	gadget flag = (merkle_tree_proof(address, leaf, proof, merkle_root, tree.height()));
	check(flag);
}

void check_transaction()
{
	std::vector<uint32_t> leaves = { 12, 43, 32, 41 };
	using hasher = merkle_tree::MimcHash<inner_field_impl, uint32_t>;
	merkle_tree::MerkleTree<hasher, uint32_t> tree(leaves);
	uint32_t raw_from_address = 1;
	uint32_t raw_to_address = 3;
	uint32_t raw_amount = 9;

	gadget merkle_root_before = gadget(std::string("d") + tree.get_root().to_string(), true);

	gadget from_address(raw_from_address, tree.height(), true);
	gadget from_balance(tree.get_leaf_at_address(raw_from_address), 32, false);
	std::vector<gadget> from_proof_before = convert_proof_to_gadget(tree.get_proof(raw_from_address));
	
	gadget to_address(raw_to_address, tree.height(), true);
	gadget to_balance(tree.get_leaf_at_address(raw_to_address), 32, false);
	std::vector<gadget> to_proof_before = convert_proof_to_gadget(tree.get_proof(raw_to_address));

	tree.update(raw_from_address, raw_to_address, raw_amount);

	gadget amount(raw_amount, 32, true);
	std::vector<gadget> from_proof_after = convert_proof_to_gadget(tree.get_proof(raw_from_address));
	std::vector<gadget> to_proof_after = convert_proof_to_gadget(tree.get_proof(raw_to_address));
	gadget merkle_root_after = gadget(std::string("d") + tree.get_root().to_string(), true);

	gadget flag = check_transaction(from_address, to_address, from_balance, to_balance, amount,
		merkle_root_before, merkle_root_after, from_proof_before, to_proof_before,
		from_proof_after, to_proof_after);

	check(flag);
}

void check_battleship_game()
{
	BattleshipGameParams game_params{ 10, 10, 4, 3, 2, 1 };
	const size_t PADDING_LEN = 4; //because 104 divides 8, and the size of battlefield is 100 = 10 x 10

	std::string valid_battlefield = "b"
		"1010110000"
		"0000000000"
		"0110101010"
		"0000101010"
		"1110001010"
		"0000100010"
		"0000000000"
		"0010000000"
		"0000000000"
		"0000000000";

	std::string first_invalid_battlefield = "b"
		"1010110000"
		"0000000000"
		"0110101000"
		"0000101000"
		"1110001000"
		"0000100000"
		"0110000000"
		"0000111100"
		"0000000000"
		"0000000000";

	std::string second_invalid_battlefield = "b"
		"1010110000"
		"0000000000"
		"0110101010"
		"0000101000"
		"1110001000"
		"0000100000"
		"0001000000"
		"0001000000"
		"0001000000"
		"0001000000";

	std::string str_refs[3] = {valid_battlefield, first_invalid_battlefield,
		second_invalid_battlefield };

	auto str_converter = [](const std::string& str) -> std::string
	{
		size_t len = (str.size() - 1) / 8;
		std::string result = std::string(len, 'x');
		for (unsigned i = 0; i < len; i++)
		{
			char c = 0;
			for (unsigned j = 0; j < 8; j++)
			{
				c *= 2;
				c += (str[1 + 8 * i + j] == '1' ? 1 : 0);
			}
			result[i] = c;
		}
		return result;
	};

	for (auto& str_elem : str_refs)
	{
		gadget battlefield(str_elem, 10 * 10, false);
		
		std::string salt_str = str_converter(str_elem + std::string(PADDING_LEN, '0')) + "1234";
		std::string hex_digest;
		picosha2::hash256_hex_string(salt_str, hex_digest);
		gadget public_hash(hex_digest, 256, true);
		gadget salt(0x31323334, 32, false);
		gadget comparison = start_battleship_game(battlefield, salt, public_hash,
			game_params, PADDING_LEN);

		check(comparison);
	}
}

void check_chooser_gadget()
{
	std::vector<gadget> gadgets;
	gadgets.emplace_back(1, 2, true);
	gadgets.emplace_back(0, 2, true);
	gadgets.emplace_back(3, 2, true);
	gadgets.emplace_back(1, 2, true);

	gadget index(2, 2, false);
	gadget result = chooser_gadget(gadgets, index);

	gadget comparison = (result == gadget(3, 2));
	check(comparison);
}

void check_shuffle()
{
	int NUM_OF_CARDS = 52;
	unsigned BITS_PER_CARD = 6;
	
	std::vector<gadget> initial_permutation;
	for (int i = NUM_OF_CARDS - 1; i >= 0; i--)
	{
		initial_permutation.emplace_back(i, BITS_PER_CARD, true);
	}
	
	std::random_device rd;
	std::mt19937 g(rd());

	std::vector<gadget> shuffle = initial_permutation;
	std::shuffle(shuffle.begin(), shuffle.end(), g);

	gadget result = shuffle_proof(shuffle, initial_permutation, BITS_PER_CARD);
	check(result);
}

void check_blackjack()
{
	int NUM_OF_CARDS = 52;
	unsigned BITS_PER_CARD = 6;

	std::vector<unsigned> initial_permutation;
	for (unsigned i = 0; i < NUM_OF_CARDS; i++)
	{
		initial_permutation.emplace_back(i);
	}

	std::random_device rd;
	std::mt19937 g(rd());

	std::vector<unsigned> dealer_shuffle = initial_permutation;
	std::vector<unsigned> player_shuffle = initial_permutation;
	std::shuffle(dealer_shuffle.begin(), dealer_shuffle.end(), g);
	std::shuffle(player_shuffle.begin(), player_shuffle.end(), g);

	std::uniform_int_distribution<unsigned> distribution(0, NUM_OF_CARDS - 1);
	unsigned index = distribution(g);  
	unsigned num = player_shuffle[dealer_shuffle[index]];

	gadget index_gadget = gadget(index, BITS_PER_CARD, true);
	gadget num_gadget = gadget(num, BITS_PER_CARD, true);


	auto convert_shuffle_to_str = [BITS_PER_CARD](const std::vector<unsigned>& vec) -> std::string
	{
		std::string result(vec.size() * BITS_PER_CARD + 1, 'x');
		result[0] = 'b';
		unsigned index = BITS_PER_CARD;
		for (auto elem : vec)
		{
			for (unsigned j = 0; j < BITS_PER_CARD; j++)
			{
				result[index] = (elem % 2 ? '1' : '0');
				elem /= 2;
				index--;
			}
			index += BITS_PER_CARD * 2;
		}
		return result;
	};

	auto dealer_shuffle_str = convert_shuffle_to_str(dealer_shuffle);
	auto player_shuffle_str = convert_shuffle_to_str(player_shuffle);
	gadget dealer_shuffle_str_gadget = gadget(dealer_shuffle_str, NUM_OF_CARDS * BITS_PER_CARD, false);
	gadget player_shuffle_str_gadget = gadget(player_shuffle_str, NUM_OF_CARDS * BITS_PER_CARD, true);

	size_t len = (dealer_shuffle_str.size() - 1) / 8;
	std::string salted_str = std::string(len, 'x');
	for (unsigned i = 0; i < len; i++)
	{
		char c = 0;
		for (unsigned j = 0; j < 8; j++)
		{
			c *= 2;
			c += (dealer_shuffle_str[1 + 8 * i + j] == '1' ? 1 : 0);
		}
		salted_str[i] = c;
	}

	std::string salt_str = "1234";
	std::string hex_digest;
	picosha2::hash256_hex_string(salted_str + salt_str, hex_digest);
	gadget dealer_commitment_gadget(hex_digest, 256, true);
	gadget salt_gadget(0x31323334, 32, false);


	gadget result = blackjack_dealer_proof(num_gadget, index_gadget, dealer_commitment_gadget,
		player_shuffle_str_gadget, dealer_shuffle_str_gadget, salt_gadget, NUM_OF_CARDS, BITS_PER_CARD);

	check(result);
}


void test_all()
{
	std::cout << "check addition: " << std::endl;
	check_addition();
	std::cout << "check concat_extract: " << std::endl;
	check_concat_extract();
	std::cout << "check concat_extract2: " << std::endl;
	check_concat_extract2();
	std::cout << "check shr: " << std::endl;
	check_shr();
	std::cout << "check rotate: " << std::endl;
	check_rotate();		
	std::cout << "check ITE: " << std::endl;
	check_ITE();
	std::cout << "check leq: " << std::endl;
	check_leq();
	std::cout << "check addition_xor: " << std::endl;
	check_addition_xor();
	std::cout << "check and: " << std::endl;
	check_and();
	std::cout << "check not: " << std::endl;
	check_not();
	std::cout << "check sha256: " << std::endl;
	check_sha256();
	std::cout << "check sha256v2: " << std::endl;
	check_sha256v2();
	std::cout << "check common prefix mask: " << std::endl;
	check_common_prefix_mask();
	std::cout << "check battleship_game: (Note that second and third tests should fail!" << std::endl;
	check_battleship_game();
	std::cout << "check chooser_gadget: " << std::endl;
	check_chooser_gadget();
	std::cout << "check shuffle: " << std::endl;
	check_shuffle();
	std::cout << "check MimC hash: " << std::endl;
	check_MimC();
	std::cout << "check Merkle-proof: " << std::endl;
	check_merkle_proof();
	std::cout << "check plasma transaction: " << std::endl;
	check_transaction();
	std::cout << "check blackjack game (first permutation): " << std::endl;
	check_blackjack();
	std::cout << "check blackjack game (second permutation): " << std::endl;
	check_blackjack();
}

int main(int argc, char* argv[])
{
	test_all();
	getchar();
}