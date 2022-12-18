#include "database.h"
#include "../config/config.h"
#include <functional>

namespace database{
    Database::Database(){
        _connection_string+="host=";
        _connection_string+=Config::get().get_host();
        _connection_string+=";user=";
        _connection_string+=Config::get().get_login();
        _connection_string+=";db=";
        _connection_string+=Config::get().get_database();
        _connection_string+=";password=";
        _connection_string+=Config::get().get_password();

        Poco::Data::MySQL::Connector::registerConnector();
        _pool = std::make_unique<Poco::Data::SessionPool>(Poco::Data::MySQL::Connector::KEY, _connection_string);
    }

    size_t Database::get_max_shard(){
        return 3;
    }

    std::vector<std::string> Database::get_all_hints(){
        std::vector<std::string> result;
        for(size_t i=0;i<=get_max_shard();++i){
            std::string shard_name = "-- sharding:";
            shard_name += std::to_string(i);
            result.push_back(shard_name);
        }
        return result;
    }

    std::string Database::sharding_hint(std::string login){

        std::string key;
        key = login + ";";

        size_t shard_number = std::hash<std::string>{}(key)%get_max_shard();

        std::string result = "-- sharding:";
        result += std::to_string(shard_number);
        return result;
    }

    Database& Database::get(){
        static Database _instance;
        return _instance;
    }

    Poco::Data::Session Database::create_session(){
        return Poco::Data::Session(_pool->get());
    }

}