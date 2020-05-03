#include <cmath>
#include <gtest/gtest.h>
#include <lemon/adaptors.h>
#include <lemon/concepts/digraph.h>
#include <lemon/lgf_reader.h>
#include <lemon/list_graph.h>
#include <lemon/preflow.h>
#include "mf_base.h"
using namespace lemon;
TEST(Preflow_Relabel, Constructor) {
    typedef concepts::Digraph Digraph;
    typedef double T;
    typedef Digraph::ArcMap<T> ArcMap;
    Digraph g;
    ArcMap aM(g);
    Preflow_Relabel<Digraph, ArcMap> pf(g, aM, INVALID, INVALID);
}

TEST(RelabelElevator, Constructor) {
    typedef concepts::Digraph Digraph;
    typedef Digraph::Node Item;
    Digraph g;
    RelabelElevator<Digraph, Item> re(g, 0);
}

TEST(RelabelElevator, MoveToFront) {
    typedef ListDigraph Digraph;
    typedef ListDigraph::Node Item;
    typedef RelabelElevator<Digraph, Item> RelabelElevator;
    Digraph g;
    Item a = g.addNode();
    Item b = g.addNode();
    RelabelElevator re(g, 2);
    re.initStart();
    re.initAddItem(a);
    re.initNewLevel();
    re.initAddItem(b);
    re.initFinish();
    EXPECT_EQ(re[a], 0);
    EXPECT_EQ(re[b], 1);
    re.activate(a);
    EXPECT_TRUE(re.active(a));
    re.deactivate(a);
    EXPECT_FALSE(re.active(a));
    RelabelElevator::iterator re_it = re.begin();
    EXPECT_EQ(g.id(*re_it), g.id(b));
    re_it++;
    EXPECT_EQ(g.id(*re_it), g.id(a));
    re.moveToFront(re_it);
    RelabelElevator::iterator re_it_2 = re.begin();
    EXPECT_EQ(g.id(*re_it_2), g.id(a));
    re_it_2++;
    EXPECT_EQ(g.id(*re_it_2), g.id(b));    
}

TEST(Preflow_Relabel, Run){
	// https://en.wikipedia.org/wiki/Push%E2%80%93relabel_maximum_flow_algorithm#Example
    typedef ListDigraph Digraph;
    typedef int T;
    typedef Digraph::ArcMap<T> ArcMap;    
    typedef ListDigraph::Node Node; 
    typedef ListDigraph::Arc Arc;
    Digraph g;
    Node n0 = g.addNode();
    Node n1 = g.addNode();
    Node n2 = g.addNode();
    Node n3 = g.addNode();
    Node n4 = g.addNode();
    Node n5 = g.addNode();   
    ArcMap aM(g);
    Arc a1 = g.addArc(n0, n1);
    Arc a2 = g.addArc(n1, n2);
    Arc a3 = g.addArc(n0, n3);
    Arc a4 = g.addArc(n3, n4);
    Arc a5 = g.addArc(n2, n5);
    Arc a6 = g.addArc(n4, n5);
    Arc a7 = g.addArc(n2, n3);
    Arc a8 = g.addArc(n4, n1);
    aM[a1] = 15;
    aM[a2] = 12;
    aM[a3] = 4;
    aM[a4] = 10;
    aM[a5] = 7;
    aM[a6] = 10;    
    aM[a7] = 3;
    aM[a8] = 5;
    Preflow_Relabel<Digraph, ArcMap> pf_relabel(g, aM, n0, n5);  
    Preflow<Digraph, ArcMap> pf(g, aM, n0, n5);
    pf.run();
    pf_relabel.init();
    pf_relabel.startFirstPhase();
    EXPECT_EQ(pf_relabel.flowValue(), pf.flowValue());
    pf_relabel.startSecondPhase();
    for (Digraph::NodeIt n(g); n != INVALID; ++n) {
        EXPECT_EQ(pf.minCut(n), pf_relabel.minCut(n));
    }
}

