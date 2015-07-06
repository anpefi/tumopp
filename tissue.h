// -*- mode: c++; coding: utf-8 -*-
/*! @file tissue.h
    @brief Interface of Gland class
*/
#pragma once
#ifndef TISSUE_H_
#define TISSUE_H_
#include <cmath>
#include <iostream>
#include <vector>
#include <map>
#include <string>

#include "gland.h"

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////

namespace boost {
    namespace program_options {
        class options_description;
    }
}

class Tissue {
  public:
    Tissue(const std::vector<int>& origin): coords_{1, origin} {
        tumor_.emplace(origin, Gland());
    }
    Tissue(const size_t dimensions=2): Tissue{std::vector<int>(dimensions)} {};

    void mark(const size_t n);

    void grow(const size_t max_size);

    std::string snapshot(const std::string& sep="\t") const;
    std::string mutation_history(const std::string& sep="\t") const;

    friend std::ostream& operator<< (std::ostream&, const Tissue&);

    static void unit_test();
    static boost::program_options::options_description& opt_description();

  private:
    void push(Gland&& daughter, std::vector<int>* current_coords, const std::vector<int>& direction);

    std::map<std::vector<int>, Gland> tumor_;
    std::vector<std::vector<int>> coords_;
    std::vector<std::vector<int>> mutation_coords_;
    std::vector<size_t> mutation_stages_;
};

#endif /* TISSUE_H_ */
