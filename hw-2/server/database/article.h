#ifndef ARTICLE_H
#define ARTICLE_H

#include <string>
#include <vector>
#include "Poco/JSON/Object.h"

namespace database
{
    class Article{
        private:
            
            long _id;
            std::string _login;
            std::string _article_number_in_catalog;
            std::string _organization;
            std::string _field_of_study;
            std::string _article_dt;
            std::string _acceptance_fg;
            std::string _presentation_tm;

            static std::string _generate_presentation_time();

        public:

            long               get_id() const;
            const std::string &get_login() const;
            const std::string &get_article_number_in_catalog() const;
            const std::string &get_organization() const;
            const std::string &get_field_of_study() const;
            const std::string &get_article_dt() const;
            const std::string &get_acceptance_fg() const;
            const std::string &get_presentation_tm() const;

            long&        id();
            std::string &login();
            std::string &article_number_in_catalog();
            std::string &organization();
            std::string &field_of_study();
            std::string &article_dt();
            std::string &acceptance_fg();
            std::string &presentation_tm();

            void save_to_mysql();
            static std::string accept_article(long id);
            static std::vector<Article> read_all();
            static std::vector<Article> read_all_on_conf();

            Poco::JSON::Object::Ptr toJSON() const;
    };
}

#endif