TEST(Preflow_Relabel, Infinity) {
    typedef ListDigraph Digraph;
    typedef double T;
    typedef Digraph::ArcMap<T> ArcMap;
    typedef ListDigraph::Node Node;
    typedef ListDigraph::Arc Arc;
    Digraph g;
    Node s = g.addNode();
    Node n1 = g.addNode();
    Node n2 = g.addNode();
    Node t = g.addNode();
    ArcMap aM(g);
    Arc a1 = g.addArc(s, n1);
    Arc a2 = g.addArc(s, n2);
    Arc a3 = g.addArc(n1, n2);
    Arc a4 = g.addArc(n2, t);
    aM[a1] = 1;
    aM[a2] = 1;
    aM[a3] = 1;
    aM[a4] = INFINITY;
    Preflow_Relabel<Digraph, ArcMap> pf_relabel(g, aM, s, t);
    pf_relabel.init();
    pf_relabel.startFirstPhase();
    EXPECT_DOUBLE_EQ(pf_relabel.flowValue(), 2);
    pf_relabel.startSecondPhase();
    EXPECT_TRUE(pf_relabel.minCut(s));
    EXPECT_TRUE(pf_relabel.minCut(n1));
    EXPECT_FALSE(pf_relabel.minCut(n2));
    EXPECT_FALSE(pf_relabel.minCut(t));
}

TEST(Preflow_Relabel, MinSourceSide) {
	typedef ListDigraph Digraph;
	typedef int T;
	typedef Digraph::ArcMap<T> ArcMap;
	typedef ListDigraph::Node Node;
	typedef ListDigraph::Arc Arc;
	Digraph g;
	Node n0 = g.addNode();
	Node n1 = g.addNode();
	Node n2 = g.addNode();
	ArcMap aM(g);
	Arc a1 = g.addArc(n0, n1);
	Arc a2 = g.addArc(n1, n2);
	aM[a1] = 0;
	aM[a2] = 0;
	Preflow_Relabel<Digraph, ArcMap> pf_relabel(g, aM, n0, n2);
	Preflow<Digraph, ArcMap> pf(g, aM, n0, n2);
	pf.run();
	pf_relabel.init();
	pf_relabel.startFirstPhase();
	EXPECT_EQ(pf_relabel.flowValue(), pf.flowValue());
	pf_relabel.startSecondPhase();
	for (Digraph::NodeIt n(g); n != INVALID; ++n) {
		EXPECT_EQ(pf.minCut(n), pf_relabel.minCut(n));
	}
}
TEST(Preflow_Relabel, Official) {
    typedef ListDigraph Digraph;
    typedef int T;
    typedef Digraph::ArcMap<T> ArcMap;
    typedef ListDigraph::Node Node;
    // use official directed graph to test
    Digraph g;
    ArcMap cap(g);
    Node s, t;
    digraphReader(g, "test.lgf")
        .arcMap("capacity", cap)
        .node("source", s)
        .node("target", t)
        .run();
    Preflow_Relabel<Digraph, ArcMap> pf_relabel(g, cap, s, t);
    Preflow<Digraph, ArcMap> pf(g, cap, s, t);
    pf.run();
    pf_relabel.init();
    pf_relabel.startFirstPhase();
    EXPECT_EQ(pf_relabel.flowValue(), pf.flowValue());
    pf_relabel.startSecondPhase();
    for (Digraph::NodeIt n(g); n != INVALID; ++n) {
        if(g.id(n) != 0 && g.id(n) != 7)
            EXPECT_EQ(pf.minCut(n), pf_relabel.minCut(n));
    }
}
TEST(Preflow_Relabel, Tolerance) {
	// any edge with capacity < Tolerance<double>::def_epsilon is treated as zero.
	typedef ListDigraph Digraph;
	typedef double T;
	typedef Digraph::ArcMap<T> ArcMap;
	typedef ListDigraph::Node Node;
	typedef ListDigraph::Arc Arc;
	Digraph g;
	Node s = g.addNode();
	Node n1 = g.addNode();
	Node n2 = g.addNode();
	Node t = g.addNode();
	ArcMap aM(g);
	Arc a1 = g.addArc(s, n1);
	Arc a2 = g.addArc(n1, t);
	Arc a3 = g.addArc(s, n2);
	Arc a4 = g.addArc(n2, t);
	aM[a1] = 1e-11;
	aM[a2] = 1.0;
	aM[a3] = 1.0;
	aM[a4] = 1.0;
	Preflow_Relabel<Digraph, ArcMap> pf_relabel(g, aM, s, t);
	Preflow<Digraph, ArcMap> pf(g, aM, s, t);
	pf.run();
	pf_relabel.init();
	pf_relabel.startFirstPhase();
	EXPECT_EQ(pf_relabel.flowValue(), pf.flowValue());
	pf_relabel.startSecondPhase(true);
	for (Digraph::NodeIt n(g); n != INVALID; ++n) {
		EXPECT_EQ(pf.minCut(n), pf_relabel.minCutSource(n));
	}
}

