'''This program converts gml format to LGF graph format(LEMON C++)
currently only digraph convertion is supported.
'''

import argparse
import networkx as nx
import os
def convert(filename):
    digraph = toNetworkX(filename)
    Ls = ['@nodes', 'label']
    for i in digraph.nodes:
        Ls.append(i)
    Ls.append('@arcs')
    Ls.append('\t\tlabel\tcapacity')
    edge_cnt = 0
    for e in digraph.edges(data=True):
        i, j, dic = e
        w = 1
        if(dic.get('weight')):
            w = dic['weight']
        Ls.append('\t'.join([i, j, str(edge_cnt), str(w)]))
        edge_cnt += 1
    with open(filename.replace('gml','lgf'),'w') as f:
        f.write('\n'.join(Ls))

    
def toNetworkX(filename):
    digraph = nx.gml.read_gml(filename)
    return digraph
    
if __name__ == '__main__':
    parser = argparse.ArgumentParser('gml to lgf')
    parser.add_argument('filename', help='GML file to be converted')
    parser.add_argument('--debug', help='enter debug mode', default=False, type=bool, nargs='?', const=True)
    args = parser.parse_args()
    if(args.debug):
        pdb.set_trace()
    convert(os.path.join('build', args.filename))