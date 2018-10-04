#ifndef FIELD_HPP_
#define FIELD_HPP_

#pragma warning (disable : 4146)
#include <NTL/ZZ_pEX.h>

#include <variant>
#include <iostream>
#include <string>
#include <sstream>

#include <boost/variant.hpp>

namespace gadgetlib
{
	template<typename T>
	class Field
	{
	public:
		NTL::ZZ_p num_;
		static bool initialized_;
		static NTL::ZZ chp;

	private:
		void initialize()
		{
			if (!initialized_)
			{
				initialized_ = true;
				chp = NTL::conv<NTL::ZZ>(T::characteristics);
				NTL::ZZ_p::init(chp);
			}
		}

		NTL::ZZ hexToZZ(const std::string& hexVal)
		{
			bool decimal = (hexVal[0] == 'd');
			bool binary = (hexVal[0] == 'b');
			
			auto convert_ch = [](char c) -> int
			{
				if (c >= '0' && c <= '9')
					return (c - '0');
				if (c >= 'a' && c <= 'f')
					return (c - 'a' + 10);

			};
			
			NTL::ZZ val;
			val = NTL::to_ZZ(0);	//initialise the value to zero
			
			if (decimal || binary)
			{
				for (unsigned i = 1; i < hexVal.length(); i++)
				{
					val *= (decimal ? 10 : 2);
					val += convert_ch(hexVal[i]);

				}
			}
			else
			{ 
				for (unsigned i = 0; i < hexVal.length(); i++)
				{
					val *= 0x10;
					val += convert_ch(hexVal[i]);

				}
			}
			return val;
		}
		
	public:
		static constexpr uint64_t safe_bitsize = T::safe_bitsize;

		Field(size_t num)
		{
			initialize();
			NTL::ZZ temp = NTL::conv<NTL::ZZ>(num);
			num_ = NTL::conv<NTL::ZZ_p>(temp);
		}

		Field()
		{
			initialize();
		}

		Field(const NTL::ZZ_p& num)
		{
			initialize();
			num_ = num;
		}

		Field(const boost::variant<uint32_t, std::string>& v)
		{
			initialize();
			//uint32_t n;
			switch (v.which())
			{
			case 0:
			{
				NTL::ZZ num = NTL::conv<NTL::ZZ>(boost::get<uint32_t>(v));
				num_ = NTL::conv<NTL::ZZ_p>(num);
				break;
			}
			case 1:
				NTL::ZZ int_num = hexToZZ(boost::get<std::string>(v));
				num_ = NTL::conv<NTL::ZZ_p>(int_num);
				break;
			};
		}

		Field(bool flag, bool q)
		{
			initialize();
			NTL::ZZ num = NTL::conv<NTL::ZZ>(flag ? 1 : 0);
			num_ = NTL::conv<NTL::ZZ_p>(num);
		}

		Field& operator+=(const Field& rhs)
		{
			this->num_ += rhs.num_;
			return *this;
		}


		Field& operator-=(const Field& rhs)
		{
			this->num_ -= rhs.num_;
			return *this;
		}

		Field& operator*=(const Field& rhs)
		{
			this->num_ *= rhs.num_;
			return *this;
		}

		Field& operator-()
		{
			this->num_ = -this->num_;
			return *this;
		}


		Field& operator/=(const Field& rhs)
		{
			this->num_ = NTL::conv<NTL::ZZ_p>(NTL::rep(num_) / NTL::rep(rhs.num_));
			return *this;
		}

		static Field one()
		{
			return 1;
		}

		operator bool() const
		{
			return num_ != 0;
		}

		std::string to_string() const
		{
			std::stringstream buffer;
			buffer << num_;
			return buffer.str();
		}

		Field inverse() const
		{
			NTL::ZZ_p inverse = NTL::conv<NTL::ZZ_p>(NTL::InvMod(NTL::rep(num_), chp));
			return Field(inverse);
		}
	};

	template<typename T>
	Field<T> operator+(const Field<T>& left,
		const Field<T>& right)
	{
		return Field<T>(left.num_ + right.num_);
	}

	template<typename T>
	Field<T> operator-(const Field<T>& left,
		const Field<T>& right)
	{
		return Field<T>(left.num_ - right.num_);
	}

	template<typename T>
	Field<T> operator*(const Field<T>& left,
		const Field<T>& right)
	{
		return Field<T>(left.num_ * right.num_);
	}

	template<typename T>
	bool operator==(const Field<T>& left,
		const Field<T>& right)
	{
		return (left.num_ == right.num_);
	}

	template<typename T>
	bool operator!=(const Field<T>& left,
		const Field<T>& right)
	{
		return (left.num_ != right.num_);
	}

	template<typename T>
	Field<T> operator%(const Field<T>& left,
		int right)
	{
		assert(right == 2);
		return NTL::rem(rep(left.num_), right);
	}

	template<typename T>
	std::ostream& operator<< (std::ostream& stream, const Field<T>& elem)
	{
		stream << elem.num_;
		return stream;
	}

	template<typename T>
	bool Field<T>::initialized_ = false;

	template<typename T>
	NTL::ZZ Field<T>::chp;
}

#endif