TEST(Preflow_Relabel, GetAndSetElevator) {
	typedef ListDigraph Digraph;
	typedef int T;
	typedef Digraph::ArcMap<T> ArcMap;
	typedef ListDigraph::Node Node;
	typedef ListDigraph::Arc Arc;
	Digraph g;
	Node n0 = g.addNode();
	Node n1 = g.addNode();
	Node n2 = g.addNode();
	Node n3 = g.addNode();
	Node n4 = g.addNode();
	Node n5 = g.addNode();
	ArcMap aM(g);
	Arc a1 = g.addArc(n0, n1);
	Arc a2 = g.addArc(n1, n2);
	Arc a3 = g.addArc(n0, n3);
	Arc a4 = g.addArc(n3, n4);
	Arc a5 = g.addArc(n2, n5);
	Arc a6 = g.addArc(n4, n5);
	Arc a7 = g.addArc(n2, n3);
	Arc a8 = g.addArc(n4, n1);
	aM[a1] = 15;
	aM[a2] = 12;
	aM[a3] = 4;
	aM[a4] = 10;
	aM[a5] = 7;
	aM[a6] = 10;
	aM[a7] = 3;
	aM[a8] = 5;
	Preflow_Relabel<Digraph, ArcMap> pf_relabel(g, aM, n0, n5);
	pf_relabel.run();
	Preflow_Relabel<Digraph, ArcMap>::Elevator* ele = pf_relabel.elevator();
	Preflow_Relabel<Digraph, ArcMap> pf_relabel2(g, aM, n0, n5);
	aM[a5] = 6;
	Preflow_Relabel<Digraph, ArcMap>::FlowMap new_flowMap(g);
	const Preflow_Relabel<Digraph, ArcMap>::FlowMap& old_flowMap = pf_relabel.flowMap();
	for (ListDigraph::ArcIt a(g); a != INVALID; ++a) {
		new_flowMap[a] = old_flowMap[a];
	}
	if (old_flowMap[a5] > aM[a5])
		new_flowMap[a5] = aM[a5];
	pf_relabel2.init(new_flowMap, ele);
	pf_relabel2.startFirstPhase();
	pf_relabel2.startSecondPhase();
	EXPECT_DOUBLE_EQ(pf_relabel2.flowValue(), 13);
}

