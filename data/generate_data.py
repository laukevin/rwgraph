"""
Generates a text file of X_id to Y_id -
we use this for our recommendation graph.

essentially consider the X_id to be 1 type of node, and the
Y_id to be another type of node. We want to compute the recommendations
from a given Y_id_i to Y_id_j based off random walking the graph.

This translates into recommendations if we consider X_id to be video_ids
and Y_id to be hashtags, in which case we can get a list of recommended
hashtags by walking the graph from Y_id through X_id.
"""


import os
import random
import numpy as np

from collections import defaultdict

curr_dir = os.path.dirname(os.path.realpath("__file__"))

def generate_random_graph_data(num_x=10000, num_y=1000):
    X_ids = list(range(0, num_x))
    Y_ids = list(range(0, num_y))

    graph = defaultdict(list)
    for i in X_ids:
        num_neighbors = np.random.poisson(lam=10)
        candidate_Y_id = random.choice(Y_ids)
        sampled_Y_ids = np.random.normal(loc=candidate_Y_id, scale=50, size=num_neighbors).astype(int)
        sampled_Y_ids = sampled_Y_ids[(sampled_Y_ids >= 0) & (sampled_Y_ids <= len(Y_ids))]
        graph[i].extend([f"_{y}" for y in sampled_Y_ids])

    with open(f"{os.path.join(curr_dir, 'random_data.txt')}", "w") as f:
        for k, v in graph.items():
            f.write(f"{k} {' '.join(v)}\n")

if __name__ == "__main__":
    generate_random_graph_data()
