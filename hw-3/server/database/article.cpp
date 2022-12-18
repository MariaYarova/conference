#include "article.h"
#include "database.h"
#include "../config/config.h"

#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/MySQL/MySQLException.h>
#include <Poco/Data/SessionFactory.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>

#include <sstream>
#include <exception>
#include <random>

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

namespace database
{
    Poco::JSON::Object::Ptr Article::toJSON() const
    {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

        root->set("id", _id);
        root->set("login", _login);
        root->set("article_number_in_catalog", _article_number_in_catalog);
        root->set("organization", _organization);
        root->set("field_of_study", _field_of_study);
        root->set("article_dt", _article_dt);
        root->set("acceptance_fg", _acceptance_fg);
        root->set("presentation_tm", _presentation_tm);

        return root;
    }

    std::string Article::_generate_presentation_time()
    {
        std::random_device rd; // obtain a random number from hardware
        std::mt19937 gen(rd()); // seed the generator
        std::uniform_int_distribution<> distr(10, 21); // define the range
        int hour = distr(gen);

        return std::to_string(hour) + ":00";
    }

    std::string Article::accept_article(long id)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement select(session);
            Article a;
            select << "SELECT login, article_number_in_catalog, organization, field_of_study, article_dt, acceptance_fg, presentation_tm FROM Article where id=?",
                into(a._login),
                into(a._article_number_in_catalog),
                into(a._organization),
                into(a._field_of_study),
                into(a._article_dt),
                into(a._acceptance_fg),
                into(a._presentation_tm),
                use(id),
                range(0, 1); //  iterate over result set one row at a time
            select.execute();
            Poco::Data::RecordSet rs(select);
            if (!rs.moveFirst()) throw std::logic_error("not found");

            // article must be not accepted yet
            if (a.get_acceptance_fg() == "1") {
                throw std::logic_error("already accepted");
            }

            std::string time = database::Article::_generate_presentation_time();
            Poco::Data::Statement update(session);
            update << "UPDATE Article SET acceptance_fg='1', presentation_tm=? WHERE id=?",
                use(time),
                use(id);
            update.execute();

            return time;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }


    void Article::save_to_mysql()
    {
        try {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement insert(session);

            insert << "INSERT INTO Article (login,article_number_in_catalog,organization,field_of_study,article_dt,acceptance_fg) VALUES(?, ?, ?, ?, ?, ?)",
                use(_login),
                use(_article_number_in_catalog),
                use(_organization),
                use(_field_of_study),
                use(_article_dt),
                use(_acceptance_fg);

            insert.execute();
        }
        catch (Poco::Data::MySQL::ConnectionException &e) {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e) {
            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    std::vector<Article> Article::read_all()
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement select(session);
            std::vector<Article> result;
            Article a;
            select << "SELECT id, login, article_number_in_catalog, organization, field_of_study, article_dt, acceptance_fg, presentation_tm FROM Article",
                into(a._id),
                into(a._login),
                into(a._article_number_in_catalog),
                into(a._organization),
                into(a._field_of_study),
                into(a._article_dt),
                into(a._acceptance_fg),
                into(a._presentation_tm),
                range(0, 1); //  iterate over result set one row at a time

            while (!select.done())
            {
                if(select.execute())  result.push_back(a);
            }
            return result;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }
    std::vector<Article> Article::read_all_on_conf()
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement select(session);
            std::vector<Article> result;
            Article a;
            a.acceptance_fg() = "1";
            select << "SELECT id, login, article_number_in_catalog, organization, field_of_study, article_dt, acceptance_fg, presentation_tm FROM Article where acceptance_fg=?",
                into(a._id),
                into(a._login),
                into(a._article_number_in_catalog),
                into(a._organization),
                into(a._field_of_study),
                into(a._article_dt),
                into(a._acceptance_fg),
                into(a._presentation_tm),
                use(a._acceptance_fg),
                range(0, 1); //  iterate over result set one row at a time

            while (!select.done())
            {
                if(select.execute())  result.push_back(a);
            }
            return result;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    long Article::get_id() const
    {
        return _id;
    }
    const std::string &Article::get_login() const
    {
        return _login;
    }
    const std::string &Article::get_article_number_in_catalog() const
    {
        return _article_number_in_catalog;
    }
    const std::string &Article::get_organization() const
    {
        return _organization;
    }
    const std::string &Article::get_field_of_study() const
    {
        return _field_of_study;
    }
    const std::string &Article::get_article_dt() const
    {
        return _article_dt;
    }
    const std::string &Article::get_acceptance_fg() const
    {
        return _acceptance_fg;
    }
    const std::string &Article::get_presentation_tm() const
    {
        return _presentation_tm;
    }

    long &Article::id()
    {
        return _id;
    }
    std::string &Article::login()
    {
        return _login;
    }
    std::string &Article::article_number_in_catalog()
    {
        return _article_number_in_catalog;
    }
    std::string &Article::organization()
    {
        return _organization;
    }
    std::string &Article::field_of_study()
    {
        return _field_of_study;
    }
    std::string &Article::article_dt()
    {
        return _article_dt;
    }
    std::string &Article::acceptance_fg()
    {
        return _acceptance_fg;
    }
    std::string &Article::presentation_tm()
    {
        return _presentation_tm;
    }
}