TEST(Preflow_Relabel, ReverseElevator){
    // the copy constructor works for ReverseDigraph
    typedef ReverseDigraph<ListDigraph> Digraph;
    typedef RelabelElevator<Digraph, typename Digraph::Node>  Elevator;
    ListDigraph g;
    Digraph g_r(g);
    Elevator a(g_r, 2);
    Elevator a2(a);
}
#ifdef INTERRUPT
void test_thread_interrupt(bool& is_interrupted) {
	typedef ListDigraph Digraph;
	typedef int T;
	typedef Digraph::ArcMap<T> ArcMap;
	typedef ListDigraph::Node Node;
	Digraph g;
	ArcMap cap(g);
	Node s, t;
	digraphReader(g, "test.lgf")
		.arcMap("capacity", cap)
		.node("source", s)
		.node("target", t)
		.run();
	Preflow_Relabel<Digraph, ArcMap> pf_relabel(g, cap, s, t);
	try {
		pf_relabel.run();
	}
	catch (InterruptibleThread::thread_interrupted&) {
		is_interrupted = true;
		return;
	}
	is_interrupted = false;
}
TEST(Preflow_Relabel, Interrupt) {
	bool is_interrupted;
    InterruptibleThread::thread t(&test_thread_interrupt, std::ref(is_interrupted));
	t.interrupt();
	t.join();
	EXPECT_TRUE(is_interrupted);
}
#endif

TEST(FIFOElevator, GetFrontBasic) {
	typedef ListDigraph::Digraph Digraph;
	typedef Digraph::Node Item;
	Digraph g;
	Item n1 = g.addNode();
	Item n2 = g.addNode();
	FIFOElevator<Digraph, Item> fifo(g, 2);
	fifo.activate(n2);
	Item n3;
	bool get_result = fifo.getFront(n3);
	EXPECT_TRUE(get_result);
	EXPECT_EQ(g.id(n3), g.id(n2));
	get_result = fifo.getFront(n3);
	EXPECT_FALSE(get_result);
}

TEST(FIFOElevator, GetFrontSecondPhase) {
	typedef ListDigraph Digraph;
	typedef ListDigraph::Node Item;
	typedef FIFOElevator<Digraph, Item> FIFOElevator;
	Digraph g;
	Item a = g.addNode();
	Item b = g.addNode();
	Item c = g.addNode();
	FIFOElevator re(g, 3);
	re.initStart();
	re.initAddItem(a);
	re.initNewLevel();
	re.initAddItem(b);
	re.initFinish();
	EXPECT_EQ(re[a], 0);
	EXPECT_EQ(re[b], 1);
	re.activate(c);
	re.activate(a);
	Item n1;
	bool get_item = re.getFront(n1, true);
	EXPECT_TRUE(get_item);
	EXPECT_EQ(g.id(n1), g.id(a));
	get_item = re.getFront(n1, true);
	EXPECT_FALSE(get_item);
	get_item = re.getFront(n1, false);
	EXPECT_TRUE(get_item);
	EXPECT_EQ(g.id(n1), g.id(c));
}
TEST(Preflow_FIFO, RUN) {
	typedef ListDigraph Digraph;
	typedef int T;
	typedef Digraph::ArcMap<T> ArcMap;
	typedef ListDigraph::Node Node;
	typedef ListDigraph::Arc Arc;
	Digraph g;
	Node n0 = g.addNode();
	Node n1 = g.addNode();
	Node n2 = g.addNode();
	Node n3 = g.addNode();
	Node n4 = g.addNode();
	Node n5 = g.addNode();
	ArcMap aM(g);
	Arc a1 = g.addArc(n0, n1);
	Arc a2 = g.addArc(n1, n2);
	Arc a3 = g.addArc(n0, n3);
	Arc a4 = g.addArc(n3, n4);
	Arc a5 = g.addArc(n2, n5);
	Arc a6 = g.addArc(n4, n5);
	Arc a7 = g.addArc(n2, n3);
	Arc a8 = g.addArc(n4, n1);
	aM[a1] = 15;
	aM[a2] = 12;
	aM[a3] = 4;
	aM[a4] = 10;
	aM[a5] = 7;
	aM[a6] = 10;
	aM[a7] = 3;
	aM[a8] = 5;
	Preflow_FIFO<Digraph, ArcMap> pf_fifo(g, aM, n0, n5);
	Preflow<Digraph, ArcMap> pf(g, aM, n0, n5);
	pf.run();
	pf_fifo.init();
	pf_fifo.startFirstPhase();
	EXPECT_EQ(pf_fifo.flowValue(), pf.flowValue());
	pf_fifo.startSecondPhase();
	for (Digraph::NodeIt n(g); n != INVALID; ++n) {
		EXPECT_EQ(pf.minCut(n), pf_fifo.minCut(n));
	}
}
TEST(Preflow_FIFO, Official) {
	typedef ListDigraph Digraph;
	typedef int T;
	typedef Digraph::ArcMap<T> ArcMap;
	typedef ListDigraph::Node Node;
	// use official directed graph to test
	Digraph g;
	ArcMap cap(g);
	Node s, t;
	digraphReader(g, "test.lgf")
		.arcMap("capacity", cap)
		.node("source", s)
		.node("target", t)
		.run();
	Preflow_FIFO<Digraph, ArcMap> pf_fifo(g, cap, s, t);
	Preflow<Digraph, ArcMap> pf(g, cap, s, t);
	pf.run();
	pf_fifo.init();
	pf_fifo.startFirstPhase();
	EXPECT_EQ(pf_fifo.flowValue(), pf.flowValue());
	pf_fifo.startSecondPhase();
	for (Digraph::NodeIt n(g); n != INVALID; ++n) {
		if (g.id(n) != 0 && g.id(n) != 7)
			EXPECT_EQ(pf.minCut(n), pf_fifo.minCut(n));
	}
}

