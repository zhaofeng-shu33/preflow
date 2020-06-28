'''
generate lgf graph used to benchmark different maximum flow algorithm.
controled by node_num
'''
import os
import argparse

from sklearn.metrics.pairwise import pairwise_kernels
from sklearn import datasets
import networkx as nx

from toLGF import write_lgf

def generate_gaussian(num_node, tolerance=1e-10):
    '''
        returns: networkx.DiGraph
    '''
    pos_list, _ = datasets.make_blobs(n_samples = num_node, centers=[[0,0]], cluster_std=1)
    affinity_matrix = pairwise_kernels(pos_list, metric='rbf', gamma = 0.6)
    ms = affinity_matrix.shape[0]
    digraph = nx.DiGraph()
    for i in range(ms):
        for j in range(i + 1, ms):
            w = affinity_matrix[i,j]
            if(w > tolerance):
                digraph.add_edge(i,j,weight=w)
    return digraph

def generate_mf_lgf(digraph):
    lgf_str = write_lgf(digraph)
    # add (s, t)
    target_id = len(digraph.nodes) - 1
    lgf_str += '\n@attributes\nsource 0\ntarget %d\n' % target_id
    return lgf_str

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('filename', help='file to be written')
    parser.add_argument('--num_node', type=int, default=100)
    args = parser.parse_args()
    digraph = generate_gaussian(args.num_node)
    with open(os.path.join('build', args.filename), 'w') as f:
        f.write(generate_mf_lgf(digraph))