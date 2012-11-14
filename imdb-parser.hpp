/**
 * Copyright (c) 2012 Kim Syrjälä <kim.syrjala@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 */

#ifndef __IMDB_PARSER_HPP
#define __IMDB_PARSER_HPP

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cstdlib>

namespace imdb {

bool operator==(const std::string lhs, const std::string& rhs)
{
    return std::operator==(to_upper(lhs), to_upper(rhs));
}

std::string to_upper(const std::string& string)
{
    std::string string_upp;
    std::transform(string.begin(), string.end(), std::back_inserter(string_upp), toupper);
    return string_upp;
}

bool find(const std::string& string, const std::string& pattern)
{
    return to_upper(string).find(to_upper(pattern)) != std::string::npos;
}

template <class T>
bool contains(const typename T::value_type& needle, const T& haystack)
{
    for(auto& e : haystack)
        if(needle == e) return true;
    return false;
}

template <class T>
bool contains(const T& haystack1, const T& haystack2)
{
    for(auto& needle1 : haystack1)
        for(auto& needle2 : haystack2)
            if(needle1 == needle2) return true;
    return false;
}

template <typename T> typename
T::const_iterator map_at(const T& map, size_t index)
{
    size_t i = 0;
    typename T::const_iterator it;
    for(it = map.begin(); it != map.end(); ++it) 
        if(++i == index) break;
    return it;
}

struct movie_info
{
    std::string title;
    int year;
    std::vector<std::string> genres;
    std::vector<std::string> keywords;
    int votes;
    float rating;
    std::string plot;
};

typedef std::string movie_id;
typedef std::pair<movie_id, movie_info> movie_entry;
typedef std::unordered_map<movie_id, movie_info> movie_entries;

movie_id& get_movie_id(movie_entry& entry)
{
    return entry.first;
}

const movie_id& get_movie_id(const movie_entry& entry)
{
    return entry.first;
}

movie_info& get_movie_info(movie_entry& entry)
{
    return entry.second;
}

const movie_info& get_movie_info(const movie_entry& entry)
{
    return entry.second;
}

template <typename T>
struct range
{
    T from, to;
    
    range() {}
    
    range(const T& x)
      : from(x), to(x) {}
    