TEST(HLElevator, GetHighestBasic) {
    typedef ListDigraph::Digraph Digraph;
    typedef Digraph::Node Item;
    Digraph g;
    Item n1 = g.addNode();
    Item n2 = g.addNode();
    HLElevator<Digraph, Item> hl(g, 2);
    hl.initNewLevel();
    hl.initAddItem(n2);
    hl.activate(n1);
    hl.activate(n2);
    Item n3;
    bool get_result = hl.get_node_with_highest_label(n3);
    EXPECT_TRUE(get_result);
    EXPECT_EQ(g.id(n3), g.id(n2));
    get_result = hl.get_node_with_highest_label(n3);
    EXPECT_TRUE(get_result);
    EXPECT_EQ(g.id(n3), g.id(n1));
    get_result = hl.get_node_with_highest_label(n3);
    EXPECT_FALSE(get_result);
}
TEST(HLElevator, GetHighestSecondPhase) {
    typedef ListDigraph Digraph;
    typedef ListDigraph::Node Item;
    typedef HLElevator<Digraph, Item> HLElevator;
    Digraph g;
    Item a = g.addNode();
    Item b = g.addNode();
    Item c = g.addNode();
    HLElevator re(g, 3);
    re.initStart();
    re.initAddItem(a);
    re.initNewLevel();
    re.initAddItem(b);
    re.initFinish();
    EXPECT_EQ(re[a], 0);
    EXPECT_EQ(re[b], 1);
    re.activate(c);
    re.activate(a);
    Item n1;
    bool get_item = re.get_node_with_highest_label(n1, true);
    EXPECT_TRUE(get_item);
    EXPECT_EQ(g.id(n1), g.id(a));
    get_item = re.get_node_with_highest_label(n1, true);
    EXPECT_FALSE(get_item);
    get_item = re.get_node_with_highest_label(n1, false);
    EXPECT_TRUE(get_item);
    EXPECT_EQ(g.id(n1), g.id(c));
}
TEST(Preflow_HL, RUN) {
	typedef ListDigraph Digraph;
	typedef int T;
	typedef Digraph::ArcMap<T> ArcMap;
	typedef ListDigraph::Node Node;
	typedef ListDigraph::Arc Arc;
	Digraph g;
	Node n0 = g.addNode();
	Node n1 = g.addNode();
	Node n2 = g.addNode();
	Node n3 = g.addNode();
	Node n4 = g.addNode();
	Node n5 = g.addNode();
	ArcMap aM(g);
	Arc a1 = g.addArc(n0, n1);
	Arc a2 = g.addArc(n1, n2);
	Arc a3 = g.addArc(n0, n3);
	Arc a4 = g.addArc(n3, n4);
	Arc a5 = g.addArc(n2, n5);
	Arc a6 = g.addArc(n4, n5);
	Arc a7 = g.addArc(n2, n3);
	Arc a8 = g.addArc(n4, n1);
	aM[a1] = 15;
	aM[a2] = 12;
	aM[a3] = 4;
	aM[a4] = 10;
	aM[a5] = 7;
	aM[a6] = 10;
	aM[a7] = 3;
	aM[a8] = 5;
	Preflow_HL<Digraph, ArcMap> pf_hl(g, aM, n0, n5);
	Preflow<Digraph, ArcMap> pf(g, aM, n0, n5);
	pf.run();
	pf_hl.init();
	pf_hl.startFirstPhase();
	EXPECT_EQ(pf_hl.flowValue(), pf.flowValue());
	pf_hl.startSecondPhase();
	for (Digraph::NodeIt n(g); n != INVALID; ++n) {
		EXPECT_EQ(pf.minCut(n), pf_hl.minCut(n));
	}
}
TEST(Preflow_HL, Official) {
	typedef ListDigraph Digraph;
	typedef int T;
	typedef Digraph::ArcMap<T> ArcMap;
	typedef ListDigraph::Node Node;
	// use official directed graph to test
	Digraph g;
	ArcMap cap(g);
	Node s, t;
	digraphReader(g, "test.lgf")
		.arcMap("capacity", cap)
		.node("source", s)
		.node("target", t)
		.run();
	Preflow_HL<Digraph, ArcMap> pf_hl(g, cap, s, t);
	Preflow<Digraph, ArcMap> pf(g, cap, s, t);
	pf.run();
	pf_hl.init();
	pf_hl.startFirstPhase();
	EXPECT_EQ(pf_hl.flowValue(), pf.flowValue());
	pf_hl.startSecondPhase();
	for (Digraph::NodeIt n(g); n != INVALID; ++n) {
		if (g.id(n) != 0 && g.id(n) != 7)
			EXPECT_EQ(pf.minCut(n), pf_hl.minCut(n));
	}
}

