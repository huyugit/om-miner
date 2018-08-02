#include "webjson.h"
#include "config/Config.h"

#include <utility>
#include <cstdio>
#include <cassert>
#include <cstring>
#include <fstream>
#include <istream>
#include <sstream>
#include <memory>
#include <set>
#include <limits>

Webjson::Webjson()
{

}

void Webjson::WriteJsonFile(const char* file, std::string str)
{
	std::ofstream ofs;
	ofs.open(file);

	ofs << str;
	ofs.close();
}

int Webjson::MinerParsefromFile(Json::Value& root)
{
    std::ifstream inputfs;
    Json::Reader reader;
	bool ret;
	
    inputfs.open(ConfigFilename, std::ios::binary);
	if(!inputfs.is_open())
	{
		return -1;
	}
	
    ret = reader.parse(inputfs, root, false);
	inputfs.close();

    return ret;
}

int Webjson::OscParsefromFile(Json::Value& root)
{
    std::ifstream inputfs;
    Json::Reader reader;
	bool ret;
	
    inputfs.open(OSCConfigFilename, std::ios::binary);
	if(!inputfs.is_open())
	{
		return -1;
	}
	
    ret = reader.parse(inputfs, root, false);
	inputfs.close();

	remove(OSCConfigFilename);

    return ret;
}

int Webjson::BinParsefromFile(Json::Value& root)
{
    std::ifstream inputfs;
    Json::Reader reader;
	bool ret;
	
    inputfs.open(BinConfigFilename, std::ios::binary);
	if(!inputfs.is_open())
	{
		return -1;
	}
	
    ret = reader.parse(inputfs, root, false);
	inputfs.close();

	remove(BinConfigFilename);

    return ret;
}

int Webjson::FanRpmParsefromFile(Json :: Value & root)
{
    std::ifstream inputfs;
    Json::Reader reader;
	bool ret;
	
    inputfs.open(FanRpmFilename, std::ios::binary);
	if(!inputfs.is_open())
	{
		return -1;
	}
	
    ret = reader.parse(inputfs, root, false);
	inputfs.close();

    return ret;
}

int Webjson::HashSNParsefromFile(Json::Value& root)
{
    std::ifstream inputfs;
    Json::Reader reader;
	bool ret;
	
    inputfs.open(HashSNConfigFilename, std::ios::binary);
	if(!inputfs.is_open())
	{
		return -1;
	}
	
    ret = reader.parse(inputfs, root, false);
	inputfs.close();

	remove(HashSNConfigFilename);

    return ret;
}

int Webjson::PsuWorkCondParsefromFile(Json::Value& root)
{
    std::ifstream inputfs;
    Json::Reader reader;
	bool ret;
	
    inputfs.open(PsuWorkCondFilename, std::ios::binary);
	if(!inputfs.is_open())
	{
		return -1;
	}
	
    ret = reader.parse(inputfs, root, false);
	inputfs.close();

    return ret;
}

int Webjson::TestConfParsefromFile(Json::Value& root)
{
	std::ifstream inputfs;
	Json::Reader reader;
	bool ret;

	inputfs.open(TestConfFilename, std::ios::binary);
	if (!inputfs.is_open())
	{
		return -1;
	}

	ret = reader.parse(inputfs, root, false);
	inputfs.close();


	return ret;
}

void Webjson::MinerInsertToFile(std::string json_str)
{
	std::ofstream ofs;
	ofs.open(StatusTmpFilename);

	ofs << json_str;
	ofs.close();

	rename(StatusTmpFilename, StatusFilename);
}

void Webjson::ASICInsertToFile(std::string json_str)
{
	std::ofstream ofs;
	ofs.open(ASICStatusFilename);

	ofs << json_str;
	ofs.close();
}

void Webjson::MasterStatusInsertToFile(std::string json_str)
{
	WriteJsonFile(MasterStatusTmpFilename, json_str);
	rename(MasterStatusTmpFilename, MasterStatusFilename);
}

void Webjson::BinStatusInsertToFile(std::string json_str)
{
	WriteJsonFile(BinConfigRdTmpFilename, json_str);
	rename(BinConfigRdTmpFilename, BinConfigRdFilename);
}

void Webjson::ChainStatusInsertToFile(std::string json_str)
{
	WriteJsonFile(ChainStatusTmpFilename, json_str);
	rename(ChainStatusTmpFilename, ChainStatusFilename);
}

void Webjson::PSUStatusInsertToFile(std::string json_str)
{
	WriteJsonFile(PSUStatusTmpFilename, json_str);
	rename(PSUStatusTmpFilename, PSUStatusFilename);
}

void Webjson::FanStatusInsertToFile(std::string json_str)
{
	WriteJsonFile(FanStatusTmpFilename, json_str);
	rename(FanStatusTmpFilename, FanStatusFilename);
}

void Webjson::PoolStatusInsertToFile(std::string json_str)
{
	WriteJsonFile(PoolStatusTmpFilename, json_str);
	rename(PoolStatusTmpFilename, PoolStatusFilename);
}

void Webjson::MinerInfoInsertToFile(std::string json_str)
{
	WriteJsonFile(MinerInfoTmpFilename, json_str);
	rename(MinerInfoTmpFilename, MinerInfoFilename);
}