    range(const T& from, const T& to)
      : from(from), to(to) {}
};

typedef range<int> year_range;
typedef range<float> rating_range;

template <typename T>
bool in_range(const T& x, const range<T>& r)
{
    return x >= r.from && x <= r.to;
}

/**
 * genres.list format:
 * The Movie Title (year)   Genre
 */
movie_entries& parse_genres(movie_entries& movies)
{
    std::ifstream file("genres.list");
    std::string line;
    int lineno = 0;
    size_t pos;
    
    while(std::getline(file, line)) {
        // Skip to line 379 where the list starts.
        if(lineno++ < 379)
            continue;
        // List ends on blank line.
        if(line.size() < 2)
            break;
    
        // Skip series.
        if(line[0] == '"' || line[0] == '\'')
            continue;
        
        // Skip VideoGames.
        pos = line.find("(VG)");
        if(pos != std::string::npos)
            continue;

        // Extract title.
        pos = line.find_first_of('(');
        std::string title = line.substr(0, pos-1);
        
        // Extract year.
        std::string year = line.substr(pos+1, 4);
        
        // Extract genre.
        pos = line.find_last_of('\t');
        std::string genre = line.substr(pos+1, std::string::npos);
        
        // Movie name = title + year for uniqueness
        std::string name = title + " (" + year + ")";
        
        // Store info.
        movie_info& info = movies[name];
        info.title = title;
        info.year = std::atoi(year.c_str());
        info.genres.push_back(genre);
    }
    
    return movies;
}

/*
 * keywords.list format:
 * The Movie Title (year)   Genre
 */
movie_entries& parse_keywords(movie_entries& movies)
{
    std::ifstream file("keywords.list");
    std::string line;
    int lineno = 0;
    size_t pos;
    
    while(std::getline(file, line)) {
        // Skip to line 379 where the list starts.
        if(lineno++ < 52833)
            continue;
        // List ends on blank line.
        if(line.size() < 2)
            break;
    
        // Skip series.
        if(line[0] == '"' || line[0] == '\'')
            continue;
        
        // Skip VideoGames.
        pos = line.find("(VG)");
        if(pos != std::string::npos)
            continue;

        // Extract title.
        pos = line.find_first_of('(');
        std::string title = line.substr(0, pos-1);
        
        // Extract year.
        std::string year = line.substr(pos+1, 4);
        
        // Extract keyword.
        pos = line.find_last_of('\t');
        std::string keyword = line.substr(pos+1, std::string::npos);
        
        // Movie name = title + year for uniqueness
        std::string name = title + " (" + year + ")";
        
        // Store info.
        movie_info& info = movies[name];
        info.keywords.push_back(keyword);
    }
    
    return movies;
}

/**
 * ratings.list format:
 * Distruction  Votes   Rating  Name
 * 0123456789   00      1.0     The Movie Title (year)
 */
movie_entries& parse_ratings(movie_entries& movies)
{
    std::ifstream file("ratings.list");
    std::string line;
    int lineno = 0;
    size_t pos;
    
    while(std::getline(file, line)) {
        // Skip to line where the list starts.
        if(lineno++ < 296)
            continue;
        // List ends on blank line.
        if(line.size() < 2)
            break;
        
        std::string distribution;
        int votes;
        float rating;
        std::string name;
        
        std::stringstream stream(line);
        stream >> distribution;
        stream >> votes;
        stream >> rating;
        name = line.substr((size_t)stream.tellg()+2, std::string::npos);
        
        // Skip series.
        if(name[0] == '"' || name[0] == '\'')
            continue;
        
        // Skip VideoGames.
        pos = name.find("(VG)");
        if(pos != std::string::npos)
            continue;
        
        movie_info& info = movies[name];
        info.votes = votes;
        info.rating = rating;
    }

    return movies;
}

/**
 * plots.list format:
 * MV: The Movie Title (year)
 * 
 * PL: Lorem ipsum
 * PL: dolor sit amet.
 *
 * BY: Author
 *
 * PL: Lorem ipsum
 * PL: dolor sit amet.
 *
 * BY: Author
 */
movie_entries& parse_plots(movie_entries& movies)
{
    std::ifstream file("plot.list");
    std::string line;
    std::string name;
    std::string plot;
    
    while(!std::getline(file, line).eof()) {
        // Read a MV line.
        if(line[0] == 'M' && line[1] == 'V') {
            name = line.substr(4, std::string::npos);
            
            // Skip series.
            if(name[0] == '"' || name[0] == '\'') {
                name = "";
                continue;
            }
            
            // Skip VideoGames.
            if(name.find("(VG)") != std::string::npos) {
                name = "";
                continue;
            }
        }
        
        // Read a PL line.
        if(line[0] == 'P' && line[1] == 'L' && name.size() > 0)
            plot += line.substr(4, std::string::npos);
        
        // Read a BY line.
        if(line[0] == 'B' && line[1] == 'Y' && name.size() > 0) {
            movies[name].plot = plot;
            plot = "";
        }
    }

    return movies;
}

movie_entries& get_movies(movie_entries& movies)
{
    parse_genres(movies);
    parse_keywords(movies);
    parse_ratings(movies);
    parse_plots(movies);
    
    return movies;
}

movie_entries& filter_movies(
    const movie_entries& movies,
    movie_entries& filtered,
    const std::string* filter_title,
    const year_range* filter_year,
    const std::vector<std::string>* filter_genre,
    const rating_range* filter_rating)
{
    for(auto& e : movies) {
        const movie_id& name = get_movie_id(e);
        const movie_info& info = get_movie_info(e);
        
        if(filter_title && !find(info.title, *filter_title))
            continue;
        
        if(filter_year && !in_range(info.year, *filter_year))
            continue;
        
        if(filter_genre && !contains(info.genres, *filter_genre))
            continue;
        
        if(filter_rating && info.votes == 0)
            continue;
        if(filter_rating && !in_range(info.rating, *filter_rating))
            continue;
        
        filtered[name] = info;
    }
    
    return filtered;
}

} // namespace imdb

#endif
