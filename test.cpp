#include <gtest/gtest.h>
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
	pf_relabel.startSecondPhase(false);
	for (Digraph::NodeIt n(g); n != INVALID; ++n) {
		EXPECT_EQ(pf.minCut(n), pf_relabel.minCutSink(n));
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
	pf_relabel.startSecondPhase();
	for (Digraph::NodeIt n(g); n != INVALID; ++n) {
		EXPECT_EQ(pf.minCut(n), pf_relabel.minCut(n));
	}
}