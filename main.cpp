#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <boost/program_options.hpp>
#include <lemon/lgf_reader.h>
#include <lemon/list_graph.h>
#include <lemon/preflow.h>
#include "mf_base.h"
int main(int argc, const char *argv[]){
    boost::program_options::options_description desc;
	desc.add_options()
		("help,h", "Show this help screen")
		("filename", boost::program_options::value<std::string>(), "lgf file name")
		("method", boost::program_options::value<std::string>()->default_value("hl"),
			"maxflow implementation: rtf, hl, fifo, o_hl, pg")
		("timing", boost::program_options::value<bool>()->implicit_value(true)->default_value(false),
			"whether to timing the algorithm")
		("print_cut", boost::program_options::value<bool>()->implicit_value(true)->default_value(false), "whether to print the min cut set of source side");

	boost::program_options::command_line_parser parser{ argc, argv };
	parser.options(desc);
	boost::program_options::variables_map vm;

	try{
		boost::program_options::parsed_options parsed_options = parser.run();
		boost::program_options::store(parsed_options, vm);
		boost::program_options::notify(vm);
	}
	catch (const boost::program_options::error & ex) {
		std::cerr << ex.what() << '\n';
	}
	if (vm.count("help") || !vm.count("filename")) {
		std::cout << desc << '\n';
		return 0;
	}
	std::string filename;
	std::string method_short_name;
	bool print_cut, timing;
	try{
		method_short_name = vm["method"].as<std::string>();
		print_cut = vm["print_cut"].as<bool>();
		timing = vm["timing"].as<bool>();
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
	typedef Digraph::NodeIt NodeIt;

	Digraph digraph;
	ArcMap cap(digraph);
	Node src, trg;
	lemon::digraphReader(digraph, fin)
		.arcMap("capacity", cap)
		.node("source", src)
		.node("target", trg)
		.run();
	double max_flow_value;
	std::string method_name;
	std::stringstream cut_set;
	cut_set << '{';
	std::chrono::system_clock::time_point start_time;
	std::chrono::system_clock::time_point end_time;
    
	
	if (method_short_name == "hl") {
		lemon::Preflow_HL<Digraph, ArcMap> alg(digraph, cap, src, trg);
		method_name = "highest label";
		start_time = std::chrono::system_clock::now();
		alg.run();
		max_flow_value = alg.flowValue();
		for (NodeIt n(digraph); n != lemon::INVALID; ++n) {
			if (alg.minCut(n))
				cut_set << digraph.id(n) << ',';
		}
	} else if (method_short_name == "rtf") {
		lemon::Preflow_Relabel<Digraph, ArcMap> alg(digraph, cap, src, trg);
		method_name = "relabel to front";
		start_time = std::chrono::system_clock::now();
		alg.run();
		max_flow_value = alg.flowValue();
		for (NodeIt n(digraph); n != lemon::INVALID; ++n) {
			if (alg.minCut(n))
				cut_set << digraph.id(n) << ',';
		}
	} else if (method_short_name == "fifo") {
		lemon::Preflow_FIFO<Digraph, ArcMap> alg(digraph, cap, src, trg);
		method_name = "first in first out";
		start_time = std::chrono::system_clock::now();
		alg.run();
		max_flow_value = alg.flowValue();
		for (NodeIt n(digraph); n != lemon::INVALID; ++n) {
			if (alg.minCut(n))
				cut_set << digraph.id(n) << ',';
		}
	} else if (method_short_name == "o_hl") {
		lemon::Preflow<Digraph, ArcMap> alg(digraph, cap, src, trg);
		method_name = "original highest label";
		start_time = std::chrono::system_clock::now();
		alg.run();
		max_flow_value = alg.flowValue();
		for (NodeIt n(digraph); n != lemon::INVALID; ++n) {
			if (alg.minCut(n))
				cut_set << digraph.id(n) << ',';
		}
	} else {
		lemon::Preflow_Parallel<Digraph, ArcMap> alg(digraph, cap, src, trg);
		method_name = "parallel generic";
		start_time = std::chrono::system_clock::now();
		alg.run();
		max_flow_value = alg.flowValue();
		for (NodeIt n(digraph); n != lemon::INVALID; ++n) {
			if (alg.minCut(n))
				cut_set << digraph.id(n) << ',';
		}
	}
	end_time = std::chrono::system_clock::now();

	std::cout << "using " << method_name  <<", max flow value : " << max_flow_value << '\n';
	if (print_cut) {
		std::string cut_set_string = cut_set.str();
		cut_set_string[cut_set_string.length() - 1] = '}';
		std::cout << "min cut set of source side: " << cut_set_string << '\n';
	}
	if (timing) {
		std::chrono::system_clock::duration dtn;
		float time_used;
        dtn = end_time - start_time;
        time_used = std::chrono::duration_cast<std::chrono::milliseconds>(dtn).count()/1000.0;
		std::cout << "time used " << time_used << "s" << std::endl;
	}
	return 0;
}