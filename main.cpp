#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>
#include <lemon/lgf_reader.h>
#include <lemon/list_graph.h>
#include <lemon/preflow.h>
#include "mf_base.h"
int main(int argc, const char *argv[]){
    boost::program_options::options_description desc;
	desc.add_options()
		("help,h", "Show this help screen")
		("use_original", boost::program_options::value<bool>()->implicit_value(true)->default_value(false), "whether to use the original test");

    boost::program_options::positional_options_description pos_desc;
	pos_desc.add("filename", 1);
	boost::program_options::command_line_parser parser{ argc, argv };
	parser.options(desc).positional(pos_desc);
	boost::program_options::parsed_options parsed_options = parser.run();

	boost::program_options::variables_map vm;
	boost::program_options::store(parsed_options, vm);
	boost::program_options::notify(vm);

	if (vm.count("help") || !vm.count("finename")) {
		std::cout << desc << '\n';
		return 0;
	}
	std::string filename;
	bool use_original_algorithm;
	try{
		use_original_algorithm = vm["use_original"].as<bool>();
		filename = vm["filename"].as<std::string>();
	}
	catch (const boost::program_options::error & ex) {
		std::cerr << ex.what() << '\n';
	}
	std::ifstream fin(filename);

	typedef lemon::ListDigraph Digraph;
	typedef double T;
	typedef Digraph::ArcMap<T> ArcMap;
	typedef Digraph::Node Node;

	Digraph digraph;
	ArcMap cap(digraph);
	Node src, trg;
	lemon::digraphReader(digraph, fin)
		.arcMap("capacity", cap)
		.node("source", src)
		.node("target", trg)
		.run();
	double max_flow_value;
	if (use_original_algorithm) {
		lemon::Preflow<Digraph, ArcMap> alg(digraph, cap, src, trg);
		alg.run();
		max_flow_value = alg.flowValue();
	}
	else {
		lemon::Preflow_Relabel<Digraph, ArcMap> alg(digraph, cap, src, trg);
		alg.run();
		max_flow_value = alg.flowValue();
	}
	std::cout << "max flow value : " << max_flow_value << '\n';
	return 0;
}