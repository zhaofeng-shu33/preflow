'''This program converts Dimacs or gml format to LGF graph format(LEMON C++)
currently only digraph convertion is supported.
'''
import os
import argparse

import networkx as nx

def convert(filename, filetype='GML'):
    digraph, others = toNetworkX_fromFile(filename, filetype)
    st = write_lgf(digraph)
    if others.get('s') and others.get('t'):
        st += '\n@attributes\n'
        st += 'source ' + others['s'] + '\n'
        st += 'target ' + others['t'] + '\n'
    with open(filename.split('.')[0] + '.lgf', 'w') as f:
        f.write(st)

def readDimacs(filename):    
    g = nx.DiGraph()
    with open(filename) as f:
        for line in f.readlines():
            if line[0] == 'n':
                _, node_id, node_type = line.split()
                if node_type == 's':
                    s = node_id
                elif node_type == 't':
                    t = node_id
            elif line[0] == 'a':
                _, _s, _t, _cap = line.split()
                g.add_edge(_s, _t, weight=int(_cap))
    return (g, s, t)

def write_lgf(digraph):
    # convert networkx digraph to lgf string
    Ls = []
    for i in digraph.nodes:
        Ls.append(int(i))
    Ls.sort()
    Ls = [str(i) for i in Ls]
    Ls = ['@nodes', 'label'] + Ls
    if(type(Ls[2]) is not str):
        for i in range(len(Ls)):
            Ls[i] = str(Ls[i])
    Ls.append('@arcs')
    Ls.append('\t\tlabel\tcapacity')
    edge_cnt = 0
    for e in digraph.edges(data=True):
        i, j, dic = e
        w = 1
        if(dic.get('weight')):
            w = dic['weight']
        Ls.append('\t'.join([str(i), str(j), str(edge_cnt), str(w)]))
        edge_cnt += 1
    return '\n'.join(Ls)
    
def toNetworkX_fromFile(filename, filetype):
    if filetype == 'GML':
        digraph = nx.gml.read_gml(filename)
        others = {}
    elif filetype == 'DIMAC':
        digraph, s, t = readDimacs(filename)
        others = {'s' : s, 't' : t}
    return (digraph, others)
 
if __name__ == '__main__':
    parser = argparse.ArgumentParser('gml to lgf')
    parser.add_argument('filename', help='file to be converted')
    parser.add_argument('--type', choices=['GML', 'DIMAC'], default='GML')
    args = parser.parse_args()
    convert(os.path.join('build', args.filename), args.type)