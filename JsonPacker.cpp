// JsonPacker.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <type_traits>
#include "boost/json/src.hpp"
#include "tlv/tlv_box.h"
//
// Container for storing json keys to number mapping.
//
using keys2NumbersDictType = std::unordered_map<std::string_view, long>;
//
// Using boost::json for parsing json objects.
namespace bj = boost::json;
//
// Fill keys to number dictionary with new keys.
// If key is already exist then do nothing.
//
void fillKey2NumberDictionary(const bj::object& obj, keys2NumbersDictType& outDict)
{
	for (auto& pair : obj)
	{
		std::string_view key(pair.key().data(), pair.key().size());
		auto it = outDict.find(key);
		if(it != outDict.end())
			outDict[key] = outDict.size() + 1;
	}
}
//
// Helper function to return value type number.
// It associate value type with number.
//
template <typename T> int getValueType(T a)
{
	if (std::is_same_v<T, long>)
		return 1;
	else if (std::is_same_v<T, bool>)
		return 2;
	else if (std::is_same_v<T, std::string_view>)
		return 3;
	else
		throw "Value type is not supported.";
	return 0;
}

int main(int argc, char* argv[])
{
	//
	// Streams for input output.
	// 
	std::ifstream istrm(argv[1], std::ios::in);
	std::ofstream ofstrm(argv[2], std::ios::out | std::ios::binary);
	if (!ofstrm.is_open())
	{
		std::cout << "Failed to open " << argv[2] << " for writing. Please provide valid file.\n";
		return 1;
	}
	if (!istrm.is_open()) 
	{
		std::cout << "Failed to open " << argv[1] << ". Please provide valid file.\n";
		return 1;
	}
	//
	// Store keys->numbers mapping.
	keys2NumbersDictType keys2Numbers;
	//
	// Parse each line to json object.
	for (std::string raw; std::getline(istrm, raw); )
	{
		bj::error_code ec;
		bj::value tmpValue = bj::parse(raw, ec);
		if (ec)
		{
			std::cout << "Parsing failed: " << ec.message() << "\n";
			return 1;
		}
		bj::object obj = tmpValue.as_object();
		fillKey2NumberDictionary(obj, keys2Numbers);

		//
		// Using tlv library for getting TLV encoded data.
		//
		tlv::TlvBox tlvBox;
		for (auto& item : obj)
		{
			std::string_view key(item.key().data(), item.key().size());
			tlvBox.PutIntValue(getValueType((long)1), keys2Numbers[key]);
			if (item.value().is_bool())
				tlvBox.PutBoolValue(getValueType((bool)true), item.value().get_bool());
			else if (item.value().is_int64()) // Threating int values in json as long values.
				tlvBox.PutLongValue(getValueType((long)1), item.value().get_int64());
			else if (item.value().is_string())
				tlvBox.PutStringValue(getValueType((std::string_view)""), item.value().get_string().c_str());
			else
			{
				std::cout << "INvalid json type is detected.\n";
			}
		}
		tlvBox.Serialize();
		ofstrm.write((const char*)tlvBox.GetSerializedBuffer(), tlvBox.GetSerializedBytes());
		ofstrm << std::endl;
	}
	//
	// Writing keys->numbers mapping at the end of file.
	//
	ofstrm << std::endl;
	tlv::TlvBox anotherTlvBox;
	for (auto& item : keys2Numbers)
	{
		anotherTlvBox.PutStringValue(getValueType((std::string_view)""), std::string(item.first));
		anotherTlvBox.PutLongValue(getValueType((long)1), item.second);
	}
	anotherTlvBox.Serialize();
	ofstrm.write((const char*)anotherTlvBox.GetSerializedBuffer(), anotherTlvBox.GetSerializedBytes());

	ofstrm.close();
	istrm.close();
	return 0;
}
