// -*- mode: c++; coding: utf-8 -*-
/*! @file simulation.cpp
    @brief Inplementation of Simulation class
*/
#include "simulation.h"

#include "cxxwtils/iostr.hpp"
#include "cxxwtils/getopt.hpp"
#include "cxxwtils/prandom.hpp"
#include "cxxwtils/os.hpp"
#include "cxxwtils/gz.hpp"

#include "tissue.h"
#include "gland.h"

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////
// functions

boost::program_options::options_description& Simulation::opt_description() {HERE;
    namespace po = boost::program_options;
    static po::options_description description("Simulation");
    description.add_options()
        ("help,h", po::value<bool>()->default_value(false)->implicit_value(true), "produce help")
        ("verbose,v", po::value<bool>(&VERBOSE)
            ->default_value(VERBOSE)->implicit_value(true), "verbose output")
        ("test", po::value<int>()->default_value(0)->implicit_value(1))
        ("mode", po::value<int>(&MODE)->default_value(MODE))
        ("label", po::value<std::string>(&LABEL)->default_value("default"))
        ("top_dir", po::value<std::string>()->default_value(OUT_DIR.string()))
        ("seed", po::value<unsigned int>(&SEED)->default_value(SEED))
    ;
    return description;
}

//! Unit test for each class
inline void test() {HERE;
    Tissue::unit_test();
    Gland::unit_test();
}

Simulation::Simulation(int argc, char* argv[]) {HERE;
    std::vector<std::string> arguments(argv, argv + argc);
    std::cout << wtl::str_join(arguments, " ") << std::endl;
    std::cout << wtl::iso8601datetime() << std::endl;

    namespace po = boost::program_options;
    po::options_description description;
    description.add(opt_description());
    description.add(Gland::opt_description());
    description.add(Tissue::opt_description());
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, description), vm);
    po::notify(vm);

    if (vm["help"].as<bool>()) {
        description.print(std::cout);
        exit(0);
    }
    PROJECT_DIR = fs::canonical(argv[0]).parent_path();
    OUT_DIR = fs::path(vm["top_dir"].as<std::string>());
    wtl::sfmt().seed(SEED);
    const std::string CONFIG_STRING = wtl::flags_into_string(description, vm);
    if (VERBOSE) {
        std::cout << CONFIG_STRING << std::endl;
    }
    switch (vm["test"].as<int>()) {
      case 0:
        break;
      case 1:
        test();
        exit(0);
      default:
        exit(1);
    }
    const std::string now(wtl::strftime("%Y%m%d_%H%M%S"));
    std::ostringstream pid_at_host;
    pid_at_host << ::getpid() << "@" << wtl::gethostname();
    WORK_DIR = TMP_DIR / (now + "_" + LABEL + "_" + pid_at_host.str());
    derr("mkdir && cd to " << WORK_DIR << std::endl);
    fs::create_directory(WORK_DIR);
    wtl::cd(WORK_DIR.string());
    fs::create_directory(OUT_DIR);
    OUT_DIR /= (LABEL + "_" + now + "_" + pid_at_host.str());
    wtl::Fout{"program_options.conf"} << CONFIG_STRING;
}

void Simulation::run() {HERE;
    switch (MODE) {
      case 0: {
        tissue_.mark(4);
        tissue_.grow();
        wtl::gzip{wtl::Fout{"mutation_history.tsv.gz"}} << tissue_.mutation_history();
        wtl::gzip{wtl::Fout{"population.tsv.gz"}} << tissue_.snapshot();
        auto plot = (PROJECT_DIR / "plot2d.R").c_str();
        if (wtl::exists(plot)) {system(plot);}
        break;
      }
      case 1: {
        tissue_.grow();
        wtl::gzip{wtl::Fout{"mutation_history.tsv.gz"}} << tissue_.mutation_history();
        wtl::gzip{wtl::Fout{"population.tsv.gz"}} << tissue_.snapshot();
        break;
      }
      default:
        exit(1);
    }
    derr("mv results to " << OUT_DIR << std::endl);
    fs::rename(WORK_DIR, OUT_DIR);
    std::cout << wtl::iso8601datetime() << std::endl;
}