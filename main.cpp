#include<iostream>
#include<fstream>
#include<algorithm>
#include<vector>

using std::cin;
using std::cout;
using std::vector;

enum GameResult {FIRST_WON, SECOND_WON, DRAW};

class game {
public:
    int first;
    int second;
    GameResult winner;

    game(const int first, const int second, const GameResult winner):
        first(first), second(second), winner(winner) {}
};


class directed_graph {
    vector<vector<int> > adjacency_lists;

public:
    explicit directed_graph(const int nodes_quantity) {
        adjacency_lists.assign(nodes_quantity, vector<int>());
    }

    void add_edge(const int from_node, const int to_node) {
        adjacency_lists[from_node].push_back(to_node);
    }

    const vector<int>& get_adjacency_list_for_node(const int node_index) const {
        return adjacency_lists[node_index];
    }

    int get_nodes_quantity() const {
        return adjacency_lists.size();
    }
};

directed_graph construct_graph_from_games_results(const vector<game> &games_results, 
                                                    const int candidates_quantity) {
    directed_graph new_graph(candidates_quantity);

    for (int i = 0; i < games_results.size(); ++i) {
        int first = games_results[i].first;
        int second = games_results[i].second;
        if (games_results[i].winner == DRAW) {
            continue;
        }
        if (games_results[i].winner == SECOND_WON) {
            std::swap(first, second);
        }
        new_graph.add_edge(first-1, second-1);
    }
    return new_graph;
}


directed_graph transpose_graph(const directed_graph &graph) {
    directed_graph transposed_graph(graph.get_nodes_quantity());

    for (int i = 0; i < graph.get_nodes_quantity(); ++i) {
        for (int j = 0; j < graph.get_adjacency_list_for_node(i).size(); ++j) {
            transposed_graph.add_edge(graph.get_adjacency_list_for_node(i)[j], i);
        }
    }
    
    return transposed_graph;
}

void dfs(const int start_node_index, vector<int> &nodes_color, 
                const directed_graph &graph, vector<int> &nodes_path_order,
                vector<bool> &looked) {
    nodes_color[start_node_index] = 1;
    for (int i = 0; i < graph.get_adjacency_list_for_node(start_node_index).size(); ++i) {
        int linked_node = graph.get_adjacency_list_for_node(start_node_index)[i];
        if (!nodes_color[linked_node]) {
            dfs(linked_node, nodes_color, graph, nodes_path_order, looked);
        }
    }
    nodes_color[start_node_index] = 2;
    nodes_path_order.push_back(start_node_index);
    looked[start_node_index] = true;
}

vector<int> traverse_graph_in_dfs_order(const directed_graph &graph) {
   
    vector<bool> looked(graph.get_nodes_quantity(), false);
    // color of the nodes
    vector<int> nodes_color(graph.get_nodes_quantity(), 0);
    // nodes sequence in processing order
    vector<int> path;
    
    // launch dfs for all nonlooked nodes
    for (int i = 0; i < graph.get_nodes_quantity(); ++i) {
        if (looked[i]) {
            continue;
        }
        dfs(i, nodes_color, graph, path, looked);
    }
    return path;
}

vector<int> find_strongly_connected_components(const directed_graph &graph) {
    // first step - forward dfs from supernode
    vector<int> nodes_path_order = traverse_graph_in_dfs_order(graph);
    // second step - backward dfs
    // set the reverse order
    std::reverse(nodes_path_order.begin(), nodes_path_order.end());

    vector<int> nodes_strong_components(nodes_path_order.size(), 0);
    // make all nodes white
    vector<int> nodes_color(graph.get_nodes_quantity(), 0);
    directed_graph transposed_graph = transpose_graph(graph);
    int components_counter = 0;
        
    for (auto node : nodes_path_order) { 
        if (nodes_color[node]) {
            continue;
        }
        vector<int> nodes_in_cur_component;
        
        vector<bool> looked(transposed_graph.get_nodes_quantity(), false);
        dfs(node, nodes_color, transposed_graph, nodes_in_cur_component, looked);
        // set this nodes this class of equivalency
        for (int i = 0; i < nodes_in_cur_component.size(); ++i) {
            nodes_strong_components[nodes_in_cur_component[i]] = components_counter;
        }
        ++components_counter;
    }
    
    return nodes_strong_components;
}

// find strong connected components without inbound edges
vector<bool> is_strong_components_root(const vector<int> &nodes_strong_components,
                                         const directed_graph &graph) {
    int strong_components_quantity = *std::max_element(nodes_strong_components.begin(),
                                                        nodes_strong_components.end()) + 1;
    vector<bool> is_root;
    is_root.assign(strong_components_quantity, true);
    for (int i = 0; i < graph.get_nodes_quantity(); ++i) {
        for (auto adjacency_node : graph.get_adjacency_list_for_node(i)) {
            if (nodes_strong_components[i] != nodes_strong_components[adjacency_node]) {
                is_root[nodes_strong_components[adjacency_node]] = false;
            }
        }
    }
    return is_root;
}

// return the size of minimal strong connected component without inbound edges
int find_min_root_strong_component_size(const vector<int> &nodes_strong_components, 
                                        const vector<bool> &is_root) {
    vector<int> class_power;
    class_power.assign(is_root.size(), 0);
    
    for (int i = 0; i < nodes_strong_components.size(); ++i) {
        ++class_power[nodes_strong_components[i]];
    }

    int min = nodes_strong_components.size();
    for (int i = 0; i < class_power.size(); ++i) {
        if (is_root[i]) {
            min = std::min(min, class_power[i]);
        }
    }
    return min;
}


int calculate_maximal_size_of_company(directed_graph &corporation_graph) {
    vector<int> strong_components_of_nodes = find_strongly_connected_components(corporation_graph);

    vector<bool> is_root = is_strong_components_root(strong_components_of_nodes, 
                                                        corporation_graph);
    
    return 
        strong_components_of_nodes.size() + 1 - 
            find_min_root_strong_component_size(strong_components_of_nodes, is_root);
}


directed_graph read_games_results() {
    int candidates_quantity;
    int games_quantity;
    cin >> candidates_quantity;
    cin >> games_quantity;

    vector<game> games;
    // read games
    for (int index = 0; index < games_quantity; ++index) {
        int first;
        int second;
        int game_final;
        GameResult result;
        cin >> first;
        cin >> second;
        cin >> game_final;
        switch (game_final) {
        case 1: 
            result = FIRST_WON;
            break;
        case 2:
            result = SECOND_WON;
            break;
        case 3:
            result = DRAW;
            break;
        }
        games.push_back(game(first, second, result));
    }
    return construct_graph_from_games_results(games, candidates_quantity);
}

int main() {
    std::ios_base::sync_with_stdio(false);

    directed_graph corporation_graph = read_games_results();

    cout << calculate_maximal_size_of_company(corporation_graph);

    // system("pause");
    return 0;
}
