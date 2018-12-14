/*
 * Node.h
 *
 *  Created on: Dec 7, 2018
 *      Author: jenda
 */

#ifndef TESTS_MEDIATOR_MOCKS_LIBNODE_NODE_H_
#define TESTS_MEDIATOR_MOCKS_LIBNODE_NODE_H_

#include "gmock/gmock.h"

class Node {
public:
  Node(){}
  Node(const Node& n){
    (void)n;
  }

  virtual ~Node() {}
  virtual void RejoinAsNormal(){}
};

class MockNode : public Node {
public:
  MockNode(){}
  MockNode(const MockNode& mn) : Node(mn) {
    (void)mn;
  }

  MOCK_METHOD0(RejoinAsNormal, void());
};

#endif /* TESTS_MEDIATOR_MOCKS_LIBNODE_NODE_H_ */