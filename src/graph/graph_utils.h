#ifndef GRAPH_UTILS_H
#define GRAPH_UTILS_H

#include <map>
#include <queue>
#include <stdexcept>
#include "DAGException.h"
#include "../nodes/Input.h"
#include <boost/range/adaptor/reversed.hpp>


/*
 * Use Kahn's algorithm to sort nodes
 * Sort Graph so that we can run forward (and backward) propagation
 */
void topologicalSort(vector<Node *> &graph);

/*
 * Sort graph and then assign inputs values to Input Nodes.
 * inputMap is loosely inspired by TensorFlow feed_dict
 */
void buildGraph(vector<Node *> &graph, map<Node*, Eigen::MatrixXd> inputMap);

/*
 * Run Forward and backward propagation given a computation graph
 */
vector<Eigen::MatrixXd> forwardBackward(const vector<Node *> &graph);

void feedValues(map<Node*, Eigen::MatrixXd> inputMap);

void buildGraph(vector<Node *> & graph)
{
  topologicalSort(graph);
}

void topologicalSort(vector<Node *> &graph)
/*
https://www.geeksforgeeks.org/topological-sorting-indegree-based-solution/
*/
{
  int visitedNodes = 0;
  /*
   * Compute in-degree (number of incoming edges) for each
   * of the vertex present in the DAG
   */
  map <Node*, int> inNodesCount;
  vector<Eigen::MatrixXd> inputs;
  for(auto n : graph){
    inputs = n->getInputValues();
    inNodesCount.insert(pair <Node*, int> (n, inputs.size()));
  }

  /*
   * Pick all the vertices with in-degree as 0 and add
   * them into a queue (Enqueue operation)
   */
  queue<Node *> q;
  map<Node*, int>::iterator it = inNodesCount.begin();
  while(it != inNodesCount.end()){
    if(it->second == 0){
      q.push(it->first);
    }
    ++it;
  }
  /* Remove a vertex from the queue (Dequeue operation) and then.
   *
   * Increment count of visited nodes by 1.
   * Decrease in-degree by 1 for all its neighboring nodes.
   * If in-degree of a neighboring nodes is reduced to zero, then add it to the queue.

   */
  Node *current;
  vector<Node *> neighbours;

  vector<Node *> temp;
  while(!q.empty()){
    current = q.front();
    temp.push_back(current);
    q.pop();

    //Increment count of visited nodes by 1.
    ++visitedNodes;

    //Decrease in-degree by 1 for all its neighboring nodes.
    neighbours = current->getOutputNodes();
    int inDegree = 0;
    for(auto n : neighbours){
      --inNodesCount.find(n)->second;
      //If in-degree of a neighboring nodes is reduced to zero, then add it to the queue.
      inDegree = inNodesCount.find(n)->second;
      if(inDegree==0){
        q.push(n);
      }
    }
  }
  if(visitedNodes <static_cast<int>(graph.size())){
    throw DAGException();
  }
  temp.swap(graph);
}


vector<Eigen::MatrixXd> forwardBackward(const vector<Node *> &graph)
{
  vector<Eigen::MatrixXd> results;
  for(auto n : graph){
    n->forward();
  }

  for (auto n : boost::adaptors::reverse(graph)) {
    n->backward();
  }

  // Find output nodes
  Eigen::MatrixXd temp;
  for(auto n : graph){
    if(n->getOutputNodes().size() == 0){
      n->getValues(temp);
      results.push_back(temp);
    }
  }
  return results;
}

void feedValues(map<Node*, Eigen::MatrixXd> inputMap)
{
  // Assign the desired values to the inputs
  map<Node*, Eigen::MatrixXd>::iterator it = inputMap.begin();
  while(it != inputMap.end()){
    // Verify that we were given only Input nodes to assign values to
    if(Input* b1 = dynamic_cast<Input*> (it->first)){
      it->first->setValues(it->second);
    }
    else{
      throw("Invalid Input type.");
    }
    ++it;
  }
}

#endif
