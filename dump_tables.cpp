#include <iostream>
#include <map>
#include "imdb-parser.hpp"

using namespace imdb;

std::string quotes(const std::string& string)
{
    std::string result;
    for(size_t i = 0; i < string.size(); ++i) {
        if(string[i] == '\'') 
            result.append("\'\'");
        else 
            result.push_back(string[i]);
    }
    return result;
}

void dump_movies_sql(const movie_entries& movies, const std::string& filename)
{
    std::ofstream file(filename);
    std::stringstream stream;
    std::string string;
    
    file << "CREATE TABLE IF NOT EXISTS `movies` (\n"
         << "\t`id` bigint(32) unsigned NOT NULL AUTO_INCREMENT,\n"
         << "\t`title` varchar(255) NOT NULL DEFAULT '',\n"
         << "\t`year` int(4) NOT NULL DEFAULT '0',\n"
         << "\t`rating` decimal(2,1) NOT NULL DEFAULT '0',\n"
         << "\t`votes` int(16) NOT NULL DEFAULT '0',\n"
         << "\t`genres` varchar(255) NOT NULL DEFAULT '',\n"
         << "\t`keywords` varchar(255) NOT NULL DEFAULT '',\n"
         << "\t`plot` longtext NOT NULL DEFAULT '',\n"
         << "\tPRIMARY KEY (`id`)\n"
         << ") ENGINE=MyISAM DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;\n\n";
    
    file << "INSERT INTO `movies` (`id`, `title`, `year`, `rating`, `votes`, `genres`, `keywords`, `plot`) VALUES\n";
    
    for(auto& movie : movies) {
        const movie_id& name = movie.first;
        const movie_info& info = movie.second;
        
        std::string genres;
        for(auto& genre : info.genres)
            genres += genre + ", ";
        genres = genres.substr(0, genres.size()-2);
        
        std::string keywords;
        for(auto& keyword : info.keywords)
            keywords += keyword + ", ";
        keywords = keywords.substr(0, keywords.size()-2);
        
        stream << "(" << "NULL" 
               << "," << "'" << quotes(info.title) << "'"
               << "," << "'" << info.year          << "'"
               << "," << "'" << info.rating        << "'"
               << "," << "'" << info.votes         << "'"
               << "," << "'" << quotes(genres)     << "'"
               << "," << "'" << quotes(keywords)   << "'"
               << "," << "'" << quotes(info.plot)  << "'"
               << "),\n";
    }
    
    string = stream.str();
    string[string.size()-2] = ';';
    file << string;
}

void dump_genres_sql(const movie_entries& movies, const std::string& filename)
{
    std::map<std::string, unsigned int> genres;
    
    for(auto& movie : movies) {
        const movie_id& name = movie.first;
        const movie_info& info = movie.second;
        
        for(auto& genre : info.genres)
            genres[genre]++;
    }
    
    std::ofstream file(filename);
    std::stringstream stream;
    std::string string;
    
    file << "CREATE TABLE IF NOT EXISTS `genres` (\n"
         << "\t`id` bigint(32) unsigned NOT NULL AUTO_INCREMENT,\n"
         << "\t`name` varchar(255) NOT NULL DEFAULT '',\n"
         << "\t`count` int(16) NOT NULL DEFAULT '0',\n"
         << "\tPRIMARY KEY (`id`)\n"
         << ") ENGINE=MyISAM DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;\n\n";
    
    file << "INSERT INTO `genres` (`id`, `name`, `count`) VALUES\n";
    
    for(auto& genre : genres) {
        const std::string& name = genre.first;
        const unsigned int count = genre.second;
    
        stream << "(" << "NULL" 
               << "," << "'" << quotes(name) << "'"
               << "," << "'" << count        << "'"
               << "),\n";
    }
    
    string = stream.str();
    string[string.size()-2] = ';';
    file << string;
}

void dump_keywords_sql(const movie_entries& movies, const std::string& filename)
{
    std::map<std::string, unsigned int> keywords;
    
    for(auto& movie : movies) {
        const movie_id& name = movie.first;
        const movie_info& info = movie.second;
        
        for(auto& keyword : info.keywords) {
            keywords[keyword]++;
        }
    }
    
    std::ofstream file(filename);
    std::stringstream stream;
    std::string string;
    
    file << "CREATE TABLE IF NOT EXISTS `keywords` (\n"
         << "\t`id` bigint(32) unsigned NOT NULL AUTO_INCREMENT,\n"
         << "\t`name` varchar(255) NOT NULL DEFAULT '',\n"
         << "\t`count` int(16) NOT NULL DEFAULT '0',\n"
         << "\tPRIMARY KEY (`id`)\n"
         << ") ENGINE=MyISAM DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;\n\n";
    
    file << "INSERT INTO `keywords` (`id`, `name`, `count`) VALUES\n";
    
    for(auto& keyword : keywords) {
        const std::string& name = keyword.first;
        const unsigned int count = keyword.second;
    
        stream << "(" << "NULL" 
               << "," << "'" << quotes(name) << "'"
               << "," << "'" << count        << "'"
               << "),\n";
    }
    
    string = stream.str();
    string[string.size()-2] = ';';
    file << string;
}

int main()
{
    movie_entries movies;
    get_movies(movies);
    
    movie_entries filtered;
    year_range filter_year(1970, 2012);
    rating_range filter_rating(5.0, 10.0);
    movies = filter_movies(movies, filtered, NULL, &filter_year, NULL, &filter_rating);
    
    std::cout << "Filtered " << movies.size() << " movies\n";
    
    dump_movies_sql(movies, "movies.sql");
    dump_genres_sql(movies, "genres.sql");
    dump_keywords_sql(movies, "keywords.sql");
    
    return 0;
}