TEST(Preflow_Parallel, Construction) {
	typedef ListDigraph Digraph;
	typedef int T;
	typedef Digraph::ArcMap<T> ArcMap;
	typedef ListDigraph::Node Node;
	typedef ListDigraph::Arc Arc;
	Digraph g;
	Node n0 = g.addNode();
	Node n1 = g.addNode();
	Node n2 = g.addNode();
	Node n3 = g.addNode();
	Node n4 = g.addNode();
	Node n5 = g.addNode();
	ArcMap aM(g);
	Arc a1 = g.addArc(n0, n1);
	Arc a2 = g.addArc(n1, n2);
	Arc a3 = g.addArc(n0, n3);
	Arc a4 = g.addArc(n3, n4);
	Arc a5 = g.addArc(n2, n5);
	Arc a6 = g.addArc(n4, n5);
	Arc a7 = g.addArc(n2, n3);
	Arc a8 = g.addArc(n4, n1);
	aM[a1] = 15;
	aM[a2] = 12;
	aM[a3] = 4;
	aM[a4] = 10;
	aM[a5] = 7;
	aM[a6] = 10;
	aM[a7] = 3;
	aM[a8] = 5;
	Preflow_Parallel<Digraph, ArcMap> pf_hl(g, aM, n0, n5);	
}
